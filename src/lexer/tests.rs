#[cfg(test)]
mod tests {
    use logos::Logos;

    use crate::lexer::tokens::Tokens;

    #[test]
    fn operands() {
        let input = "+ - * / % ^ = ! && || < > <= >= != ( ) { } ; , . //";
        let mut lex = Tokens::lexer(input);

        assert_eq!(lex.next(), Some(Ok(Tokens::Plus)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Minus)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Asterisk)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Slash)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Percent)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Caret)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Equals)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Not)));
        assert_eq!(lex.next(), Some(Ok(Tokens::And)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Or)));
        assert_eq!(lex.next(), Some(Ok(Tokens::LessThan)));
        assert_eq!(lex.next(), Some(Ok(Tokens::GreaterThan)));
        assert_eq!(lex.next(), Some(Ok(Tokens::LessEqual)));
        assert_eq!(lex.next(), Some(Ok(Tokens::GreaterEqual)));
        assert_eq!(lex.next(), Some(Ok(Tokens::NotEqual)));
        assert_eq!(lex.next(), Some(Ok(Tokens::LeftParen)));
        assert_eq!(lex.next(), Some(Ok(Tokens::RightParen)));
        assert_eq!(lex.next(), Some(Ok(Tokens::LeftBrace)));
        assert_eq!(lex.next(), Some(Ok(Tokens::RightBrace)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Semicolon)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Comma)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Dot)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Comment)));
    }

    #[test]
    fn keywords() {
        let input = "local global if else while for program return break pass ->";
        let mut lex = Tokens::lexer(input);

        assert_eq!(lex.next(), Some(Ok(Tokens::Local)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Global)));
        assert_eq!(lex.next(), Some(Ok(Tokens::If)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Else)));
        assert_eq!(lex.next(), Some(Ok(Tokens::While)));
        assert_eq!(lex.next(), Some(Ok(Tokens::For)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Program)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Return)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Break)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Pass)));
        assert_eq!(lex.next(), Some(Ok(Tokens::Assign)));
    }

    #[test]
    fn literals() {
        let input = "foo SAMPLE _var \"Escaped \\\"String\\\"\" \"String Literal\" 64 -64 0.0 -1.0 true false null";
        let mut lex = Tokens::lexer(input);

        assert_eq!(lex.next(), Some(Ok(Tokens::Identifier("foo".into()))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Identifier("SAMPLE".into()))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Identifier("_var".into()))));
        assert_eq!(lex.next(), Some(Ok(Tokens::String("Escaped \"String\"".into()))));
        assert_eq!(lex.next(), Some(Ok(Tokens::String("String Literal".into()))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Integer(64))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Integer(-64))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Float(0.0))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Float(-1.0))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Boolean(true))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Boolean(false))));
        assert_eq!(lex.next(), Some(Ok(Tokens::Null)));
    }
}