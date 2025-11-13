use crate::lexer::token::*;
use crate::parser::ast::*;

pub struct Parser {
    tokens: Vec<Token>,
    pos: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, pos: 0 }
    }

    fn peek(&self) -> Option<&Token> {
        self.tokens.get(self.pos)
    }

    fn next(&mut self) -> Option<&Token> {
        if self.pos < self.tokens.len() {
            let tok = &self.tokens[self.pos];
            self.pos += 1;
            Some(tok)
        } else {
            None
        }
    }

    fn match_keyword(&mut self, kw: Keyword) -> bool {
        let peeked = self.peek();
        //println!("match_keyword: looking for {:?}, peeked: {:?}", kw, peeked);

        if matches!(peeked, Some(Token::Keyword(k)) if *k == kw) {
            self.pos += 1;
            //println!("match_keyword: matched {:?}", kw);
            true
        } else {
            //println!("match_keyword: did not match {:?}", kw);
            false
        }
    }

    fn match_symbol(&mut self, sym: Symbols) -> bool {
        if matches!(self.peek(), Some(Token::Symbols(s)) if *s == sym) {
            self.pos += 1;
            true
        } else {
            false
        }
    }

    fn expect_symbol(&mut self, sym: Symbols) -> Result<(), String> {
        //println!("testing symbol: {:?}", sym);
        if self.match_symbol(sym) {
            Ok(())
        } else {
            Err(format!("Expected symbol {:?}", sym))
        }
    }

    fn expect_keyword(&mut self, kw: Keyword) -> Result<(), String> {
        //println!("testing keyword: {:?}", kw);
        if self.match_keyword(kw) {
            Ok(())
        } else {
            Err(format!("Expected keyword {:?}", kw))
        }
    }

    fn expect_ident(&mut self) -> Result<String, String> {
        match self.next() {
            Some(Token::Ident(id)) => Ok(id.clone()),
            other => Err(format!("Expected identifier, got {:?}", other)),
        }
    }

    // ------------------------
    // Top-level parsing
    // ------------------------
    pub fn parse(&mut self) -> Result<Vec<Statement>, String> {
        let mut stmts = vec![];
        //if self.match_keyword(Keyword::Program) {
        //    stmts.push(self.parse_functions()?);
        //} else {
        //    return Err("Expected top-level 'program -> main'".into());
        //}

        while self.peek().is_some() {
            stmts.push(self.parse_statement()?);
        }
        
        Ok(stmts)
    }

    // ------------------------
    // Statements
    // ------------------------
    fn parse_statement(&mut self) -> Result<Statement, String> {
        while let Some(Token::Symbols(Symbols::Comment)) = self.peek() {
            self.next();
        }
        if self.match_keyword(Keyword::Include) {
            return self.parse_include();
        }
        if self.match_keyword(Keyword::Local) || self.match_keyword(Keyword::Global) {
            return self.parse_var_decl();
        }
        if self.match_keyword(Keyword::While) {
            return self.parse_while_loops();
        }
        if self.match_keyword(Keyword::If) {
            return self.parse_if_statement();
        }
        if self.match_keyword(Keyword::Program) {
            return self.parse_functions();
        }
        if self.match_keyword(Keyword::Return) {
            return self.parse_return();
        }
        if self.match_symbol(Symbols::LCurlyBracket) {
            return Ok(Statement::Block(self.parse_block()?));
        }
        let expr = self.parse_expr()?;
        //self.expect_symbol(Symbols::SemiColon)?;
        if !matches!(expr, Expr::Block(_)) {
            self.expect_symbol(Symbols::SemiColon)?;
        }
        Ok(Statement::Expression(expr))
    }

    fn parse_include(&mut self) -> Result<Statement, String> {
        // self.expect_keyword(Keyword::Include)?;

        let mut path = String::new();
        loop {
            match self.next().ok_or("Unexpected EOF in include")? {
                Token::Ident(s) => path.push_str(&s),
                Token::Symbols(Symbols::Period) => path.push('.'),
                Token::Symbols(Symbols::SemiColon) => break,
                tok => return Err(format!("Unexpected token in include: {:?}", tok)),
            }
        }

        Ok(Statement::Include(path))
    }

    fn parse_return(&mut self) -> Result<Statement, String> {
        //self.expect_keyword(Keyword::Return)?;
        let value = if !self.match_symbol(Symbols::SemiColon) {
            Some(self.parse_expr()?)
        } else {
            None
        };

        self.expect_symbol(Symbols::SemiColon)?;
        Ok(Statement::Return { value })
    }

    fn parse_functions(&mut self) -> Result<Statement, String> {
        self.expect_keyword(Keyword::Assign)?;
        
        let name = self.expect_ident()?;
        let mut params = Vec::new();
        if self.match_symbol(Symbols::LParen) {
            while !self.match_symbol(Symbols::RParen) {
                let param_name = self.expect_ident()?;
                self.expect_symbol(Symbols::Colon)?;
                if let Some(Token::Types(_ty)) = self.peek().cloned() {
                    self.next();
                } else {
                    return Err("Expected type after ':' in function parameter".to_string());
                }
        
                self.match_symbol(Symbols::Comma);
                params.push(param_name);
            }
        }
        
        let body = self.parse_block()?;
        Ok(Statement::Function(FunctionDecl { name, params, body }))
    }

    fn parse_while_loops(&mut self) -> Result<Statement, String> {
        //self.expect_keyword(Keyword::While)?;
        self.expect_symbol(Symbols::LParen)?;

        let condition = self.parse_expr()?;

        self.expect_symbol(Symbols::RParen)?;

        let body = if self.match_symbol(Symbols::LCurlyBracket) {
            Statement::Block(self.parse_block()?).into()
        } else {
            Box::new(self.parse_statement()?)
        };

        Ok(Statement::While { condition, body })
    }

    fn parse_if_statement(&mut self) -> Result<Statement, String> {
        //self.expect_keyword(Keyword::If)?;
        //println!("a");
        self.expect_symbol(Symbols::LParen)?;
       
        let condition = self.parse_expr()?;
        //println!("condition parsed: {:?}", condition);
       
        self.expect_symbol(Symbols::RParen)?;
        //self.expect_symbol(Symbols::LCurlyBracket)?;

        let then_branch = Box::new(Statement::Block(self.parse_block()?));
        let else_branch = if self.match_keyword(Keyword::Else) { // Use match_keyword here for
                                                                // optional else handling.
            //self.expect_symbol(Symbols::LCurlyBracket)?;
            // else {
            Some(Box::new(Statement::Block(self.parse_block()?)))
        } else {
            None
        };

        //println!("{:?}", then_branch);
        Ok(Statement::If { condition, then_branch, else_branch })
    }

    fn parse_block(&mut self) -> Result<Vec<Statement>, String> {
        self.expect_symbol(Symbols::LCurlyBracket)?;
        
        let mut stmts = Vec::new();
        while !self.match_symbol(Symbols::RCurlyBracket) {
            stmts.push(self.parse_statement()?);
        }

        Ok(stmts)
    }

    fn parse_var_decl(&mut self) -> Result<Statement, String> {
        let name = self.expect_ident()?;
        self.expect_symbol(Symbols::Colon)?;
        let var_type = if let Some(Token::Types(t)) = self.peek().cloned() {
            self.next();
            Some(t)
        } else {
            None
        };

        let initializer = if self.match_keyword(Keyword::Assign) {
            Some(self.parse_expr()?)
        } else {
            None
        };

        self.expect_symbol(Symbols::SemiColon)?;
        Ok(Statement::VarDecl { name, var_type, initializer })
    }

    // PEMDAS RULING
    // parse_primary, parse_unary, parse_mul_div_mod,
    // parse_add_sub, parse_comparisons, parse_equality,

    fn parse_equality(&mut self) -> Result<Expr, String> {
        let mut left = self.parse_comparisons()?;
        while let Some(Token::Symbols(Symbols::Equal)) = self.peek() {
            self.next();
            
            let right = self.parse_comparisons()?;
            left = Expr::Binary { left: Box::new(left), op: Operation::Equal, right: Box::new(right) };
        }

        Ok(left)
    }

    fn parse_comparisons(&mut self) -> Result<Expr, String> {
        let mut left = self.parse_add_sub()?;
        while let Some(tok) = self.peek() {
            let op = match tok {
                Token::Symbols(Symbols::Greater) => Operation::Greater,
                Token::Symbols(Symbols::Lesser) => Operation::Lesser,
                Token::Symbols(Symbols::Gequal) => Operation::Gequal,
                Token::Symbols(Symbols::Lequal) => Operation::Lequal,
                _ => break,
            };
            self.next();

            let right = self.parse_add_sub()?;
            left = Expr::Binary { left: Box::new(left), op, right: Box::new(right) };
        }

        Ok(left)
    }

    fn parse_add_sub(&mut self) -> Result<Expr, String> {
        let mut left = self.parse_mul_div_mod()?;
        while let Some(tok) = self.peek() {
            let op = match tok {
                Token::Symbols(Symbols::Add) => Operation::Add,
                Token::Symbols(Symbols::Sub) => Operation::Subtract,
                _ => break,
            };
            self.next();

            let right = self.parse_mul_div_mod()?;
            left = Expr::Binary { left: Box::new(left), op, right: Box::new(right) };
        }

        Ok(left)
    }

    fn parse_mul_div_mod(&mut self) -> Result<Expr, String> {
        let mut left = self.parse_unary()?;
        while let Some(tok) = self.peek() {
            let op = match tok {
                Token::Symbols(Symbols::Mul) => Operation::Multiply,
                Token::Symbols(Symbols::Div) => Operation::Divide,
                Token::Symbols(Symbols::Mod) => Operation::Modulo,
                _ => break,
            };
            self.next();

            let right = self.parse_unary()?;
            left = Expr::Binary { left: Box::new(left), op, right: Box::new(right) };
        }

        Ok(left)
    }

    fn parse_unary(&mut self) -> Result<Expr, String> {
        let next_token = self.peek();
        match next_token {
            Some(Token::Symbols(Symbols::Sub)) => {
                self.next();
                let right = self.parse_unary()?;
                Ok(Expr::Unary { op: Operation::Negate, right: Box::new(right) })
            }
            Some(Token::Symbols(Symbols::Not)) => {
                self.next();
                let right = self.parse_unary()?;
                Ok(Expr::Unary { op: Operation::Not, right: Box::new(right) })
            }
            _ => self.parse_primary(),
        }
    }

    fn parse_primary(&mut self) -> Result<Expr, String> {
        match self.peek().cloned() {
            Some(Token::Number(n)) => {
                let value = n.parse::<f64>().unwrap_or(0.0);
                self.next();
                Ok(Expr::Literal(Literal::Number(value)))
            }
            Some(Token::Literal(s)) => {
                self.next();
                Ok(Expr::Literal(Literal::String(s)))
            }
            Some(Token::CharLiteral(c)) => {
                self.next();
                Ok(Expr::Literal(Literal::Char(c)))
            }
            Some(Token::Ident(name)) => {
                let mut base = name.clone();
                self.next();
                while self.match_symbol(Symbols::Period) {
                    let member = self.expect_ident()?;
                    base = format!("{}.{}", base, member);
                }

                if self.match_symbol(Symbols::LParen) { // (
                    let mut args = Vec::new();
                    if !self.match_symbol(Symbols::RParen) {
                        loop {
                            args.push(self.parse_expr()?);
                            if self.match_symbol(Symbols::RParen) {
                                break;
                            }

                            self.expect_symbol(Symbols::Comma)?; // (<param_1>, <param_2>, ...)
                        }
                    }

                    Ok(Expr::FCall { callee: base, args })
                } else {
                    Ok(Expr::Variable { var_name: base, var_type: None }) // `if (<var>) {}` instead of `if (<var>: String) {}`
                }
            }
            Some(Token::Symbols(Symbols::LParen)) => { // Opening part of the if statement
                self.next();
                let expr = self.parse_expr()?;
                self.expect_symbol(Symbols::RParen)?;
                Ok(expr)
            }
            other => Err(format!("Unexpected token in primary expression: {:?}", other))
        }
    }

    // ------------------------
    // Expressions
    // ------------------------
    fn parse_expr(&mut self) -> Result<Expr, String> {
        //self.parse_term()
        self.parse_equality()
    }

    fn parse_factor(&mut self) -> Result<Expr, String> {
        self.parse_unary()
    }
}
