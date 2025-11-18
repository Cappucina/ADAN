use logos::Logos;

use crate::lexer::lex::LexerError;

#[derive(Logos, Debug, PartialEq, Clone)]
#[logos(error = LexerError)]
pub enum Tokens<'source> {
    // ========================================================================
    // WHITESPACE & COMMENTS
    // ========================================================================

    /// Horizontal whitespace (spaces, tabs)
    #[regex(r"[[:blank:]]+", |lex| lex.slice())]
    HorizontalWhitespace(&'source str),

    /// Vertical whitespace (newlines, line breaks)
    #[regex(r#"[\n\v\f\r\x85\u2028\u2029]+"#, |lex| lex.slice())]
    VerticalWhitespace(&'source str),

    /// Single-line comments (// ...)
    #[regex(r"//[^\n]*", |lex| lex.slice())]
    Comment(&'source str),

    // ========================================================================
    // KEYWORDS
    // ========================================================================

    // --- Module & Scope Keywords ---

    /// Import/include external files or modules
    #[token("include")]
    Include,

    /// Declare a variable with local scope (within current function/block)
    #[token("local")]
    Local,

    /// Declare a variable with global scope (accessible everywhere)
    #[token("global")]
    Global,

    // --- Control Flow Keywords ---

    /// Conditional statement (if condition { ... })
    #[token("if")]
    If,

    /// Alternative branch for if statement (else { ... })
    #[token("else")]
    Else,

    /// Loop that continues while condition is true
    #[token("while")]
    While,

    /// Loop that iterates over a range or collection
    #[token("for")]
    For,

    /// Exit early from a loop
    #[token("break")]
    Break,

    /// No-operation statement (placeholder that does nothing, like Python's pass)
    #[token("pass")]
    Pass,

    // --- Function/Program Definition Keywords ---

    /// Define a function/program (similar to 'fn' or 'def' in other languages)
    #[token("program")]
    Program,

    /// Return a value from a function
    #[token("return")]
    Return,

    // ========================================================================
    // OPERATORS
    // ========================================================================

    // --- Assignment Operator ---

    /// Assignment operator (variable -> value)
    #[token("->")]
    Assign,

    // --- Arithmetic Operators ---

    /// Addition operator (+)
    #[token("+")]
    Plus,

    /// Subtraction operator (-)
    #[token("-")]
    Minus,

    /// Multiplication operator (*)
    #[token("*")]
    Asterisk,

    /// Division operator (/)
    #[token("/")]
    Slash,

    /// Modulus/remainder operator (%)
    #[token("%")]
    Modulus,

    /// Exponentiation/power operator (^)
    #[token("^")]
    Caret,

    // --- Bitwise Operators ---

    /// Bitwise OR operator (|)
    #[token("|")]
    BitWiseOr,

    /// Bitwise NOT operator (!~)
    #[token("!~")]
    BitWiseNot,

    /// Bitwise left shift operator (<<)
    #[token("<<")]
    ShiftLeft,

    /// Bitwise right shift operator (>>)
    #[token(">>")]
    ShiftRight,

    /// Bitwise AND operator ($)
    #[token("$")]
    BitWiseAnd,

    // --- Logical Operators ---

    /// Logical NOT operator (!)
    #[token("!")]
    Not,

    /// Logical AND operator (&&)
    #[token("&&")]
    And,

    /// Logical OR operator (||)
    #[token("||")]
    Or,

    // --- Comparison Operators ---

    /// Equality comparison (=)
    #[token("=")]
    Equals,

    /// Inequality comparison (!=)
    #[token("!=")]
    NotEqual,

    /// Less than comparison (<)
    #[token("<")]
    LessThan,

    /// Greater than comparison (>)
    #[token(">")]
    GreaterThan,

    /// Less than or equal comparison (<=)
    #[token("<=")]
    LessEqual,

    /// Greater than or equal comparison (>=)
    #[token(">=")]
    GreaterEqual,

    // --- String/Collection Operators ---

    /// String/list concatenation operator (..)
    #[token("..")]
    DoubleDot,

    // ========================================================================
    // DELIMITERS & PUNCTUATION
    // ========================================================================

    /// Left parenthesis - used for function calls, grouping expressions
    #[token("(")]
    LeftParen,

    /// Right parenthesis
    #[token(")")]
    RightParen,

    /// Left brace - used for code blocks, scopes
    #[token("{")]
    LeftBrace,

    /// Right brace
    #[token("}")]
    RightBrace,

    /// Comma - used for separating function parameters, list items
    #[token(",")]
    Comma,

    /// Semicolon - statement terminator
    #[token(";")]
    Semicolon,

    /// Colon - used in type annotations, ranges
    #[token(":")]
    Colon,

    /// Dot - member access operator (object.property)
    #[token(".")]
    Dot,

    // ========================================================================
    // LITERALS (Constant Values)
    // ========================================================================

    /// Boolean literal (true or false)
    #[regex("true|false", |lex| lex.slice() == "true")]
    Boolean(bool),

    /// Null/nil literal (represents absence of value)
    #[token("null")]
    Null,

    /// Floating-point number literal (e.g., 3.14, -0.5)
    #[regex("-?[0-9]+\\.[0-9]+", |lex| lex.slice().parse::<f64>().ok())]
    Float(f64),

    /// Integer number literal (e.g., 42, -100)
    #[regex("-?[0-9]+", |lex| lex.slice().parse::<i64>().ok())]
    Integer(i64),

    /// String literal with escape sequences (e.g., "hello\nworld")
    #[regex("\"(\\\\.|[^\"])*\"", |lex| {
        let slice = lex.slice();
        let unquoted = &slice[1..slice.len() - 1];
        unquoted.replace("\\\"", "\"").replace("\\n", "\n").replace("\\t", "\t")
    })]
    String(String),

    // ========================================================================
    // IDENTIFIERS (Variable/Function Names)
    // ========================================================================

    /// User-defined names for variables, functions, etc. (e.g., myVariable, calculate_sum)
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*", |lex| lex.slice())]
    Identifier(&'source str),
}
