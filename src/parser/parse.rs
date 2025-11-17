use std::vec;

use crate::{lexer::tokens::Tokens, parser::ast::{Expr, Statement}};

pub struct Parser {
    tokens: Vec<Tokens>,
    position: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Tokens>) -> Self {
        Parser {
            tokens,
            position: 0,
        }
    }

    fn peek(&self) -> Option<&Tokens> {
        self.tokens.get(self.position)
    }

    fn advance(&mut self) -> Option<&Tokens> {
        if self.position < self.tokens.len() {
            let token = &self.tokens[self.position];
            self.position += 1;
            Some(token)
        } else {
            None
        }
    }

    fn expect(&mut self, expected: &Tokens) -> Result<(), String> {
        match self.advance() {
            Some(token) if token == expected => Ok(()),
            Some(token) => Err(format!("Expected {:?}, found {:?}", expected, token)),
            None => Err(format!("Expected {:?}, found end of input", expected)),
        }
    }

    fn expect_identifier(&mut self) -> Result<String, String> {
        match self.advance() {
            Some(Tokens::Identifier(name)) => Ok(name.clone()),
            Some(token) => Err(format!("Expected identifier, found {:?}", token)),
            None => Err("Expected identifier, found end of input".to_string()),
        }
    }

    fn parse(&mut self) -> Result<Vec<Statement>, String> {
        let mut statements = vec![];
        while let Some(_) = self.peek() {
            let stmt = self.parse_statement()?;
            statements.push(stmt);
        }

        Ok(statements)
    }

    fn parse_statement(&mut self) -> Result<Statement, String> {
        while let Some(Tokens::Comment) = self.peek() {
            self.advance();
        }

        let expression = self.parse_expression();
        self.expect(&Tokens::Semicolon);

        Ok(Statement::Expr(expression))
    }

    fn parse_expression(&mut self) -> Result<Expr, String> {
        
    }
}