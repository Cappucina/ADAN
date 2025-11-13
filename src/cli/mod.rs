/*
use clap::{Parser, Subcommand};
use rustyline::error::ReadlineError;
use rustyline::Editor;
use std::process::Command;

#[derive(Parser)]
#[clap(name ="Adan CLI", about="THE ADAN CLI: Written by the Cappucina Team.")]
pub struct Cli {
    #[clap(Subcommand)]
    commands: Cmd
}

pub enum Cmd {
    Compile {
        #[arg(short, long)]
        file: String
    },
    Repl
}

pub fn initialize() {
    let cli = Cli::parse();

    match cli.command {
        Cmd::Repl {} => {
            let rl = Editor::<()>::new().unwrap();

            loop {
                let readline = rl.readline("†> ");
                match readline {
                    Ok(line) => {
                        rl.add_history_entry(line.as_str());
                        println!("You typed: {}", line);
                    }
                    Err(ReadlineError::Interrupted) => {
                        println!("CTRL-C");
                        break;
                    }
                    Err(ReadlineError::Eof) => {
                        println!("CTRL-D");
                        break;
                    }
                    Err(err) => {
                        println!("Error: {:?}", err);
                        break;
                    }
                }
            }
        },
        Cmd::Compile { file } => {
            
        }
    }
}
*/
