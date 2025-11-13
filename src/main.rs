mod lexer;
mod parser;
mod native;
mod code_gen;
mod cli;

use std::fs;
use std::path::Path;
use std::process::Command;
use crate::lexer::lexer::Lexer;
use crate::parser::parser::Parser;
use crate::code_gen::statements::{codegen_statements, load_native_registry};
use crate::code_gen::builder::CodeGenContext;
//use crate::cli::initialize;

fn create_binary() -> Result<(), Box<dyn std::error::Error>> {
    fs::create_dir_all("compiled")?;

    Command::new("llc")
        .args(&["compiled/output.ll", "-filetype=obj", "-o", "compiled/output.o"])
        .status()?;

    Command::new("gcc")
        .args(&["compiled/output.o", "-o", "compiled/output_exec"])
        .status()?;

    //Command::new("./compiled/output_exec")
    //    .status()?;

    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let input_path = Path::new("./examples/hello_adan.adn");
    let source = fs::read_to_string(&input_path)?;
    let tokens = Lexer::new(&source).tokenize()?;
    let mut parser = Parser::new(tokens);
    let statements = parser.parse()?;

    let context = inkwell::context::Context::create();
    let mut ctx = CodeGenContext::new(&context, "adan_module");

    let registry = load_native_registry();
    for register_fn in registry.values() {
        register_fn(&mut ctx);
    }

    for stmt in &statements {
        codegen_statements(&mut ctx, stmt, &registry)?;
    }

    fs::create_dir_all("compiled")?;
    ctx.module.print_to_file("compiled/output.ll")?;
    create_binary()?;

    //initialize();
    Ok(())
}
