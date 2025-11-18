use crate::interpreter::value::Value;
use crate::parser::ast::*;
use std::collections::HashMap;

pub struct Runtime {
    globals: HashMap<String, Value>,
}

impl Runtime {
    pub fn new() -> Self {
        Runtime {
            globals: HashMap::new(),
        }
    }

    pub fn execute(&mut self, expr: &Expression) -> Result<Value, String> {
        match expr {
            Expression::NullLiteral => Ok(Value::Null),

            Expression::BooleanLiteral { value } => Ok(Value::Boolean(*value)),

            Expression::IntegerLiteral { value } => Ok(Value::Integer(*value)),

            Expression::FloatLiteral { value } => Ok(Value::Float(*value)),

            Expression::StringLiteral { value } => Ok(Value::String(value.to_string())),

            Expression::Identifier { value } => {
                self.globals
                    .get(*value)
                    .cloned()
                    .ok_or_else(|| format!("Undefined variable: {}", value))
            }

            Expression::BinaryOperation {
                operator,
                left,
                right,
            } => self.execute_binary_operation(operator, left, right),

            Expression::Call {
                function,
                arguments,
            } => self.execute_call(function, arguments),

            Expression::IndexName {
                operator,
                expression,
                name,
            } => self.execute_index_name(operator, expression, name),

            Expression::Block { block } => self.execute_block(block),

            Expression::Program {
                name: _,
                parameters: _,
                body,
            } => self.execute_block(body),

            _ => Err(format!("Unimplemented expression: {:?}", expr)),
        }
    }

    fn execute_block(&mut self, block: &Block) -> Result<Value, String> {
        let mut last_value = Value::Null;
        for expr in &block.expressions {
            last_value = self.execute(expr)?;
        }
        Ok(last_value)
    }

    fn execute_binary_operation(
        &mut self,
        operator: &BinaryOperator,
        left: &Expression,
        right: &Expression,
    ) -> Result<Value, String> {
        // Handle assignment specially - don't evaluate left side
        if matches!(operator, BinaryOperator::Assign) {
            if let Expression::Identifier { value: name } = left {
                let right_val = self.execute(right)?;
                self.globals.insert(name.to_string(), right_val.clone());
                return Ok(right_val);
            } else {
                return Err("Left side of assignment must be an identifier".to_string());
            }
        }

        let left_val = self.execute(left)?;
        let right_val = self.execute(right)?;

        match operator {
            BinaryOperator::Add => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l + r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l + r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 + r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l + r as f64)),
                (Value::String(l), Value::String(r)) => Ok(Value::String(format!("{}{}", l, r))),
                _ => Err("Invalid operands for addition".to_string()),
            },

            BinaryOperator::Subtract => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l - r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l - r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 - r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l - r as f64)),
                _ => Err("Invalid operands for subtraction".to_string()),
            },

            BinaryOperator::Multiply => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l * r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l * r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 * r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l * r as f64)),
                _ => Err("Invalid operands for multiplication".to_string()),
            },

            BinaryOperator::Divide => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => {
                    if r == 0 {
                        Err("Division by zero".to_string())
                    } else {
                        Ok(Value::Integer(l / r))
                    }
                }
                (Value::Float(l), Value::Float(r)) => {
                    if r == 0.0 {
                        Err("Division by zero".to_string())
                    } else {
                        Ok(Value::Float(l / r))
                    }
                }
                (Value::Integer(l), Value::Float(r)) => {
                    if r == 0.0 {
                        Err("Division by zero".to_string())
                    } else {
                        Ok(Value::Float(l as f64 / r))
                    }
                }
                (Value::Float(l), Value::Integer(r)) => {
                    if r == 0 {
                        Err("Division by zero".to_string())
                    } else {
                        Ok(Value::Float(l / r as f64))
                    }
                }
                _ => Err("Invalid operands for division".to_string()),
            },

            BinaryOperator::Modulo => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => {
                    if r == 0 {
                        Err("Modulo by zero".to_string())
                    } else {
                        Ok(Value::Integer(l % r))
                    }
                }
                _ => Err("Invalid operands for modulo".to_string()),
            },

            BinaryOperator::Exponentiate => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => {
                    if r < 0 {
                        Ok(Value::Float((l as f64).powf(r as f64)))
                    } else {
                        Ok(Value::Integer(l.pow(r as u32)))
                    }
                }
                (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l.powf(r))),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Float((l as f64).powf(r))),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l.powf(r as f64))),
                _ => Err("Invalid operands for exponentiation".to_string()),
            },

            BinaryOperator::Equal => Ok(Value::Boolean(left_val == right_val)),

            BinaryOperator::NotEqual => Ok(Value::Boolean(left_val != right_val)),

            BinaryOperator::LessThan => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l < r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l < r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) < r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l < (r as f64))),
                _ => Err("Invalid operands for less than".to_string()),
            },

            BinaryOperator::LessThanOrEqual => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l <= r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l <= r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) <= r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l <= (r as f64))),
                _ => Err("Invalid operands for less than or equal".to_string()),
            },

            BinaryOperator::GreaterThan => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l > r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l > r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) > r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l > (r as f64))),
                _ => Err("Invalid operands for greater than".to_string()),
            },

            BinaryOperator::GreaterThanOrEqual => match (left_val, right_val) {
                (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l >= r)),
                (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l >= r)),
                (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) >= r)),
                (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l >= (r as f64))),
                _ => Err("Invalid operands for greater than or equal".to_string()),
            },

            BinaryOperator::And => {
                Ok(Value::Boolean(left_val.is_truthy() && right_val.is_truthy()))
            }

            BinaryOperator::Or => {
                Ok(Value::Boolean(left_val.is_truthy() || right_val.is_truthy()))
            }

            BinaryOperator::Concatenate => {
                Ok(Value::String(format!("{}{}", left_val, right_val)))
            }

            BinaryOperator::Assign => {
                unreachable!("Assignment is handled earlier in the function")
            }
        }
    }

    fn execute_call(
        &mut self,
        function: &Expression,
        arguments: &[Expression],
    ) -> Result<Value, String> {
        // Handle built-in functions
        if let Expression::IndexName {
            operator: _,
            expression,
            name,
        } = function
        {
            if let Expression::Identifier { value: module } = &**expression {
                if *module == "io" {
                    return self.execute_io_function(name, arguments);
                }
            }
        }

        Err(format!("Unknown function: {:?}", function))
    }

    fn execute_io_function(
        &mut self,
        function_name: &str,
        arguments: &[Expression],
    ) -> Result<Value, String> {
        match function_name {
            "print" => {
                for (i, arg) in arguments.iter().enumerate() {
                    let value = self.execute(arg)?;
                    print!("{}", value);
                    if i < arguments.len() - 1 {
                        print!(" ");
                    }
                }
                println!();
                Ok(Value::Null)
            }

            "printf" => {
                for arg in arguments {
                    let value = self.execute(arg)?;
                    print!("{}", value);
                }
                Ok(Value::Null)
            }

            _ => Err(format!("Unknown io function: {}", function_name)),
        }
    }

    fn execute_index_name(
        &mut self,
        _operator: &IndexOperator,
        expression: &Expression,
        _name: &str,
    ) -> Result<Value, String> {
        // For now, just evaluate the expression
        // In a full implementation, this would handle member access
        self.execute(expression)
    }
}

impl Default for Runtime {
    fn default() -> Self {
        Self::new()
    }
}
