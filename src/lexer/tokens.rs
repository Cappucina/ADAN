use logos::Logos;

#[derive(Logos, Debug, PartialEq, Clone)]
pub enum Tokens {
    // Operands
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
    #[token("//")] Comment,

    // Keywords
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
    #[regex("[a-zA-Z_][a-zA-Z0-9_]*", |lex| lex.slice().to_string())] Identifier(String),
    #[regex("-?[0-9]+\\.[0-9]+", |lex| lex.slice().parse::<f64>().unwrap())] Float(f64),
    #[regex("-?[0-9]+", |lex| lex.slice().parse::<i64>().unwrap())] Integer(i64),
    #[regex("\"(\\\\.|[^\"])*\"", |lex| {
        let slice = lex.slice();
        let unquoted = &slice[1..slice.len() - 1];
        unquoted.replace("\\\"", "\"").replace("\\n", "\n").replace("\\t", "\t")
    })] String(String),
    #[regex("true|false", |lex| lex.slice() == "true")] Boolean(bool),
    #[token("null")] Null,

    // Error handling
    #[regex(r"[ \t\n\f]+", logos::skip)]
    Error,
}