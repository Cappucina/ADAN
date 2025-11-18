use crate::parser::ast::*;
use std::collections::HashMap;

#[derive(Debug, Clone, Copy, PartialEq)]
enum ValueType {
    Integer,
    Float,
    String,
    Boolean,
    Null,
}

pub struct X86_64CodeGen {
    output: Vec<String>,
    label_counter: usize,
    string_literals: HashMap<String, String>,
    float_literals: HashMap<String, String>,
}

impl X86_64CodeGen {
    pub fn new() -> Self {
        X86_64CodeGen {
            output: Vec::new(),
            label_counter: 0,
            string_literals: HashMap::new(),
            float_literals: HashMap::new(),
        }
    }

    pub fn compile(&mut self, expressions: &[Expression]) -> Result<String, String> {
        self.emit_header();
        self.emit_data_section();
        self.emit_text_section();
        self.emit_start();

        // Compile all expressions
        for expr in expressions {
            self.compile_expression(expr)?;
        }

        self.emit_exit();
        self.emit_string_literals();
        self.emit_float_literals();

        Ok(self.output.join("\n"))
    }

    fn emit_header(&mut self) {
        self.output.push("; ADAN compiled to x86-64 assembly (Linux System V ABI)".to_string());
        self.output.push("".to_string());
    }

    fn emit_data_section(&mut self) {
        self.output.push("section .data".to_string());
        self.output.push("    fmt_int: db \"%ld\", 0".to_string());
        self.output.push("    fmt_float: db \"%.15g\", 0".to_string());
        self.output.push("    fmt_str: db \"%s\", 0".to_string());
        self.output.push("    fmt_bool_true: db \"true\", 0".to_string());
        self.output.push("    fmt_bool_false: db \"false\", 0".to_string());
        self.output.push("    fmt_null: db \"null\", 0".to_string());
        self.output.push("    fmt_newline: db 10, 0".to_string());
        self.output.push("    fmt_space: db \" \", 0".to_string());
        self.output.push("".to_string());
    }

    fn emit_text_section(&mut self) {
        self.output.push("section .bss".to_string());
        self.output.push("LVAR_STORAGE: resq 1000    ; Storage for variables".to_string());
        self.output.push("".to_string());
        self.output.push("section .text".to_string());
        self.output.push("    global _main".to_string());
        self.output.push("    extern _printf".to_string());
        self.output.push("    extern _exit".to_string());
        self.output.push("    extern _strlen".to_string());
        self.output.push("    extern _malloc".to_string());
        self.output.push("    extern _strcpy".to_string());
        self.output.push("    extern _strcat".to_string());
        self.output.push("".to_string());
    }

    fn emit_start(&mut self) {
        self.output.push("_main:".to_string());
        self.output.push("    push rbp".to_string());
        self.output.push("    mov rbp, rsp".to_string());
        self.output.push("".to_string());
    }

    fn emit_exit(&mut self) {
        self.output.push("".to_string());
        self.output.push("    ; Exit program".to_string());
        self.output.push("    mov rsp, rbp".to_string());
        self.output.push("    pop rbp".to_string());
        self.output.push("    xor rdi, rdi        ; exit code 0".to_string());
        self.output.push("    call _exit".to_string());
        self.output.push("".to_string());
    }

    fn emit_string_literals(&mut self) {
        if !self.string_literals.is_empty() {
            self.output.push("section .data".to_string());
            for (label, value) in &self.string_literals {
                self.output.push(format!("    {}: db {}, 0", label, value));
            }
            self.output.push("".to_string());
        }
    }

    fn emit_float_literals(&mut self) {
        if !self.float_literals.is_empty() {
            self.output.push("section .data".to_string());
            for (label, value) in &self.float_literals {
                self.output.push(format!("    {}: dq {}", label, value));
            }
            self.output.push("".to_string());
        }
    }

    fn new_label(&mut self) -> String {
        let label = format!(".L{}", self.label_counter);
        self.label_counter += 1;
        label
    }

    fn compile_expression(&mut self, expr: &Expression) -> Result<ValueType, String> {
        match expr {
            Expression::IntegerLiteral { value } => {
                self.output.push(format!("    mov rax, {}", value));
                Ok(ValueType::Integer)
            }

            Expression::FloatLiteral { value } => {
                let label = self.new_label();
                self.float_literals.insert(label.clone(), value.to_string());
                self.output.push(format!("    movsd xmm0, [rel {}]", label));
                Ok(ValueType::Float)
            }

            Expression::StringLiteral { value } => {
                let label = self.new_label();
                let escaped = self.escape_string(value);
                self.string_literals.insert(label.clone(), escaped);
                self.output.push(format!("    lea rax, [rel {}]", label));
                Ok(ValueType::String)
            }

            Expression::BooleanLiteral { value } => {
                self.output.push(format!("    mov rax, {}", if *value { 1 } else { 0 }));
                Ok(ValueType::Boolean)
            }

            Expression::NullLiteral => {
                self.output.push("    xor rax, rax".to_string());
                Ok(ValueType::Null)
            }

            Expression::BinaryOperation { operator, left, right } => {
                if matches!(operator, BinaryOperator::Assign) {
                    return self.compile_assignment(left, right);
                }

                // Evaluate left operand
                let _left_type = self.compile_expression(left)?;
                self.output.push("    push rax        ; Save left operand".to_string());

                // Evaluate right operand
                let _right_type = self.compile_expression(right)?;
                self.output.push("    mov rbx, rax    ; Right operand in rbx".to_string());
                self.output.push("    pop rax         ; Left operand in rax".to_string());

                let result_type = match operator {
                    BinaryOperator::Add => {
                        self.output.push("    add rax, rbx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Subtract => {
                        self.output.push("    sub rax, rbx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Multiply => {
                        self.output.push("    imul rax, rbx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Divide => {
                        self.output.push("    xor rdx, rdx".to_string());
                        self.output.push("    idiv rbx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Modulo => {
                        self.output.push("    xor rdx, rdx".to_string());
                        self.output.push("    idiv rbx".to_string());
                        self.output.push("    mov rax, rdx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Exponentiate => {
                        // Use a simple integer power implementation
                        self.output.push("    push rcx".to_string());
                        self.output.push("    push rdx".to_string());
                        self.output.push("    mov rcx, rbx        ; exponent in rcx".to_string());
                        self.output.push("    mov rbx, rax        ; base in rbx".to_string());
                        self.output.push("    mov rax, 1          ; result = 1".to_string());
                        self.output.push("    test rcx, rcx".to_string());
                        let loop_label = self.new_label();
                        let done_label = self.new_label();
                        self.output.push(format!("    jle {}", done_label));
                        self.output.push(format!("{}:", loop_label));
                        self.output.push("    imul rax, rbx       ; result *= base".to_string());
                        self.output.push("    dec rcx".to_string());
                        self.output.push(format!("    jnz {}", loop_label));
                        self.output.push(format!("{}:", done_label));
                        self.output.push("    pop rdx".to_string());
                        self.output.push("    pop rcx".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::And => {
                        // Logical AND: convert to boolean then AND
                        self.output.push("    test rax, rax".to_string());
                        self.output.push("    setnz al".to_string());
                        self.output.push("    test rbx, rbx".to_string());
                        self.output.push("    setnz bl".to_string());
                        self.output.push("    and al, bl".to_string());
                        self.output.push("    movzx rax, al".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::Or => {
                        // Logical OR: convert to boolean then OR
                        self.output.push("    test rax, rax".to_string());
                        self.output.push("    setnz al".to_string());
                        self.output.push("    test rbx, rbx".to_string());
                        self.output.push("    setnz bl".to_string());
                        self.output.push("    or al, bl".to_string());
                        self.output.push("    movzx rax, al".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::Concatenate => {
                        // String concatenation: use extern strcat/malloc
                        // Left string pointer is in rax, right string will be in rbx
                        self.output.push("    push rax        ; Save left string".to_string());

                        // Get lengths of both strings
                        self.output.push("    mov rdi, rax    ; Left string for _strlen".to_string());
                        self.output.push("    call _strlen".to_string());
                        self.output.push("    mov r12, rax    ; Save left length in r12".to_string());

                        self.output.push("    pop rax         ; Restore left string".to_string());
                        self.output.push("    push rax        ; Save left string again".to_string());
                        self.output.push("    mov rdi, rbx    ; Right string for _strlen".to_string());
                        self.output.push("    call _strlen".to_string());
                        self.output.push("    mov r13, rax    ; Save right length in r13".to_string());

                        // Allocate memory for result (left_len + right_len + 1)
                        self.output.push("    mov rdi, r12    ; Left length".to_string());
                        self.output.push("    add rdi, r13    ; + right length".to_string());
                        self.output.push("    add rdi, 1      ; + null terminator".to_string());
                        self.output.push("    call _malloc".to_string());
                        self.output.push("    mov r14, rax    ; Save result buffer in r14".to_string());

                        // Copy left string to result
                        self.output.push("    mov rdi, r14    ; Destination".to_string());
                        self.output.push("    pop rsi         ; Source (left string)".to_string());
                        self.output.push("    push rsi        ; Save left string again".to_string());
                        self.output.push("    call _strcpy".to_string());

                        // Concatenate right string to result
                        self.output.push("    mov rdi, r14    ; Destination".to_string());
                        self.output.push("    mov rsi, rbx    ; Source (right string)".to_string());
                        self.output.push("    call _strcat".to_string());

                        self.output.push("    pop rax         ; Clean up stack".to_string());
                        self.output.push("    mov rax, r14    ; Result pointer in rax".to_string());

                        ValueType::String
                    }
                    BinaryOperator::Equal | BinaryOperator::NotEqual |
                    BinaryOperator::LessThan | BinaryOperator::GreaterThan |
                    BinaryOperator::LessThanOrEqual | BinaryOperator::GreaterThanOrEqual => {
                        self.output.push("    cmp rax, rbx".to_string());
                        let instr = match operator {
                            BinaryOperator::Equal => "sete",
                            BinaryOperator::NotEqual => "setne",
                            BinaryOperator::LessThan => "setl",
                            BinaryOperator::GreaterThan => "setg",
                            BinaryOperator::LessThanOrEqual => "setle",
                            BinaryOperator::GreaterThanOrEqual => "setge",
                            _ => unreachable!(),
                        };
                        self.output.push(format!("    {} al", instr));
                        self.output.push("    movzx rax, al".to_string());
                        ValueType::Boolean
                    }
                    _ => return Err(format!("Unsupported binary operator: {:?}", operator)),
                };
                Ok(result_type)
            }

            Expression::Identifier { value } => {
                let hash = self.hash_variable_name(value);
                self.output.push(format!("    mov rax, [rel LVAR_STORAGE + {}]", hash * 8));
                Ok(ValueType::Integer) // Assume integer for now
            }

            Expression::Call { function, arguments } => {
                self.compile_call(function, arguments)
            }

            Expression::Program { name: _, parameters: _, body } => {
                for expr in &body.expressions {
                    self.compile_expression(expr)?;
                }
                Ok(ValueType::Null)
            }

            Expression::Block { block } => {
                let mut last_type = ValueType::Null;
                for expr in &block.expressions {
                    last_type = self.compile_expression(expr)?;
                }
                Ok(last_type)
            }

            _ => Err(format!("Unsupported expression: {:?}", expr)),
        }
    }

    fn compile_assignment(&mut self, left: &Expression, right: &Expression) -> Result<ValueType, String> {
        if let Expression::Identifier { value: name } = left {
            // Compile the right side
            let value_type = self.compile_expression(right)?;

            // Store the result
            let hash = self.hash_variable_name(name);
            self.output.push(format!("    mov [rel LVAR_STORAGE + {}], rax", hash * 8));
            Ok(value_type)
        } else {
            Err("Left side of assignment must be an identifier".to_string())
        }
    }

    fn compile_call(&mut self, function: &Expression, arguments: &[Expression]) -> Result<ValueType, String> {
        // Handle io.print and io.printf
        if let Expression::IndexName { operator: _, expression, name } = function {
            if let Expression::Identifier { value: module } = &**expression {
                if *module == "io" {
                    return self.compile_io_function(name, arguments);
                }
            }
        }
        Err(format!("Unknown function: {:?}", function))
    }

    fn compile_io_function(&mut self, function_name: &str, arguments: &[Expression]) -> Result<ValueType, String> {
        match function_name {
            "print" => {
                // Print each argument
                for (i, arg) in arguments.iter().enumerate() {
                    let arg_type = self.compile_expression(arg)?;

                    // Select format based on type
                    let fmt = match arg_type {
                        ValueType::Integer => "fmt_int",
                        ValueType::Float => "fmt_float",
                        ValueType::String => "fmt_str",
                        ValueType::Boolean => {
                            let false_label = self.new_label();
                            let print_label = self.new_label();
                            self.output.push("    cmp rax, 0".to_string());
                            self.output.push(format!("    je {}", false_label));
                            self.output.push("    lea rdi, [rel fmt_bool_true]".to_string());
                            self.output.push(format!("    jmp {}", print_label));
                            self.output.push(format!("{}:", false_label));
                            self.output.push("    lea rdi, [rel fmt_bool_false]".to_string());
                            self.output.push(format!("{}:", print_label));
                            self.output.push("    xor rax, rax".to_string());
                            self.output.push("    call _printf".to_string());

                            if i < arguments.len() - 1 {
                                self.output.push("    lea rdi, [rel fmt_space]".to_string());
                                self.output.push("    xor rax, rax".to_string());
                                self.output.push("    call _printf".to_string());
                            }
                            continue;
                        }
                        ValueType::Null => "fmt_null",
                    };

                    // Print the value
                    self.output.push("    push rax".to_string());
                    self.output.push(format!("    lea rdi, [rel {}]", fmt));
                    self.output.push("    mov rsi, rax".to_string());
                    self.output.push("    xor rax, rax".to_string());
                    self.output.push("    call _printf".to_string());
                    self.output.push("    pop rax".to_string());

                    if i < arguments.len() - 1 {
                        self.output.push("    lea rdi, [rel fmt_space]".to_string());
                        self.output.push("    xor rax, rax".to_string());
                        self.output.push("    call _printf".to_string());
                    }
                }

                // Print newline
                self.output.push("    lea rdi, [rel fmt_newline]".to_string());
                self.output.push("    xor rax, rax".to_string());
                self.output.push("    call _printf".to_string());
                Ok(ValueType::Null)
            }
            "printf" => {
                // Print without newline
                for arg in arguments {
                    let arg_type = self.compile_expression(arg)?;

                    let fmt = match arg_type {
                        ValueType::Integer => "fmt_int",
                        ValueType::Float => "fmt_float",
                        ValueType::String => "fmt_str",
                        ValueType::Boolean => {
                            let false_label = self.new_label();
                            let print_label = self.new_label();
                            self.output.push("    cmp rax, 0".to_string());
                            self.output.push(format!("    je {}", false_label));
                            self.output.push("    lea rdi, [rel fmt_bool_true]".to_string());
                            self.output.push(format!("    jmp {}", print_label));
                            self.output.push(format!("{}:", false_label));
                            self.output.push("    lea rdi, [rel fmt_bool_false]".to_string());
                            self.output.push(format!("{}:", print_label));
                            self.output.push("    xor rax, rax".to_string());
                            self.output.push("    call _printf".to_string());
                            continue;
                        }
                        ValueType::Null => "fmt_null",
                    };

                    self.output.push("    push rax".to_string());
                    self.output.push(format!("    lea rdi, [rel {}]", fmt));
                    self.output.push("    mov rsi, rax".to_string());
                    self.output.push("    xor rax, rax".to_string());
                    self.output.push("    call _printf".to_string());
                    self.output.push("    pop rax".to_string());
                }
                Ok(ValueType::Null)
            }
            _ => Err(format!("Unknown io function: {}", function_name)),
        }
    }

    fn hash_variable_name(&self, name: &str) -> usize {
        // Simple hash function for variable storage
        let mut hash: usize = 0;
        for byte in name.bytes() {
            hash = hash.wrapping_mul(31).wrapping_add(byte as usize);
        }
        hash % 1000 // Stay within our storage array
    }

    fn escape_string(&self, s: &str) -> String {
        let mut result = String::from("\"");
        for ch in s.chars() {
            match ch {
                '\n' => result.push_str("\", 10, \""),
                '\t' => result.push_str("\", 9, \""),
                '"' => result.push_str("\\\""),
                _ => result.push(ch),
            }
        }
        result.push('"');
        result
    }
}

impl Default for X86_64CodeGen {
    fn default() -> Self {
        Self::new()
    }
}
