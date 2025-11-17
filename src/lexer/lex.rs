use crate::lexer::tokens::Tokens;
use logos::Logos;

pub struct Lexer<'source> {
    pub lexer: logos::Lexer<'source, Tokens>,
}

impl<'source> Lexer<'source> {
    pub fn new(input: &'source str) -> Self {
        Lexer {
            lexer: Tokens::lexer(input),
        }
    }

    pub fn next(&mut self) -> Option<Tokens> {
        self.lexer.next().and_then(|result| result.ok())
    }
}
