use crate::compiler::bytecode::{Chunk, Instruction, Value};
use crate::parser::ast::*;

pub struct Compiler {
    chunk: Chunk,
}

impl Compiler {
    pub fn new() -> Self {
        Compiler {
            chunk: Chunk::new(),
        }
    }

    pub fn compile(&mut self, expressions: &[Expression]) -> Result<Chunk, String> {
        for expr in expressions {
            self.compile_expression(expr)?;
            // Pop the result if it's not needed (statement-level expression)
            self.chunk.write(Instruction::Pop);
        }
        self.chunk.write(Instruction::Halt);
        Ok(self.chunk.clone())
    }

    fn compile_expression(&mut self, expr: &Expression) -> Result<(), String> {
        match expr {
            Expression::NullLiteral => {
                self.chunk.write(Instruction::Constant(Value::Null));
                Ok(())
            }

            Expression::BooleanLiteral { value } => {
                self.chunk
                    .write(Instruction::Constant(Value::Boolean(*value)));
                Ok(())
            }

            Expression::IntegerLiteral { value } => {
                self.chunk
                    .write(Instruction::Constant(Value::Integer(*value)));
                Ok(())
            }

            Expression::FloatLiteral { value } => {
                self.chunk.write(Instruction::Constant(Value::Float(*value)));
                Ok(())
            }

            Expression::StringLiteral { value } => {
                self.chunk
                    .write(Instruction::Constant(Value::String(value.to_string())));
                Ok(())
            }

            Expression::Identifier { value } => {
                self.chunk
                    .write(Instruction::LoadGlobal(value.to_string()));
                Ok(())
            }

            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => {
                // Special handling for assignment
                if matches!(operator, BinaryOperator::Assign) {
                    if let Expression::Identifier { value: name } = &**left {
                        // Compile the right side
                        self.compile_expression(right)?;
                        // Store to variable (keeps value on stack)
                        self.chunk
                            .write(Instruction::StoreGlobal(name.to_string()));
                        return Ok(());
                    } else {
                        return Err("Left side of assignment must be an identifier".to_string());
                    }
                }

                // Compile operands
                self.compile_expression(left)?;
                self.compile_expression(right)?;

                // Compile operator
                match operator {
                    BinaryOperator::Add => self.chunk.write(Instruction::Add),
                    BinaryOperator::Subtract => self.chunk.write(Instruction::Subtract),
                    BinaryOperator::Multiply => self.chunk.write(Instruction::Multiply),
                    BinaryOperator::Divide => self.chunk.write(Instruction::Divide),
                    BinaryOperator::Modulo => self.chunk.write(Instruction::Modulo),
                    BinaryOperator::Exponentiate => self.chunk.write(Instruction::Exponentiate),
                    BinaryOperator::Equal => self.chunk.write(Instruction::Equal),
                    BinaryOperator::NotEqual => self.chunk.write(Instruction::NotEqual),
                    BinaryOperator::LessThan => self.chunk.write(Instruction::LessThan),
                    BinaryOperator::LessThanOrEqual => {
                        self.chunk.write(Instruction::LessThanOrEqual)
                    }
                    BinaryOperator::GreaterThan => self.chunk.write(Instruction::GreaterThan),
                    BinaryOperator::GreaterThanOrEqual => {
                        self.chunk.write(Instruction::GreaterThanOrEqual)
                    }
                    BinaryOperator::And => self.chunk.write(Instruction::And),
                    BinaryOperator::Or => self.chunk.write(Instruction::Or),
                    BinaryOperator::Concatenate => self.chunk.write(Instruction::Concatenate),
                    BinaryOperator::Assign => {
                        unreachable!("Assignment handled above")
                    }
                }
                Ok(())
            }

            Expression::Call {
                function,
                arguments,
            } => {
                // Handle built-in functions
                if let Expression::IndexName {
                    operator: _,
                    expression,
                    name,
                } = &**function
                {
                    if let Expression::Identifier { value: module } = &**expression {
                        if *module == "io" {
                            return self.compile_io_function(name, arguments);
                        }
                    }
                }
                Err(format!("Unknown function: {:?}", function))
            }

            Expression::Block { block } => {
                for expr in &block.expressions {
                    self.compile_expression(expr)?;
                    self.chunk.write(Instruction::Pop);
                }
                // Push null for empty blocks
                self.chunk.write(Instruction::Constant(Value::Null));
                Ok(())
            }

            Expression::Program {
                name: _,
                parameters: _,
                body,
            } => {
                for expr in &body.expressions {
                    self.compile_expression(expr)?;
                    self.chunk.write(Instruction::Pop);
                }
                // Push null as program result
                self.chunk.write(Instruction::Constant(Value::Null));
                Ok(())
            }

            _ => Err(format!("Unimplemented expression: {:?}", expr)),
        }
    }

    fn compile_io_function(
        &mut self,
        function_name: &str,
        arguments: &[Expression],
    ) -> Result<(), String> {
        match function_name {
            "print" => {
                // Compile all arguments
                for arg in arguments {
                    self.compile_expression(arg)?;
                }
                // Push argument count as a constant
                self.chunk
                    .write(Instruction::Constant(Value::Integer(arguments.len() as i64)));
                self.chunk.write(Instruction::Print);
                Ok(())
            }

            "printf" => {
                // Compile all arguments
                for arg in arguments {
                    self.compile_expression(arg)?;
                }
                // Push argument count as a constant
                self.chunk
                    .write(Instruction::Constant(Value::Integer(arguments.len() as i64)));
                self.chunk.write(Instruction::PrintFormat);
                Ok(())
            }

            _ => Err(format!("Unknown io function: {}", function_name)),
        }
    }
}

impl Default for Compiler {
    fn default() -> Self {
        Self::new()
    }
}
