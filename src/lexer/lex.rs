#![allow(unused)]

use crate::lexer::tokens::Tokens;
use logos::Logos;

#[derive(Clone, Debug, Default, PartialEq)]
pub struct LexerError;

pub type LexerResult<'source> = Result<Tokens<'source>, LexerError>;

pub struct Lexer<'source> {
    pub lexer: logos::Lexer<'source, Tokens<'source>>,
}

impl<'source> Lexer<'source> {
    pub fn new(input: &'source str) -> Self {
        Lexer {
            lexer: Tokens::lexer(input),
        }
    }

    pub fn next(&mut self) -> Option<Tokens> {
        self.lexer.next().and_then(|result: Result<Tokens, LexerError>| result.ok())
    }
}