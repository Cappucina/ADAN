use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    Null,
    Boolean(bool),
    Integer(i64),
    Float(f64),
    String(String),
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Null => write!(f, "null"),
            Value::Boolean(b) => write!(f, "{}", b),
            Value::Integer(i) => write!(f, "{}", i),
            Value::Float(fl) => write!(f, "{}", fl),
            Value::String(s) => write!(f, "{}", s),
        }
    }
}

impl Value {
    pub fn is_truthy(&self) -> bool {
        match self {
            Value::Null => false,
            Value::Boolean(b) => *b,
            Value::Integer(i) => *i != 0,
            Value::Float(f) => *f != 0.0,
            Value::String(s) => !s.is_empty(),
        }
    }
}

#[derive(Debug, Clone)]
pub enum Instruction {
    // Constants and literals
    Constant(Value),
    LoadGlobal(String),
    StoreGlobal(String),

    // Arithmetic operations
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Exponentiate,

    // Comparison operations
    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,

    // Logical operations
    And,
    Or,

    // String operations
    Concatenate,

    // I/O operations
    Print,       // Print with newline
    PrintFormat, // Print without newline (printf)

    // Stack operations
    Pop,

    // Control flow
    Halt,
}

#[derive(Debug, Clone)]
pub struct Chunk {
    pub instructions: Vec<Instruction>,
}

impl Chunk {
    pub fn new() -> Self {
        Chunk {
            instructions: Vec::new(),
        }
    }

    pub fn write(&mut self, instruction: Instruction) {
        self.instructions.push(instruction);
    }

    pub fn disassemble(&self, name: &str) {
        println!("== {} ==", name);
        for (offset, instruction) in self.instructions.iter().enumerate() {
            println!("{:04} {:?}", offset, instruction);
        }
    }
}

impl Default for Chunk {
    fn default() -> Self {
        Self::new()
    }
}
