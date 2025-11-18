use crate::compiler::bytecode::{Chunk, Instruction, Value};
use std::collections::HashMap;

pub struct VirtualMachine {
    stack: Vec<Value>,
    globals: HashMap<String, Value>,
    ip: usize, // instruction pointer
}

impl VirtualMachine {
    pub fn new() -> Self {
        VirtualMachine {
            stack: Vec::new(),
            globals: HashMap::new(),
            ip: 0,
        }
    }

    pub fn execute(&mut self, chunk: &Chunk) -> Result<(), String> {
        self.ip = 0;

        while self.ip < chunk.instructions.len() {
            let instruction = &chunk.instructions[self.ip];
            self.ip += 1;

            match instruction {
                Instruction::Constant(value) => {
                    self.stack.push(value.clone());
                }

                Instruction::LoadGlobal(name) => {
                    let value = self
                        .globals
                        .get(name)
                        .cloned()
                        .ok_or_else(|| format!("Undefined variable: {}", name))?;
                    self.stack.push(value);
                }

                Instruction::StoreGlobal(name) => {
                    let value = self
                        .stack
                        .last()
                        .cloned()
                        .ok_or_else(|| "Stack underflow".to_string())?;
                    self.globals.insert(name.clone(), value);
                }

                Instruction::Add => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.add(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Subtract => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.subtract(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Multiply => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.multiply(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Divide => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.divide(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Modulo => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.modulo(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Exponentiate => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.exponentiate(left, right)?;
                    self.stack.push(result);
                }

                Instruction::Equal => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    self.stack.push(Value::Boolean(left == right));
                }

                Instruction::NotEqual => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    self.stack.push(Value::Boolean(left != right));
                }

                Instruction::LessThan => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.less_than(left, right)?;
                    self.stack.push(result);
                }

                Instruction::LessThanOrEqual => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.less_than_or_equal(left, right)?;
                    self.stack.push(result);
                }

                Instruction::GreaterThan => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.greater_than(left, right)?;
                    self.stack.push(result);
                }

                Instruction::GreaterThanOrEqual => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    let result = self.greater_than_or_equal(left, right)?;
                    self.stack.push(result);
                }

                Instruction::And => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    self.stack
                        .push(Value::Boolean(left.is_truthy() && right.is_truthy()));
                }

                Instruction::Or => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    self.stack
                        .push(Value::Boolean(left.is_truthy() || right.is_truthy()));
                }

                Instruction::Concatenate => {
                    let right = self.pop()?;
                    let left = self.pop()?;
                    self.stack
                        .push(Value::String(format!("{}{}", left, right)));
                }

                Instruction::Print => {
                    let count = self.pop()?;
                    if let Value::Integer(n) = count {
                        let mut args = Vec::new();
                        for _ in 0..n {
                            args.push(self.pop()?);
                        }
                        args.reverse();
                        for (i, arg) in args.iter().enumerate() {
                            print!("{}", arg);
                            if i < args.len() - 1 {
                                print!(" ");
                            }
                        }
                        println!();
                        self.stack.push(Value::Null);
                    } else {
                        return Err("Print requires integer argument count".to_string());
                    }
                }

                Instruction::PrintFormat => {
                    let count = self.pop()?;
                    if let Value::Integer(n) = count {
                        let mut args = Vec::new();
                        for _ in 0..n {
                            args.push(self.pop()?);
                        }
                        args.reverse();
                        for arg in args {
                            print!("{}", arg);
                        }
                        self.stack.push(Value::Null);
                    } else {
                        return Err("PrintFormat requires integer argument count".to_string());
                    }
                }

                Instruction::Pop => {
                    self.pop()?;
                }

                Instruction::Halt => {
                    break;
                }
            }
        }

        Ok(())
    }

    fn pop(&mut self) -> Result<Value, String> {
        self.stack.pop().ok_or_else(|| "Stack underflow".to_string())
    }

    fn add(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l + r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l + r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 + r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l + r as f64)),
            (Value::String(l), Value::String(r)) => Ok(Value::String(format!("{}{}", l, r))),
            _ => Err("Invalid operands for addition".to_string()),
        }
    }

    fn subtract(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l - r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l - r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 - r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l - r as f64)),
            _ => Err("Invalid operands for subtraction".to_string()),
        }
    }

    fn multiply(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Integer(l * r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Float(l * r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Float(l as f64 * r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Float(l * r as f64)),
            _ => Err("Invalid operands for multiplication".to_string()),
        }
    }

    fn divide(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
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
        }
    }

    fn modulo(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => {
                if r == 0 {
                    Err("Modulo by zero".to_string())
                } else {
                    Ok(Value::Integer(l % r))
                }
            }
            _ => Err("Invalid operands for modulo".to_string()),
        }
    }

    fn exponentiate(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
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
        }
    }

    fn less_than(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l < r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l < r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) < r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l < (r as f64))),
            _ => Err("Invalid operands for less than".to_string()),
        }
    }

    fn less_than_or_equal(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l <= r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l <= r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) <= r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l <= (r as f64))),
            _ => Err("Invalid operands for less than or equal".to_string()),
        }
    }

    fn greater_than(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l > r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l > r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) > r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l > (r as f64))),
            _ => Err("Invalid operands for greater than".to_string()),
        }
    }

    fn greater_than_or_equal(&self, left: Value, right: Value) -> Result<Value, String> {
        match (left, right) {
            (Value::Integer(l), Value::Integer(r)) => Ok(Value::Boolean(l >= r)),
            (Value::Float(l), Value::Float(r)) => Ok(Value::Boolean(l >= r)),
            (Value::Integer(l), Value::Float(r)) => Ok(Value::Boolean((l as f64) >= r)),
            (Value::Float(l), Value::Integer(r)) => Ok(Value::Boolean(l >= (r as f64))),
            _ => Err("Invalid operands for greater than or equal".to_string()),
        }
    }
}

impl Default for VirtualMachine {
    fn default() -> Self {
        Self::new()
    }
}
