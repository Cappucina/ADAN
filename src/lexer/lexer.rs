use crate::lexer::token::*;

pub struct Lexer {
    input: Vec<char>,
    pos: usize,
}

impl Lexer {
    pub fn new(to_process: &str) -> Self {
        Self {
            input: to_process.chars().collect(),
            pos: 0,
        }
    }

    pub fn preview(&self) -> Option<char> {
        self.input.get(self.pos).copied()
    }

    pub fn advance(&mut self) {
        self.pos += 1
    }

    fn read_while<F>(&mut self, condition: F) -> String
    where
        F: Fn(char) -> bool,
    {
        let mut result = String::new();
        while let Some(c) = self.preview() {
            if !condition(c) {
                break;
            }
            result.push(c);
            self.advance();
        }
        result
    }

    fn skip_whitespace(&mut self) {
        self.read_while(|c| c.is_whitespace());
    }

    pub fn next_token(&mut self) -> Token {
        self.skip_whitespace();

        let c = match self.preview() {
            Some(c) => c,
            None => return Token::Error("Unexpected EOF".to_string()),
        };

        // Handle alphabetic keywords or identifiers
        if c.is_alphabetic() {
            let word = self.read_while(|ch| ch.is_alphanumeric() || ch == '_');
            return match word.as_str() {
                "include" => Token::Keyword(Keyword::Include),
                "local" => Token::Keyword(Keyword::Local),
                "global" => Token::Keyword(Keyword::Global),
                "program" => Token::Keyword(Keyword::Program),
                "if" => Token::Keyword(Keyword::If),
                "while" => Token::Keyword(Keyword::While),
                "else" => Token::Keyword(Keyword::Else),
                "return" => Token::Keyword(Keyword::Return),

                "String" => Token::Types(Types::String),
                "Boolean" => Token::Types(Types::Boolean),
                "Char" => Token::Types(Types::Char),
                "Array" => Token::Types(Types::Array),
                "Object" => Token::Types(Types::Object),

                "i8" => Token::Types(Types::i8),
                "i32" => Token::Types(Types::i32),
                "i64" => Token::Types(Types::i64),
                "u8" => Token::Types(Types::u8),
                "u32" => Token::Types(Types::u32),
                "u64" => Token::Types(Types::u64),
                "f32" => Token::Types(Types::f32),
                "f64" => Token::Types(Types::f64),

                _ => Token::Ident(word),
            };
        }

        if c.is_digit(10) {
            let number = self.read_while(|ch| ch.is_digit(10));
            return Token::Number(number);
        }

        let next = self.input.get(self.pos + 1).copied();
        if c == '-' && next == Some('>') {
            self.advance();
            self.advance();
            return Token::Keyword(Keyword::Assign);
        }

        if c == '/' && next == Some('/') {
            self.advance();
            self.advance();
            self.read_while(|ch| ch != '\n');
            return self.next_token();
        }

        if c == '/' && next == Some('*') {
            self.advance();
            self.advance();
            while let Some(ch) = self.preview() {
                if ch == '*' && self.input.get(self.pos + 1) == Some(&'/') {
                    self.advance();
                    self.advance();
                    break;
                }
                self.advance();
            }
            return self.next_token();
        }

        if c == '>' && next == Some('=') {
            self.advance();
            self.advance();
            return Token::Symbols(Symbols::Gequal);
        }
        
        if c == '<' && next == Some('=') {
            self.advance();
            self.advance();
            return Token::Symbols(Symbols::Lequal);
        }

        if c == '"' {
            self.advance();
            
            let mut string_content = String::new();
            while let Some(ch) = self.preview() {
                if ch == '"' {
                    self.advance(); // us closing "
                    break;
                }

                if ch == '\\' {
                    self.advance();
                    match self.preview() {
                        Some('n') => string_content.push('\n'),
                        Some('t') => string_content.push('\t'),
                        Some('"') => string_content.push('"'),
                        Some('\\') => string_content.push('\\'),
                        Some(other) => string_content.push(other), // Push the string literally
                                                                   // like \q if they aren't
                                                                   // supported.
                        None => break,
                    }
                    self.advance();
                } else {
                    string_content.push(ch);
                    self.advance();
                }

                //self.advance();
            }

            return Token::Literal(string_content);
        }

        if c == '\'' {
            self.advance();
            if let Some(ch) = self.preview() {
                self.advance();
                if self.preview() == Some ('\'') {
                    self.advance();
                    return Token::CharLiteral(ch);
                } else {
                    return Token::Error("Unterminated char literal".to_string());
                }
            }
        }

        self.advance();
        match c {
            ';' => Token::Symbols(Symbols::SemiColon),
            ':' => Token::Symbols(Symbols::Colon),
            '(' => Token::Symbols(Symbols::LParen),
            ')' => Token::Symbols(Symbols::RParen),
            '{' => Token::Symbols(Symbols::LCurlyBracket),
            '}' => Token::Symbols(Symbols::RCurlyBracket),
            '\'' => Token::Symbols(Symbols::SingleQuote),
            '.' => Token::Symbols(Symbols::Period),
            ',' => Token::Symbols(Symbols::Comma),
            '=' => Token::Symbols(Symbols::Equal),
            '>' => Token::Symbols(Symbols::Greater),
            '<' => Token::Symbols(Symbols::Lesser),
            '+' => Token::Symbols(Symbols::Add),
            '-' => Token::Symbols(Symbols::Sub),
            '*' => Token::Symbols(Symbols::Mul),
            '/' => Token::Symbols(Symbols::Div),
            '%' => Token::Symbols(Symbols::Mod),
            _ => Token::Error(format!("Unexpected char: {}", c)),
        }
    }

    pub fn tokenize(&mut self) -> Result<Vec<Token>, String> {
        let mut tokens = Vec::new();
        loop {
            let tok = self.next_token();
            if let Token::Error(e) = &tok {
                if e == "Unexpected EOF" {
                    break;
                }
            }
            tokens.push(tok);
        }
        Ok(tokens)
    }
}
