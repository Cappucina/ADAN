use crate::lexer::lex::{Lexer, LexerResult};
use crate::lexer::tokens::Tokens;
use crate::parser::ast::*;
use bumpalo::Bump;
use std::cell::RefCell;
use std::ops::Range;

type Allocator = Bump;
type Span = Range<usize>;

#[derive(Debug)]
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

impl<'alloc, 'source: 'alloc> Parser<'alloc, 'source> {
    /// Create a new parser from a lexer and allocator
    pub fn new(lexer: Lexer<'source>, allocator: &'alloc Allocator) -> Self {
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
        self.tokens.get(*self.cursor.borrow()).cloned()
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

    fn ignore_whitespace(&self) {
        while let Some((Ok(Tokens::HorizontalWhitespace(_) | Tokens::VerticalWhitespace(_)), _)) =
            self.peek()
        {
            self.advance();
        }
    }

    fn ignore_whitespace_and_comments(&self) {
        while let Some((Ok(token), _)) = self.peek() {
            match token {
                Tokens::HorizontalWhitespace(_)
                | Tokens::VerticalWhitespace(_)
                | Tokens::Comment(_) => {
                    self.advance();
                }
                _ => break,
            }
        }
    }

    fn expect(&self, expected: Tokens<'source>) -> ParseResult<'source, ()> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(token), span)) = self.peek() {
            if std::mem::discriminant(&token) == std::mem::discriminant(&expected) {
                self.advance();
                Ok(())
            } else {
                Err(ParseError::IllegalToken(span))
            }
        } else {
            Err(ParseError::UnexpectedEof)
        }
    }

    fn at_eof(&self) -> bool {
        *self.cursor.borrow() >= self.tokens.len()
    }

    // Parse a literal value (integer, float, string, boolean, null)
    fn parse_literal(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(token), _)) = self.peek() {
            let expr = match token {
                Tokens::Integer(value) => {
                    self.advance();
                    Expression::IntegerLiteral { value }
                }
                Tokens::Float(value) => {
                    self.advance();
                    Expression::FloatLiteral { value }
                }
                Tokens::String(ref s) => {
                    let value = self.allocator.alloc_str(s);
                    self.advance();
                    Expression::StringLiteral { value }
                }
                Tokens::Boolean(value) => {
                    self.advance();
                    Expression::BooleanLiteral { value }
                }
                Tokens::Null => {
                    self.advance();
                    Expression::NullLiteral
                }
                _ => return Err(ParseError::IllegalToken(self.peek().unwrap().1)),
            };
            Ok(expr)
        } else {
            Err(ParseError::UnexpectedEof)
        }
    }

    // Parse an identifier
    pub fn parse_identifier(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(Tokens::Identifier(name)), _)) = self.peek() {
            self.advance();
            Ok(Expression::Identifier { value: name })
        } else {
            let span = self.peek().map(|t| t.1).unwrap_or(0..0);
            Err(ParseError::ExpectedIdentifier(span))
        }
    }

    // Parse a primary expression (literal, identifier, or parenthesized expression)
    fn parse_primary(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(token), span)) = self.peek() {
            match token {
                Tokens::Integer(_)
                | Tokens::Float(_)
                | Tokens::String(_)
                | Tokens::Boolean(_)
                | Tokens::Null => self.parse_literal(),
                Tokens::Identifier(_) => self.parse_identifier(),
                Tokens::LeftParen => {
                    self.advance(); // consume '('
                    let expr = self.parse_expression()?;
                    self.ignore_whitespace_and_comments();

                    if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
                        self.advance();
                        Ok(expr)
                    } else {
                        Err(ParseError::MissingRightParen(span))
                    }
                }
                Tokens::LeftBrace => {
                    let block = self.parse_block()?;
                    Ok(Expression::Block { block })
                }
                _ => Err(ParseError::IllegalToken(span)),
            }
        } else {
            Err(ParseError::UnexpectedEof)
        }
    }

    // Parse postfix expressions (function calls, member access, indexing)
    fn parse_postfix(&self) -> ParseResult<'source, Expression<'alloc>> {
        let mut expr = self.parse_primary()?;

        loop {
            self.ignore_whitespace_and_comments();

            if let Some((Ok(token), _)) = self.peek() {
                match token {
                    // Function call: foo()
                    Tokens::LeftParen => {
                        self.advance();
                        let arguments = self.parse_arguments()?;
                        self.ignore_whitespace_and_comments();

                        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
                            self.advance();
                        } else {
                            return Err(ParseError::MissingRightParen(self.peek().unwrap().1));
                        }

                        expr = Expression::Call {
                            function: Box::new(expr),
                            arguments,
                        };
                    }
                    // Member access: foo.bar or foo:bar
                    Tokens::Dot => {
                        self.advance();
                        self.ignore_whitespace_and_comments();

                        if let Some((Ok(Tokens::Identifier(name)), _)) = self.peek() {
                            self.advance();
                            expr = Expression::IndexName {
                                operator: IndexOperator::Dot,
                                expression: Box::new(expr),
                                name,
                            };
                        } else {
                            return Err(ParseError::ExpectedIdentifier(self.peek().unwrap().1));
                        }
                    }
                    Tokens::Colon => {
                        self.advance();
                        self.ignore_whitespace_and_comments();

                        if let Some((Ok(Tokens::Identifier(name)), _)) = self.peek() {
                            self.advance();
                            expr = Expression::IndexName {
                                operator: IndexOperator::Colon,
                                expression: Box::new(expr),
                                name,
                            };
                        } else {
                            return Err(ParseError::ExpectedIdentifier(self.peek().unwrap().1));
                        }
                    }
                    _ => break,
                }
            } else {
                break;
            }
        }

        Ok(expr)
    }

    // Parse function call arguments
    fn parse_arguments(&self) -> ParseResult<'source, Expressions<'alloc>> {
        let mut arguments = Vec::new();
        self.ignore_whitespace_and_comments();

        // Check for empty argument list
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            return Ok(arguments);
        }

        loop {
            let arg = self.parse_expression()?;
            arguments.push(arg);

            self.ignore_whitespace_and_comments();

            if let Some((Ok(token), _)) = self.peek() {
                match token {
                    Tokens::Comma => {
                        self.advance();
                        continue;
                    }
                    Tokens::RightParen => break,
                    _ => return Err(ParseError::IllegalToken(self.peek().unwrap().1)),
                }
            } else {
                break;
            }
        }

        Ok(arguments)
    }

    // Get operator precedence (higher number = higher precedence)
    fn get_precedence(&self, op: &BinaryOperator) -> u8 {
        match op {
            BinaryOperator::Or => 1,
            BinaryOperator::And => 2,
            BinaryOperator::Equal | BinaryOperator::NotEqual => 3,
            BinaryOperator::LessThan
            | BinaryOperator::LessThanOrEqual
            | BinaryOperator::GreaterThan
            | BinaryOperator::GreaterThanOrEqual => 4,
            BinaryOperator::Concatenate => 5,
            BinaryOperator::Add | BinaryOperator::Subtract => 6,
            BinaryOperator::Multiply | BinaryOperator::Divide | BinaryOperator::Modulo => 7,
            BinaryOperator::Exponentiate => 8,
            BinaryOperator::Assign => 0,
        }
    }

    // Convert token to binary operator
    fn token_to_binary_op(&self, token: &Tokens<'source>) -> Option<BinaryOperator> {
        match token {
            Tokens::Plus => Some(BinaryOperator::Add),
            Tokens::Minus => Some(BinaryOperator::Subtract),
            Tokens::Asterisk => Some(BinaryOperator::Multiply),
            Tokens::Slash => Some(BinaryOperator::Divide),
            Tokens::Modulus => Some(BinaryOperator::Modulo),
            Tokens::Caret => Some(BinaryOperator::Exponentiate),
            Tokens::Equals => Some(BinaryOperator::Equal),
            Tokens::NotEqual => Some(BinaryOperator::NotEqual),
            Tokens::LessThan => Some(BinaryOperator::LessThan),
            Tokens::LessEqual => Some(BinaryOperator::LessThanOrEqual),
            Tokens::GreaterThan => Some(BinaryOperator::GreaterThan),
            Tokens::GreaterEqual => Some(BinaryOperator::GreaterThanOrEqual),
            Tokens::And => Some(BinaryOperator::And),
            Tokens::Or => Some(BinaryOperator::Or),
            Tokens::DoubleDot => Some(BinaryOperator::Concatenate),
            Tokens::Assign => Some(BinaryOperator::Assign),
            _ => None,
        }
    }

    // Parse binary operations with precedence climbing
    fn parse_binary_expression(
        &self,
        min_precedence: u8,
    ) -> ParseResult<'source, Expression<'alloc>> {
        let mut left = self.parse_postfix()?;

        loop {
            self.ignore_whitespace_and_comments();

            let (op, current_precedence) = if let Some((Ok(token), _)) = self.peek() {
                if let Some(binary_op) = self.token_to_binary_op(&token) {
                    let prec = self.get_precedence(&binary_op);
                    if prec < min_precedence {
                        break;
                    }
                    (binary_op, prec)
                } else {
                    break;
                }
            } else {
                break;
            };

            self.advance(); // consume operator

            // Right associativity for exponentiation and assignment
            let next_min_precedence = match op {
                BinaryOperator::Exponentiate | BinaryOperator::Assign => current_precedence,
                _ => current_precedence + 1,
            };

            let right = self.parse_binary_expression(next_min_precedence)?;

            left = Expression::BinaryOperation {
                operator: op,
                left: Box::new(left),
                right: Box::new(right),
            };
        }

        Ok(left)
    }

    // Parse a complete expression
    pub fn parse_expression(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.parse_binary_expression(0)
    }

    // Parse a block of code { ... }
    pub fn parse_block(&self) -> ParseResult<'source, Block<'alloc>> {
        self.ignore_whitespace_and_comments();

        let opening_span = if let Some((Ok(Tokens::LeftBrace), span)) = self.peek() {
            self.advance();
            span
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        };

        let mut expressions = Vec::new();

        loop {
            self.ignore_whitespace_and_comments();

            if let Some((Ok(Tokens::RightBrace), _)) = self.peek() {
                self.advance();
                break;
            }

            if self.at_eof() {
                return Err(ParseError::MissingClosingBrace {
                    expected: Tokens::RightBrace,
                    opening_span,
                    found: Err(crate::lexer::lex::LexerError),
                    span: self.peek().map(|t| t.1).unwrap_or(0..0),
                });
            }

            let expr = self.parse_statement()?;
            expressions.push(expr);
        }

        Ok(Block { expressions })
    }

    // Parse a statement (expression with semicolon, or control flow)
    pub fn parse_statement(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(token), _span)) = self.peek() {
            match token {
                Tokens::Local | Tokens::Global => self.parse_variable_declaration(),
                Tokens::If => self.parse_if_statement(),
                Tokens::While => self.parse_while_statement(),
                Tokens::For => self.parse_for_statement(),
                Tokens::Return => {
                    self.advance();
                    let expr = self.parse_expression()?;
                    self.expect_semicolon()?;
                    Ok(expr)
                }
                Tokens::Break | Tokens::Pass => {
                    self.advance();
                    self.expect_semicolon()?;
                    // For now, return a placeholder; proper implementation would need AST nodes
                    Ok(Expression::NullLiteral)
                }
                Tokens::Program => self.parse_program(),
                _ => {
                    let expr = self.parse_expression()?;
                    self.expect_semicolon()?;
                    Ok(expr)
                }
            }
        } else {
            Err(ParseError::UnexpectedEof)
        }
    }

    // Expect a semicolon
    fn expect_semicolon(&self) -> ParseResult<'source, ()> {
        self.ignore_whitespace_and_comments();

        if let Some((Ok(Tokens::Semicolon), _)) = self.peek() {
            self.advance();
            Ok(())
        } else {
            let span = self.peek().map(|t| t.1).unwrap_or(0..0);
            Err(ParseError::MissingSemicolon(span))
        }
    }

    // Parse variable declaration: local x -> 42;
    fn parse_variable_declaration(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.ignore_whitespace_and_comments();

        // Skip 'local' or 'global' keyword
        self.advance();

        // Parse identifier
        let name_expr = self.parse_identifier()?;
        let name = if let Expression::Identifier { value } = name_expr {
            value
        } else {
            return Err(ParseError::ExpectedIdentifier(self.peek().unwrap().1));
        };

        self.ignore_whitespace_and_comments();

        // Expect '->' (assignment operator)
        if let Some((Ok(Tokens::Assign), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        }

        // Parse value expression
        let value = self.parse_expression()?;

        self.expect_semicolon()?;

        // Return as assignment binary operation
        Ok(Expression::BinaryOperation {
            operator: BinaryOperator::Assign,
            left: Box::new(Expression::Identifier { value: name }),
            right: Box::new(value),
        })
    }

    // Parse if statement
    fn parse_if_statement(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.advance(); // consume 'if'

        self.ignore_whitespace_and_comments();

        // Expect '('
        if let Some((Ok(Tokens::LeftParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        }

        let _condition = self.parse_expression()?;

        self.ignore_whitespace_and_comments();

        // Expect ')'
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::MissingRightParen(self.peek().unwrap().1));
        }

        let then_block = self.parse_block()?;

        // For now, return the block expression
        // A complete implementation would have an If AST node
        Ok(Expression::Block { block: then_block })
    }

    // Parse while statement
    fn parse_while_statement(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.advance(); // consume 'while'

        self.ignore_whitespace_and_comments();

        // Expect '('
        if let Some((Ok(Tokens::LeftParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        }

        let _condition = self.parse_expression()?;

        self.ignore_whitespace_and_comments();

        // Expect ')'
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::MissingRightParen(self.peek().unwrap().1));
        }

        let body = self.parse_block()?;

        // For now, return the block expression
        // A complete implementation would have a While AST node
        Ok(Expression::Block { block: body })
    }

    // Parse for statement
    fn parse_for_statement(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.advance(); // consume 'for'

        self.ignore_whitespace_and_comments();

        // Expect '('
        if let Some((Ok(Tokens::LeftParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        }

        // For now, just parse the expression inside and the block
        let _init = self.parse_expression()?;

        self.ignore_whitespace_and_comments();

        // Expect ')'
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::MissingRightParen(self.peek().unwrap().1));
        }

        let body = self.parse_block()?;

        // For now, return the block expression
        // A complete implementation would have a For AST node
        Ok(Expression::Block { block: body })
    }

    // Parse program definition: program name(params) { ... }
    pub fn parse_program(&self) -> ParseResult<'source, Expression<'alloc>> {
        self.advance(); // consume 'program'

        self.ignore_whitespace_and_comments();

        // Parse program name (optional for anonymous programs)
        let name = if let Some((Ok(Tokens::Identifier(id)), _)) = self.peek() {
            self.advance();
            Some(id.to_string())
        } else {
            None
        };

        self.ignore_whitespace_and_comments();

        // Expect '('
        if let Some((Ok(Tokens::LeftParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::IllegalToken(self.peek().unwrap().1));
        }

        // Parse parameters
        let parameters = self.parse_parameters()?;

        self.ignore_whitespace_and_comments();

        // Expect ')'
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            self.advance();
        } else {
            return Err(ParseError::MissingRightParen(self.peek().unwrap().1));
        }

        // Parse body
        let body = self.parse_block()?;

        Ok(Expression::Program {
            name,
            parameters,
            body,
        })
    }

    // Parse function parameters
    fn parse_parameters(&self) -> ParseResult<'source, Bindings<'alloc>> {
        let mut parameters = Vec::new();
        self.ignore_whitespace_and_comments();

        // Check for empty parameter list
        if let Some((Ok(Tokens::RightParen), _)) = self.peek() {
            return Ok(parameters);
        }

        loop {
            self.ignore_whitespace_and_comments();

            // Parse parameter name
            let name = if let Some((Ok(Tokens::Identifier(id)), _)) = self.peek() {
                self.advance();
                id
            } else {
                return Err(ParseError::ExpectedIdentifier(self.peek().unwrap().1));
            };

            self.ignore_whitespace_and_comments();

            // Expect ':'
            if let Some((Ok(Tokens::Colon), _)) = self.peek() {
                self.advance();
            } else {
                // If no type annotation, use a default type
                parameters.push(Binding {
                    name,
                    ty: Type::Reference { name: "Any" },
                });

                self.ignore_whitespace_and_comments();

                if let Some((Ok(token), _)) = self.peek() {
                    match token {
                        Tokens::Comma => {
                            self.advance();
                        }
                        Tokens::RightParen => break,
                        _ => return Err(ParseError::IllegalToken(self.peek().unwrap().1)),
                    }
                } else {
                    break;
                }
            }

            self.ignore_whitespace_and_comments();

            // Parse type
            let ty_name = if let Some((Ok(Tokens::Identifier(id)), _)) = self.peek() {
                self.advance();
                id
            } else {
                return Err(ParseError::ExpectedIdentifier(self.peek().unwrap().1));
            };

            parameters.push(Binding {
                name,
                ty: Type::Reference { name: ty_name },
            });

            self.ignore_whitespace_and_comments();

            if let Some((Ok(token), _)) = self.peek() {
                match token {
                    Tokens::Comma => {
                        self.advance();
                        continue;
                    }
                    Tokens::RightParen => break,
                    _ => return Err(ParseError::IllegalToken(self.peek().unwrap().1)),
                }
            } else {
                break;
            }
        }

        Ok(parameters)
    }

    // Parse entire file
    pub fn parse(&self) -> ParseResult<'source, Vec<Expression<'alloc>>> {
        let mut expressions = Vec::new();

        while !self.at_eof() {
            self.ignore_whitespace_and_comments();

            if self.at_eof() {
                break;
            }

            let expr = self.parse_statement()?;
            expressions.push(expr);
        }

        Ok(expressions)
    }
}
