# ADAN Examples

This directory contains example programs written in ADAN.

## Running Examples

To run any example:
```bash
cargo run examples/<filename>.adn
```

Or use the Makefile:
```bash
make run-file FILE=examples/<filename>.adn
```

## Viewing Compiled Bytecode

To see the compiled bytecode (shows the compilation step):
```bash
cargo run -- --disassemble examples/<filename>.adn
# or
cargo run -- -d examples/<filename>.adn
```

## Available Examples

### Playground.adn
A simple "Hello, World!" program demonstrating basic program structure and output.

**Output:**
```
Hello, World!
```

### Math.adn
Demonstrates various mathematical and logical operations:
- Basic arithmetic (+, -, *, /, %)
- Exponentiation (^)
- Floating point operations
- Mixed integer/float operations
- Comparison operators (>, <, =, !=, <=, >=)
- Logical operators (&&, ||)
- String concatenation (..)

### Variables.adn
Shows variable assignment and usage:
- Variable assignment using the `->` operator
- Using variables in expressions
- Variable reassignment
- String concatenation with variables

### SimpleCalc.adn
A simple calculator demonstrating:
- Variable storage and retrieval
- Complex expressions with operator precedence
- Exponentiation
- Perfect for viewing bytecode compilation with `--disassemble`

## Language Syntax Highlights

### Program Structure
```adan
program main() {
    // Your code here
}
```

### Variable Assignment
```adan
x -> 42;
name -> "ADAN";
```

### Output
```adan
io.print("Hello");      // Print with newline
io.printf("Hello");     // Print without newline
```

### Operators
- Assignment: `x -> value`
- Arithmetic: `+`, `-`, `*`, `/`, `%`, `^`
- Comparison: `=`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`
- Concatenation: `..`
