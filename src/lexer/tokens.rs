use logos::Logos;

use crate::lexer::lex::LexerError;

#[derive(Logos, Debug, PartialEq, Clone)]
#[logos(error = LexerError)]
pub enum Tokens<'source> {
    // Whitespace
    #[regex(r"[[:blank:]]+", |lex| lex.slice())] HorizontalWhitespace(&'source str),
    #[regex(r#"[\n\v\f\r\x85\u2028\u2029]+"#, |lex| lex.slice())] VerticalWhitespace(&'source str),

    // Comments
    #[regex(r"//[^\n]*", |lex| lex.slice())] Comment(&'source str),

    // Keywords
    #[token("include")] Include,
    #[token("local")] Local,
    #[token("global")] Global,
    #[token("if")] If,
    #[token("else")] Else,
    #[token("while")] While,
    #[token("for")] For,
    #[token("program")] Program,
    #[token("return")] Return,
    #[token("break")] Break,
    #[token("pass")] Pass,
    #[token("->")] Assign,

    // Literals
    #[regex("true|false", |lex| lex.slice() == "true")] Boolean(bool),
    #[token("null")] Null,
    #[regex("-?[0-9]+\\.[0-9]+", |lex| lex.slice().parse::<f64>().ok())] Float(f64),
    #[regex("-?[0-9]+", |lex| lex.slice().parse::<i64>().ok())] Integer(i64),
    #[regex("\"(\\\\.|[^\"])*\"", |lex| {
        let slice = lex.slice();
        let unquoted = &slice[1..slice.len() - 1];
        unquoted.replace("\\\"", "\"").replace("\\n", "\n").replace("\\t", "\t")
    })] String(String),

    // Identifiers
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*", |lex| lex.slice())] Identifier(&'source str),

    // Operands / Symbols
    #[token("+")] Plus,
    #[token("-")] Minus,
    #[token("*")] Asterisk,
    #[token("/")] Slash,
    #[token("%")] Percent,
    #[token("^")] Caret,
    #[token("=")] Equals,
    #[token("!")] Not,
    #[token("&&")] And,
    #[token("||")] Or,
    #[token("<")] LessThan,
    #[token(">")] GreaterThan,
    #[token("<=")] LessEqual,
    #[token(">=")] GreaterEqual,
    #[token("!=")] NotEqual,
    #[token("(")] LeftParen,
    #[token(")")] RightParen,
    #[token("{")] LeftBrace,
    #[token("}")] RightBrace,
    #[token(",")] Comma,
    #[token(";")] Semicolon,
    #[token(":")] Colon,
    #[token(".")] Dot,
}
