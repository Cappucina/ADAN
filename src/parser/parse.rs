use std::cell::RefCell;
use std::ops::Range;
use crate::parser::ast::*;
use bumpalo::Bump;
use crate::lexer::lex::LexerResult;
use crate::lexer::tokens::Tokens;
use logos::Lexer;

type Allocator = Bump;
type Span = Range<usize>;

pub enum ParseError<'source> {
    UnexpectedEof,
    UnexpectedEol,

    IllegalToken(Span),
    MissingSemicolon(Span),
    ExpectedIdentifier(Span),
    MissingRightParen(Span),

    MissingClosingBrace {
        expected: Tokens<'source>,
        opening_span: Span,
        found: LexerResult<'source>,
        span: Span,
    },
}

pub type ParseResult<'source, T> = Result<T, ParseError<'source>>;

pub struct Parser<'alloc, 'source> {
    allocator: &'alloc Allocator,
    source: &'source str,
    tokens: Vec<(LexerResult<'source>, Span)>,
    errors: RefCell<Vec<ParseError<'source>>>,
    cursor: RefCell<usize>,
}

impl<'alloc, 'source> Parser<'alloc, 'source> {
    pub fn new(lexer: Lexer<'source, Tokens<'source>>, allocator: &'alloc Allocator) -> Self {
        let source = lexer.source();
        let tokens = lexer.spanned().collect();
        
        Parser {
            allocator,
            source,
            tokens,
            errors: RefCell::new(Vec::new()),
            cursor: RefCell::new(0),
        }
    }

    // https://github.com/aatxe/witch-hazel/blob/primary/syntax/src/parser/mod.rs
    // Inspiration for parser structure

    fn no_errors(&self) -> bool {
        self.errors.borrow().is_empty()
    }

    fn peek(&self) -> Option<(LexerResult<'source>, Span)> {
        self.tokens.get(*self.cursor.borrow())
            .cloned()
    }

    fn advance(&self) {
        *self.cursor.borrow_mut() += 1;
    }

    // Identical to `advance` but returns the token during advancement.
    fn bump(&self) -> Option<(LexerResult<'source>, Span)> {
        let token = self.peek();
        self.advance();
        token
    }

    
}