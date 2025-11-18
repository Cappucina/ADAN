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

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} [options] <script.adn>", args[0]);
        eprintln!("");
        eprintln!("Options:");
        eprintln!("  -n, --native         Compile to native x86-64 executable");
        eprintln!("  -d, --disassemble    Show compiled code");
        eprintln!("  -o <file>            Output file (for --native)");
        eprintln!("  -s, --save-asm       Keep assembly file (for --native)");
        eprintln!("");
        eprintln!("Default mode: Compile to bytecode and execute with VM");
        process::exit(1);
    }

    let mut native = false;
    let mut disassemble = false;
    let mut save_asm = false;
    let mut output_file = None;
    let mut filename = None;

    // Parse arguments
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-n" | "--native" => native = true,
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

    if native {
        // Native compilation mode
        compile_to_native(&expressions, &filename, output_file, disassemble, save_asm);
    } else {
        // VM mode (bytecode)
        compile_and_run_vm(&expressions, &filename, disassemble);
    }
}

fn compile_to_native(
    expressions: &[parser::ast::Expression],
    input_file: &str,
    output_file: Option<String>,
    show_asm: bool,
    save_asm: bool,
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

    let output_exe = output_file.unwrap_or_else(|| base_name.to_string());
    let asm_file = format!("{}.asm", base_name);
    let obj_file = format!("{}.o", base_name);

    // Write assembly to file
    if let Err(err) = fs::write(&asm_file, &assembly) {
        eprintln!("Error writing assembly file: {}", err);
        process::exit(1);
    }

    if show_asm {
        println!("Generated assembly:");
        println!("{}", "=".repeat(80));
        println!("{}", assembly);
        println!("{}", "=".repeat(80));
        println!();
    }

    // Assemble with nasm
    println!("Assembling {} ...", asm_file);
    let nasm_status = Command::new("nasm")
        .args(&["-f", "elf64", &asm_file, "-o", &obj_file])
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

    // Link with ld
    println!("Linking {} ...", output_exe);
    let ld_status = Command::new("ld")
        .args(&[
            "-dynamic-linker", "/lib64/ld-linux-x86-64.so.2",
            &obj_file,
            "-lc",
            "-o", &output_exe
        ])
        .status();

    match ld_status {
        Ok(status) if status.success() => {},
        Ok(_) => {
            eprintln!("Error: linker failed");
            process::exit(1);
        }
        Err(err) => {
            eprintln!("Error running linker: {}", err);
            process::exit(1);
        }
    }

    // Clean up temporary files
    if !save_asm {
        let _ = fs::remove_file(&asm_file);
    }
    let _ = fs::remove_file(&obj_file);

    println!("✓ Successfully compiled to: {}", output_exe);
    println!("  Run with: ./{}", output_exe);
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
