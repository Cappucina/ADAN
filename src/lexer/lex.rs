#![allow(unused)]

use crate::lexer::tokens::Tokens;
use logos::{Logos, SpannedIter};
use std::ops::Range;

#[derive(Clone, Debug, Default, PartialEq)]
pub struct LexerError;

pub type LexerResult<'source> = Result<Tokens<'source>, LexerError>;
pub type Span = Range<usize>;

/// Custom lexer wrapper that provides a convenient API around the logos lexer
pub struct Lexer<'source> {
    lexer: logos::Lexer<'source, Tokens<'source>>,
}

impl<'source> Lexer<'source> {
    /// Create a new lexer from source code
    pub fn new(input: &'source str) -> Self {
        Lexer {
            lexer: Tokens::lexer(input),
        }
    }

    /// Get the next token, filtering out errors
    pub fn next(&mut self) -> Option<Tokens<'source>> {
        self.lexer
            .next()
            .and_then(|result: Result<Tokens, LexerError>| result.ok())
    }

    /// Get the source string being lexed
    pub fn source(&self) -> &'source str {
        self.lexer.source()
    }

    /// Get an iterator over tokens with their spans
    pub fn spanned(self) -> SpannedIter<'source, Tokens<'source>> {
        self.lexer.spanned()
    }

    /// Get access to the underlying logos lexer
    pub fn inner(&self) -> &logos::Lexer<'source, Tokens<'source>> {
        &self.lexer
    }

    /// Get mutable access to the underlying logos lexer
    pub fn inner_mut(&mut self) -> &mut logos::Lexer<'source, Tokens<'source>> {
        &mut self.lexer
    }
}
