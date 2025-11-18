mod compiler;
mod interpreter;
mod lexer;
mod parser;

use bumpalo::Bump;
use compiler::{Compiler, VirtualMachine, X86_64CodeGen};
use lexer::lex::Lexer;
use parser::parse::Parser;
use std::env;
use std::fs;
use std::path::Path;
use std::process::{self, Command};

#[derive(Debug, Clone, Copy, PartialEq)]
enum OutputFormat {
    Bytecode,    // Compile to bytecode and execute with VM (default)
    Native,      // Compile to native executable
    Assembly,    // Generate assembly file only
    Object,      // Generate object file only
    Ast,         // Output the AST as JSON/pretty-printed
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum CompilationTarget {
    X86_64Linux,     // x86-64 Linux (System V ABI)
    X86_64MacOS,     // x86-64 macOS (Mach-O)
    AArch64MacOS,    // ARM64 macOS (Apple Silicon)
    AArch64Linux,    // ARM64 Linux
}

impl CompilationTarget {
    fn from_str(s: &str) -> Option<Self> {
        match s.to_lowercase().as_str() {
            "x86_64-linux" | "x86-64-linux" | "linux" => Some(CompilationTarget::X86_64Linux),
            "x86_64-macos" | "x86-64-macos" | "macos" => Some(CompilationTarget::X86_64MacOS),
            "aarch64-macos" | "arm64-macos" | "apple-silicon" => Some(CompilationTarget::AArch64MacOS),
            "aarch64-linux" | "arm64-linux" => Some(CompilationTarget::AArch64Linux),
            _ => None,
        }
    }

    fn detect_host() -> Self {
        // Detect the current platform
        #[cfg(all(target_arch = "x86_64", target_os = "linux"))]
        return CompilationTarget::X86_64Linux;

        #[cfg(all(target_arch = "x86_64", target_os = "macos"))]
        return CompilationTarget::X86_64MacOS;

        #[cfg(all(target_arch = "aarch64", target_os = "macos"))]
        return CompilationTarget::AArch64MacOS;

        #[cfg(all(target_arch = "aarch64", target_os = "linux"))]
        return CompilationTarget::AArch64Linux;

        // Default fallback
        #[cfg(not(any(
            all(target_arch = "x86_64", target_os = "linux"),
            all(target_arch = "x86_64", target_os = "macos"),
            all(target_arch = "aarch64", target_os = "macos"),
            all(target_arch = "aarch64", target_os = "linux")
        )))]
        CompilationTarget::X86_64Linux
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} [options] <script.adn>", args[0]);
        eprintln!("");
        eprintln!("Options:");
        eprintln!("  -f, --format <format>  Output format (bytecode, native, assembly, object, ast)");
        eprintln!("  -t, --target <target>  Compilation target (x86_64-linux, x86_64-macos, aarch64-macos, aarch64-linux)");
        eprintln!("  -n, --native           Compile to native x86-64 executable (same as --format native)");
        eprintln!("  -d, --disassemble      Show compiled code");
        eprintln!("  -o <file>              Output file");
        eprintln!("  -s, --save-asm         Keep assembly file (for native compilation)");
        eprintln!("");
        eprintln!("Output formats:");
        eprintln!("  bytecode   - Compile to bytecode and execute with VM (default)");
        eprintln!("  native     - Compile to native executable");
        eprintln!("  assembly   - Generate assembly file only (.asm)");
        eprintln!("  object     - Generate object file only (.o)");
        eprintln!("  ast        - Output the AST (Abstract Syntax Tree)");
        eprintln!("");
        eprintln!("Compilation targets:");
        eprintln!("  x86_64-linux   - x86-64 Linux (System V ABI, ELF)");
        eprintln!("  x86_64-macos   - x86-64 macOS (Mach-O)");
        eprintln!("  aarch64-macos  - ARM64 macOS / Apple Silicon (Mach-O)");
        eprintln!("  aarch64-linux  - ARM64 Linux (ELF)");
        eprintln!("  (default: auto-detect host platform)");
        eprintln!("");
        eprintln!("Examples:");
        eprintln!("  {} script.adn                          # Run with VM (default)", args[0]);
        eprintln!("  {} --format native script.adn          # Compile to executable", args[0]);
        eprintln!("  {} --format native --target x86_64-linux script.adn  # Cross-compile for Linux", args[0]);
        eprintln!("  {} --format assembly -o out.asm script.adn  # Generate assembly", args[0]);
        eprintln!("  {} --format ast script.adn             # Show AST", args[0]);
        process::exit(1);
    }

    let mut format = OutputFormat::Bytecode; // Default format
    let mut disassemble = false;
    let mut save_asm = false;
    let mut output_file = None;
    let mut filename = None;

    // Parse arguments
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-f" | "--format" => {
                i += 1;
                if i < args.len() {
                    format = match args[i].to_lowercase().as_str() {
                        "bytecode" => OutputFormat::Bytecode,
                        "native" => OutputFormat::Native,
                        "assembly" | "asm" => OutputFormat::Assembly,
                        "object" | "obj" => OutputFormat::Object,
                        "ast" => OutputFormat::Ast,
                        _ => {
                            eprintln!("Unknown format: {}", args[i]);
                            eprintln!("Valid formats: bytecode, native, assembly, object, ast");
                            process::exit(1);
                        }
                    };
                } else {
                    eprintln!("Error: --format requires an argument");
                    process::exit(1);
                }
            }
            "-n" | "--native" => format = OutputFormat::Native,
            "-d" | "--disassemble" => disassemble = true,
            "-s" | "--save-asm" => save_asm = true,
            "-o" => {
                i += 1;
                if i < args.len() {
                    output_file = Some(args[i].clone());
                }
            }
            arg if !arg.starts_with('-') => filename = Some(arg.to_string()),
            _ => {
                eprintln!("Unknown option: {}", args[i]);
                process::exit(1);
            }
        }
        i += 1;
    }

    let filename = match filename {
        Some(f) => f,
        None => {
            eprintln!("Error: No input file specified");
            process::exit(1);
        }
    };

    // Read the source file
    let source = match fs::read_to_string(&filename) {
        Ok(content) => content,
        Err(err) => {
            eprintln!("Error reading file '{}': {}", filename, err);
            process::exit(1);
        }
    };

    // Create allocator for parser
    let allocator = Bump::new();

    // Lex the source code
    let lexer = Lexer::new(&source);

    // Parse the source code
    let parser = Parser::new(lexer, &allocator);
    let expressions = match parser.parse() {
        Ok(ast) => ast,
        Err(err) => {
            eprintln!("Parse error: {:?}", err);
            process::exit(1);
        }
    };

    // Compile based on output format
    match format {
        OutputFormat::Bytecode => {
            compile_and_run_vm(&expressions, &filename, disassemble);
        }
        OutputFormat::Native => {
            compile_to_native(&expressions, &filename, output_file, save_asm);
        }
        OutputFormat::Assembly => {
            generate_assembly(&expressions, &filename, output_file);
        }
        OutputFormat::Object => {
            generate_object(&expressions, &filename, output_file);
        }
        OutputFormat::Ast => {
            output_ast(&expressions, output_file);
        }
    }
}

fn compile_to_native(
    expressions: &[parser::ast::Expression],
    input_file: &str,
    output_file: Option<String>,
    save_asm: bool,
) {
    // Detect target platform
    let target = CompilationTarget::detect_host();

    // Generate assembly code
    let mut codegen = X86_64CodeGen::new();
    let assembly = match codegen.compile(expressions) {
        Ok(asm) => asm,
        Err(err) => {
            eprintln!("Code generation error: {}", err);
            process::exit(1);
        }
    };

    // Determine output filename
    let base_name = Path::new(input_file)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("output");

    let output_exe = output_file.unwrap_or_else(|| base_name.to_string());
    let asm_file = format!("{}.asm", base_name);
    let obj_file = format!("{}.o", base_name);

    // Write assembly to file
    if let Err(err) = fs::write(&asm_file, &assembly) {
        eprintln!("Error writing assembly file: {}", err);
        process::exit(1);
    }

    // Platform-specific assembly and linking
    match target {
        CompilationTarget::X86_64MacOS | CompilationTarget::AArch64MacOS => {
            compile_native_macos(&asm_file, &obj_file, &output_exe, save_asm);
        }
        CompilationTarget::X86_64Linux | CompilationTarget::AArch64Linux => {
            compile_native_linux(&asm_file, &obj_file, &output_exe, save_asm);
        }
    }

    println!("✓ Successfully compiled to: {}", output_exe);
    println!("  Run with: ./{}", output_exe);
}

fn compile_native_macos(asm_file: &str, obj_file: &str, output_exe: &str, save_asm: bool) {
    // Assemble with nasm for macOS
    println!("Assembling {} for macOS x86-64...", asm_file);
    let nasm_status = Command::new("nasm")
        .args(&["-f", "macho64", asm_file, "-o", obj_file])
        .status();

    match nasm_status {
        Ok(status) if status.success() => {},
        Ok(_) => {
            eprintln!("Error: nasm assembler failed");
            eprintln!("Make sure nasm is installed: brew install nasm");
            process::exit(1);
        }
        Err(err) => {
            eprintln!("Error running nasm: {}", err);
            eprintln!("Make sure nasm is installed: brew install nasm");
            process::exit(1);
        }
    }

    // Link with clang targeting x86-64 (will run via Rosetta on ARM Macs)
    println!("Linking {} ...", output_exe);
    let link_status = Command::new("clang")
        .args(&[
            "-arch", "x86_64",  // Explicitly target x86-64
            obj_file,
            "-o", output_exe,
            "-e", "_main",  // Entry point is _main on macOS
        ])
        .status();

    match link_status {
        Ok(status) if status.success() => {},
        Ok(_) => {
            eprintln!("Error: linker failed");
            eprintln!("Note: Make sure Xcode Command Line Tools are installed");
            eprintln!("Run: xcode-select --install");
            eprintln!("Note: x86-64 binaries will run via Rosetta 2 on Apple Silicon");
            process::exit(1);
        }
        Err(err) => {
            eprintln!("Error running linker: {}", err);
            eprintln!("Note: Make sure Xcode Command Line Tools are installed");
            eprintln!("Run: xcode-select --install");
            process::exit(1);
        }
    }

    // Clean up temporary files
    if !save_asm {
        let _ = fs::remove_file(asm_file);
    }
    let _ = fs::remove_file(obj_file);

    println!("Note: Compiled for x86-64. Will run via Rosetta 2 on Apple Silicon Macs");
}

fn compile_native_linux(asm_file: &str, obj_file: &str, output_exe: &str, save_asm: bool) {
    // Assemble with nasm for Linux
    println!("Assembling {} for Linux...", asm_file);
    let nasm_status = Command::new("nasm")
        .args(&["-f", "elf64", asm_file, "-o", obj_file])
        .status();

    match nasm_status {
        Ok(status) if status.success() => {},
        Ok(_) => {
            eprintln!("Error: nasm assembler failed");
            eprintln!("Make sure nasm is installed: sudo apt-get install nasm");
            process::exit(1);
        }
        Err(err) => {
            eprintln!("Error running nasm: {}", err);
            eprintln!("Make sure nasm is installed: sudo apt-get install nasm");
            process::exit(1);
        }
    }

    // Link with ld or gcc
    println!("Linking {} ...", output_exe);

    // Try gcc first (more portable), fall back to ld
    let link_status = Command::new("gcc")
        .args(&[
            obj_file,
            "-o", output_exe,
            "-no-pie",
        ])
        .status();

    let success = match link_status {
        Ok(status) if status.success() => true,
        _ => {
            // Try ld as fallback
            let ld_status = Command::new("ld")
                .args(&[
                    "-dynamic-linker", "/lib64/ld-linux-x86-64.so.2",
                    obj_file,
                    "-lc",
                    "-o", output_exe
                ])
                .status();

            match ld_status {
                Ok(status) if status.success() => true,
                _ => false,
            }
        }
    };

    if !success {
        eprintln!("Error: linker failed");
        eprintln!("Make sure gcc or ld is installed");
        process::exit(1);
    }

    // Clean up temporary files
    if !save_asm {
        let _ = fs::remove_file(asm_file);
    }
    let _ = fs::remove_file(obj_file);
}

fn compile_and_run_vm(
    expressions: &[parser::ast::Expression],
    filename: &str,
    disassemble: bool,
) {
    // Compile to bytecode
    let mut compiler = Compiler::new();
    let chunk = match compiler.compile(expressions) {
        Ok(chunk) => chunk,
        Err(err) => {
            eprintln!("Compilation error: {}", err);
            process::exit(1);
        }
    };

    // Optionally disassemble the bytecode
    if disassemble {
        chunk.disassemble(filename);
        println!();
    }

    // Execute the bytecode
    let mut vm = VirtualMachine::new();
    if let Err(err) = vm.execute(&chunk) {
        eprintln!("Runtime error: {}", err);
        process::exit(1);
    }
}

fn generate_assembly(
    expressions: &[parser::ast::Expression],
    input_file: &str,
    output_file: Option<String>,
) {
    // Generate assembly code
    let mut codegen = X86_64CodeGen::new();
    let assembly = match codegen.compile(expressions) {
        Ok(asm) => asm,
        Err(err) => {
            eprintln!("Code generation error: {}", err);
            process::exit(1);
        }
    };

    // Determine output filename
    let base_name = Path::new(input_file)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("output");

    let asm_file = output_file.unwrap_or_else(|| format!("{}.asm", base_name));

    // Write assembly to file or stdout
    if asm_file == "-" {
        println!("{}", assembly);
    } else {
        if let Err(err) = fs::write(&asm_file, &assembly) {
            eprintln!("Error writing assembly file: {}", err);
            process::exit(1);
        }
        println!("✓ Generated assembly: {}", asm_file);
    }
}

fn generate_object(
    expressions: &[parser::ast::Expression],
    input_file: &str,
    output_file: Option<String>,
) {
    // Generate assembly code
    let mut codegen = X86_64CodeGen::new();
    let assembly = match codegen.compile(expressions) {
        Ok(asm) => asm,
        Err(err) => {
            eprintln!("Code generation error: {}", err);
            process::exit(1);
        }
    };

    // Determine output filename
    let base_name = Path::new(input_file)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("output");

    let asm_file = format!("{}.tmp.asm", base_name);
    let obj_file = output_file.unwrap_or_else(|| format!("{}.o", base_name));

    // Write assembly to temporary file
    if let Err(err) = fs::write(&asm_file, &assembly) {
        eprintln!("Error writing assembly file: {}", err);
        process::exit(1);
    }

    // Assemble with nasm
    println!("Assembling {} ...", obj_file);
    let nasm_status = Command::new("nasm")
        .args(&["-f", "elf64", &asm_file, "-o", &obj_file])
        .status();

    match nasm_status {
        Ok(status) if status.success() => {},
        Ok(_) => {
            eprintln!("Error: nasm assembler failed");
            eprintln!("Make sure nasm is installed: sudo apt-get install nasm");
            let _ = fs::remove_file(&asm_file);
            process::exit(1);
        }
        Err(err) => {
            eprintln!("Error running nasm: {}", err);
            eprintln!("Make sure nasm is installed: sudo apt-get install nasm");
            let _ = fs::remove_file(&asm_file);
            process::exit(1);
        }
    }

    // Clean up temporary assembly file
    let _ = fs::remove_file(&asm_file);

    println!("✓ Generated object file: {}", obj_file);
}

fn output_ast(expressions: &[parser::ast::Expression], output_file: Option<String>) {
    // Format AST as debug output (pretty-printed)
    let ast_output = format!("{:#?}", expressions);

    // Write to file or stdout
    if let Some(file) = output_file {
        if file == "-" {
            println!("{}", ast_output);
        } else {
            if let Err(err) = fs::write(&file, &ast_output) {
                eprintln!("Error writing AST file: {}", err);
                process::exit(1);
            }
            println!("✓ Generated AST output: {}", file);
        }
    } else {
        println!("{}", ast_output);
    }
}
