#[cfg(test)]
mod tests {
    use crate::lexer::{
        tokens::Tokens,
        lex::Lexer
    };

    fn next_non_whitespace<'source>(lex: &mut Lexer<'source>) -> Option<Tokens<'source>> {
        while let Some(token) = lex.next() {
            match token {
                Tokens::HorizontalWhitespace(_) | Tokens::VerticalWhitespace(_) => continue,
                _ => return Some(token),
            }
        }
        None
    }

    #[test]
    fn operands() {
        let input = "+ - * / % ^ = ! && || < > <= >= != ( ) { } ; , . //";
        let mut lex = Lexer::new(input);

        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Plus));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Minus));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Asterisk));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Slash));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Percent));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Caret));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Equals));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Not));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::And));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Or));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::LessThan));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::GreaterThan));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::LessEqual));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::GreaterEqual));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::NotEqual));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::LeftParen));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::RightParen));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::LeftBrace));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::RightBrace));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Semicolon));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Comma));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Dot));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Comment("//".into())));
    }

    #[test]
    fn keywords() {
        let input = "include local global if else while for program return break pass ->";
        let mut lex = Lexer::new(input);

        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Include));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Local));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Global));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::If));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Else));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::While));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::For));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Program));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Return));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Break));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Pass));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Assign));
    }

    #[test]
    fn literals() {
        let input = "foo SAMPLE _var \"Escaped \\\"String\\\"\" \"String Literal\" 64 -64 0.0 -1.0 true false null";
        let mut lex = Lexer::new(input);

        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Identifier("foo".into())));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Identifier("SAMPLE".into())));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Identifier("_var".into())));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::String("Escaped \"String\"".into())));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::String("String Literal".into())));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Integer(64)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Integer(-64)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Float(0.0)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Float(-1.0)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Boolean(true)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Boolean(false)));
        assert_eq!(next_non_whitespace(&mut lex), Some(Tokens::Null));
    }
}
