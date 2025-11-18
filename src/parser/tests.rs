#[cfg(test)]
mod parser_tests {
    use crate::lexer::lex::Lexer;
    use crate::parser::ast::{BinaryOperator, Expression, IndexOperator};
    use crate::parser::parse::Parser;
    use bumpalo::Bump;

    #[test]
    fn test_parse_integer_literal() {
        let allocator = Bump::new();
        let source = "42;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::IntegerLiteral { value } => assert_eq!(*value, 42),
            _ => panic!("Expected integer literal, got {:?}", expressions[0]),
        }
    }

    #[test]
    fn test_parse_float_literal() {
        let allocator = Bump::new();
        let source = "3.14;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::FloatLiteral { value } => assert_eq!(*value, 3.14),
            _ => panic!("Expected float literal"),
        }
    }

    #[test]
    fn test_parse_string_literal() {
        let allocator = Bump::new();
        let source = r#""hello world";"#;
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::StringLiteral { value } => assert_eq!(*value, "hello world"),
            _ => panic!("Expected string literal"),
        }
    }

    #[test]
    fn test_parse_boolean_true() {
        let allocator = Bump::new();
        let source = "true;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BooleanLiteral { value } => assert_eq!(*value, true),
            _ => panic!("Expected boolean literal"),
        }
    }

    #[test]
    fn test_parse_boolean_false() {
        let allocator = Bump::new();
        let source = "false;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BooleanLiteral { value } => assert_eq!(*value, false),
            _ => panic!("Expected boolean literal"),
        }
    }

    #[test]
    fn test_parse_null() {
        let allocator = Bump::new();
        let source = "null;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::NullLiteral => {}
            _ => panic!("Expected null literal"),
        }
    }

    #[test]
    fn test_parse_identifier() {
        let allocator = Bump::new();
        let source = "my_variable;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Identifier { value } => assert_eq!(*value, "my_variable"),
            _ => panic!("Expected identifier"),
        }
    }

    #[test]
    fn test_parse_binary_operation_add() {
        let allocator = Bump::new();
        let source = "2 + 3;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => {
                assert_eq!(*operator, BinaryOperator::Add);
                match **left {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 2),
                    _ => panic!("Expected integer literal for left operand"),
                }
                match **right {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 3),
                    _ => panic!("Expected integer literal for right operand"),
                }
            }
            _ => panic!("Expected binary operation"),
        }
    }

    #[test]
    fn test_parse_binary_operation_multiply() {
        let allocator = Bump::new();
        let source = "4 * 5;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BinaryOperation { operator, .. } => {
                assert_eq!(*operator, BinaryOperator::Multiply);
            }
            _ => panic!("Expected binary operation"),
        }
    }

    #[test]
    fn test_parse_operator_precedence() {
        let allocator = Bump::new();
        let source = "2 + 3 * 4;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        // Should parse as: 2 + (3 * 4)
        match &expressions[0] {
            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => {
                assert_eq!(*operator, BinaryOperator::Add);
                match **left {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 2),
                    _ => panic!("Expected integer literal for left"),
                }
                match **right {
                    Expression::BinaryOperation { operator, .. } => {
                        assert_eq!(operator, BinaryOperator::Multiply);
                    }
                    _ => panic!("Expected binary operation for right"),
                }
            }
            _ => panic!("Expected binary operation"),
        }
    }

    #[test]
    fn test_parse_function_call() {
        let allocator = Bump::new();
        let source = "foo();";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Call {
                function,
                arguments,
            } => {
                match **function {
                    Expression::Identifier { value } => assert_eq!(value, "foo"),
                    _ => panic!("Expected identifier for function"),
                }
                assert_eq!(arguments.len(), 0);
            }
            _ => panic!("Expected function call"),
        }
    }

    #[test]
    fn test_parse_function_call_with_args() {
        let allocator = Bump::new();
        let source = "add(1, 2);";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Call {
                function,
                arguments,
            } => {
                match **function {
                    Expression::Identifier { value } => assert_eq!(value, "add"),
                    _ => panic!("Expected identifier for function"),
                }
                assert_eq!(arguments.len(), 2);
            }
            _ => panic!("Expected function call"),
        }
    }

    #[test]
    fn test_parse_member_access() {
        let allocator = Bump::new();
        let source = "io.printf;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::IndexName {
                operator,
                expression,
                name,
            } => {
                assert_eq!(*operator, IndexOperator::Dot);
                assert_eq!(*name, "printf");
                match **expression {
                    Expression::Identifier { value } => assert_eq!(value, "io"),
                    _ => panic!("Expected identifier"),
                }
            }
            _ => panic!("Expected member access"),
        }
    }

    #[test]
    fn test_parse_variable_declaration() {
        let allocator = Bump::new();
        let source = "local x -> 42;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => {
                assert_eq!(*operator, BinaryOperator::Assign);
                match **left {
                    Expression::Identifier { value } => assert_eq!(value, "x"),
                    _ => panic!("Expected identifier"),
                }
                match **right {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 42),
                    _ => panic!("Expected integer literal"),
                }
            }
            _ => panic!("Expected assignment"),
        }
    }

    #[test]
    fn test_parse_program() {
        let allocator = Bump::new();
        let source = "program main() { 42; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Program {
                name,
                parameters,
                body,
            } => {
                assert_eq!(name.as_deref(), Some("main"));
                assert_eq!(parameters.len(), 0);
                assert_eq!(body.expressions.len(), 1);
            }
            _ => panic!("Expected program"),
        }
    }

    #[test]
    fn test_parse_program_with_parameters() {
        let allocator = Bump::new();
        let source = "program add(a: Integer, b: Integer) { return a + b; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Program {
                name,
                parameters,
                body,
            } => {
                assert_eq!(name.as_deref(), Some("add"));
                assert_eq!(parameters.len(), 2);
                assert_eq!(parameters[0].name, "a");
                assert_eq!(parameters[1].name, "b");
                assert_eq!(body.expressions.len(), 1);
            }
            _ => panic!("Expected program"),
        }
    }

    #[test]
    fn test_parse_block() {
        let allocator = Bump::new();
        let source = "{ 1; 2; 3; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse_block();
        assert!(result.is_ok());

        let block = result.unwrap();
        assert_eq!(block.expressions.len(), 3);
    }

    #[test]
    fn test_parse_multiple_statements() {
        let allocator = Bump::new();
        let source = "local x -> 1; local y -> 2; x + y;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 3);
    }

    #[test]
    fn test_parse_concatenate_operator() {
        let allocator = Bump::new();
        let source = r#""hello" .. " world";"#;
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BinaryOperation { operator, .. } => {
                assert_eq!(*operator, BinaryOperator::Concatenate);
            }
            _ => panic!("Expected concatenate operation"),
        }
    }

    #[test]
    fn test_parse_if_statement() {
        let allocator = Bump::new();
        let source = "if (x > 5) { 42; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Block { block } => {
                assert_eq!(block.expressions.len(), 1);
            }
            _ => panic!("Expected block expression"),
        }
    }

    #[test]
    fn test_parse_while_statement() {
        let allocator = Bump::new();
        let source = "while (x < 10) { x -> x + 1; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Block { block } => {
                assert_eq!(block.expressions.len(), 1);
            }
            _ => panic!("Expected block expression"),
        }
    }

    #[test]
    fn test_parse_for_statement() {
        let allocator = Bump::new();
        let source = "for (i) { i; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Block { .. } => {}
            _ => panic!("Expected block expression"),
        }
    }

    #[test]
    fn test_parse_return_statement() {
        let allocator = Bump::new();
        let source = "return 42;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);
    }

    #[test]
    fn test_parse_parenthesized_expression() {
        let allocator = Bump::new();
        let source = "(2 + 3) * 4;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        // Should parse as: (2 + 3) * 4
        match &expressions[0] {
            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => {
                assert_eq!(*operator, BinaryOperator::Multiply);
                match **left {
                    Expression::BinaryOperation {
                        operator: BinaryOperator::Add,
                        ..
                    } => {}
                    _ => panic!("Expected addition in left operand"),
                }
                match **right {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 4),
                    _ => panic!("Expected integer literal for right operand"),
                }
            }
            _ => panic!("Expected binary operation"),
        }
    }

    #[test]
    fn test_parse_colon_member_access() {
        let allocator = Bump::new();
        let source = "obj:method;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::IndexName {
                operator,
                expression,
                name,
            } => {
                assert_eq!(*operator, IndexOperator::Colon);
                assert_eq!(*name, "method");
                match **expression {
                    Expression::Identifier { value } => assert_eq!(value, "obj"),
                    _ => panic!("Expected identifier"),
                }
            }
            _ => panic!("Expected colon member access"),
        }
    }

    #[test]
    fn test_parse_chained_member_access() {
        let allocator = Bump::new();
        let source = "a.b.c;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::IndexName { name, .. } => {
                assert_eq!(*name, "c");
            }
            _ => panic!("Expected member access"),
        }
    }

    #[test]
    fn test_parse_nested_function_calls() {
        let allocator = Bump::new();
        let source = "foo(bar(42));";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Call {
                function,
                arguments,
            } => {
                assert_eq!(arguments.len(), 1);
                match **function {
                    Expression::Identifier { value } => assert_eq!(value, "foo"),
                    _ => panic!("Expected identifier for function"),
                }
                match &arguments[0] {
                    Expression::Call { .. } => {}
                    _ => panic!("Expected nested call in argument"),
                }
            }
            _ => panic!("Expected function call"),
        }
    }

    #[test]
    fn test_parse_complex_precedence() {
        let allocator = Bump::new();
        let source = "2 + 3 * 4 - 5 / 2;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        // Should parse as: (2 + (3 * 4)) - (5 / 2)
        match &expressions[0] {
            Expression::BinaryOperation {
                operator: BinaryOperator::Subtract,
                left,
                ..
            } => match **left {
                Expression::BinaryOperation {
                    operator: BinaryOperator::Add,
                    ..
                } => {}
                _ => panic!("Expected addition in left operand"),
            },
            _ => panic!("Expected subtraction at top level"),
        }
    }

    #[test]
    fn test_parse_comparison_operators() {
        let allocator = Bump::new();
        let source = "x < 5 && y >= 10;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::BinaryOperation {
                operator: BinaryOperator::And,
                left,
                right,
            } => {
                match **left {
                    Expression::BinaryOperation {
                        operator: BinaryOperator::LessThan,
                        ..
                    } => {}
                    _ => panic!("Expected less than in left operand"),
                }
                match **right {
                    Expression::BinaryOperation {
                        operator: BinaryOperator::GreaterThanOrEqual,
                        ..
                    } => {}
                    _ => panic!("Expected greater than or equal in right operand"),
                }
            }
            _ => panic!("Expected logical AND operation"),
        }
    }

    #[test]
    fn test_parse_exponentiation_right_associative() {
        let allocator = Bump::new();
        let source = "2 ^ 3 ^ 2;";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        // Should parse as: 2 ^ (3 ^ 2) (right associative)
        match &expressions[0] {
            Expression::BinaryOperation {
                operator: BinaryOperator::Exponentiate,
                left,
                right,
            } => {
                match **left {
                    Expression::IntegerLiteral { value } => assert_eq!(value, 2),
                    _ => panic!("Expected integer literal for left"),
                }
                match **right {
                    Expression::BinaryOperation {
                        operator: BinaryOperator::Exponentiate,
                        ..
                    } => {}
                    _ => panic!("Expected exponentiation in right operand"),
                }
            }
            _ => panic!("Expected exponentiation at top level"),
        }
    }

    #[test]
    fn test_parse_method_call_chain() {
        let allocator = Bump::new();
        let source = "obj.method().another();";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Call { .. } => {}
            _ => panic!("Expected function call"),
        }
    }

    #[test]
    fn test_parse_empty_block() {
        let allocator = Bump::new();
        let source = "{}";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse_block();
        assert!(result.is_ok());

        let block = result.unwrap();
        assert_eq!(block.expressions.len(), 0);
    }

    #[test]
    fn test_parse_anonymous_program() {
        let allocator = Bump::new();
        let source = "program () { 42; }";
        let lexer = Lexer::new(source);
        let parser = Parser::new(lexer, &allocator);

        let result = parser.parse();
        assert!(result.is_ok());

        let expressions = result.unwrap();
        assert_eq!(expressions.len(), 1);

        match &expressions[0] {
            Expression::Program { name, .. } => {
                assert_eq!(name.as_deref(), None);
            }
            _ => panic!("Expected program"),
        }
    }
}
