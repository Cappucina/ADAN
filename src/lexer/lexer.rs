use crate::lexer::token::*;

pub struct Lexer {
    input: Vec<char>, // The character being tokenized.
    pos: usize,       // A pointer which directs to the tokenized character.
    
    // ['l', 'o', 'c', 'a', 'l']
    //  ^^^            ^^^
    //   0              3
}

impl Lexer {
    pub fn new(to_process: &str) -> Self {
        Self {
            input: to_process.chars().collect(), // Convert to Vec<char>
            pos: 0
        }
    }

    // Preview the next character without moving bits
    pub fn preview(&self) -> Option(char) {
        self.input.get(self.pos).copied()
    }

    pub fn advance(&mut self) {
        self.pos += 1
    }

    fn read_while<F>(&mut self, condition: F) -> String
    where 
        F: Fn(char) -> bool,
    {
        let mut result = String::from("");
        
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

    // Searches and tokenizes everything until it reaches the EOF.
    pub fn next_token(&mut self) -> Token {
        self.skip_whitespace();

        let c = match self.preview() { // Searches for `char`, returns `None` if not found.
            Some(c) => c,
            None => return Token::EOF,
        };

        if c.is_alphabetic() {
            let word = self.read_while(|ch| ch.is_alphanumeric() || ch == '_');

            return match word.as_str() {
                "local" => Token::Keyword(Keyword::Local),
                "global" => Token::Keyword(Keyword::Global),

                "String" => Token::Types(Types::String),
                "Boolean" => Token::Types(Types::Boolean),
                "Char" => Token::Types(Types::Char),
                "Array" => Token::Types(Types::Array),
                "Object" => Token::Types(Types::Object),

                "i8" | "i32" | "i64" |
                "u8" | "u32" | "u64" |
                       "f32" | "f64" => {
                    
                    let t = match word.as_str() {
                         "i8" => Types::i8,
                        "i32" => Types::i32,
                        "i64" => Types::i64,
                        
                         "u8" => Types::u8,
                        "u32" => Types::u32,
                        "u64" => Types::u64,
                        
                        "f32" => Types::f32,
                        "f64" => Types::f64,
                        _ => unreachable!(),
                    };
                    
                    Token::Types(t)
                }
                _ => Token::Ident(word), }; // Anything the lexer cannot tokenize go here.
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
        } else if c == '/' && next == Some('/') {
            self.advance();
            self.advance();
            
            return Token::Symbols(Symbols::Comment);
        } else if c == '/' && next == Some('*') {
            self.advance();
            self.advance();
            
            return Token::Symbols(Symbols::MultiLine);
        } else if c == '*' && next == Some('/') {
            self.advance();
            self.advance();
        
            return Token::Symbols(Symbols::MultiLine);
        }
        
        self.advance();
        match c {
             ';' => Token::Keyword(Keyword::SemiColon),
             ':' => Token::Keyword(Keyword::Colon),
            
             '(' => Token::Symbols(Symbols::LParen),
             ')' => Token::Symbols(Symbols::RParen),
             '{' => Token::Symbols(Symbols::LCurlyBracket),
             '}' => Token::Symbols(Symbols::RCurlyBracket),
             '"' => Token::Symbols(Symbols::Quotation),
            '\'' => Token::Symbols(Symbols::SingleQuote),
             '.' => Token::Symbols(Symbols::Period),

            _ => Token::Error(format!("Unexpected char: {}", c)),
        }
    }
}
