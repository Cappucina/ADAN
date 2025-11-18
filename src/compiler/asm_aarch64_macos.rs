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

pub struct AArch64MacOSCodeGen {
    output: Vec<String>,
    label_counter: usize,
    string_literals: HashMap<String, String>,
    float_literals: HashMap<String, String>,
}

impl AArch64MacOSCodeGen {
    pub fn new() -> Self {
        AArch64MacOSCodeGen {
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
        self.output.push("; ADAN compiled to ARM64 assembly (macOS)".to_string());
        self.output.push("".to_string());
    }

    fn emit_data_section(&mut self) {
        self.output.push(".data".to_string());
        self.output.push(".align 3".to_string());
        self.output.push("fmt_int: .asciz \"%lld\"".to_string());
        self.output.push("fmt_float: .asciz \"%.15g\"".to_string());
        self.output.push("fmt_str: .asciz \"%s\"".to_string());
        self.output.push("fmt_bool_true: .asciz \"true\"".to_string());
        self.output.push("fmt_bool_false: .asciz \"false\"".to_string());
        self.output.push("fmt_null: .asciz \"null\"".to_string());
        self.output.push("fmt_newline: .asciz \"\\n\"".to_string());
        self.output.push("fmt_space: .asciz \" \"".to_string());
        self.output.push("".to_string());
    }

    fn emit_text_section(&mut self) {
        self.output.push(".bss".to_string());
        self.output.push(".align 3".to_string());
        self.output.push("var_storage: .space 8000    ; Storage for 1000 variables".to_string());
        self.output.push("".to_string());
        self.output.push(".text".to_string());
        self.output.push(".global _main".to_string());
        self.output.push(".align 2".to_string());
        self.output.push("".to_string());
    }

    fn emit_start(&mut self) {
        self.output.push("_main:".to_string());
        self.output.push("    stp x29, x30, [sp, #-16]!".to_string());
        self.output.push("    mov x29, sp".to_string());
        self.output.push("".to_string());
    }

    fn emit_exit(&mut self) {
        self.output.push("".to_string());
        self.output.push("    ; Exit program".to_string());
        self.output.push("    mov w0, #0              ; exit code 0".to_string());
        self.output.push("    ldp x29, x30, [sp], #16".to_string());
        self.output.push("    ret".to_string());
        self.output.push("".to_string());
    }

    fn emit_string_literals(&mut self) {
        if !self.string_literals.is_empty() {
            self.output.push(".data".to_string());
            self.output.push(".align 3".to_string());
            for (label, value) in &self.string_literals {
                self.output.push(format!("{}: .asciz {}", label, value));
            }
            self.output.push("".to_string());
        }
    }

    fn emit_float_literals(&mut self) {
        if !self.float_literals.is_empty() {
            self.output.push(".data".to_string());
            self.output.push(".align 3".to_string());
            for (label, value) in &self.float_literals {
                self.output.push(format!("{}: .double {}", label, value));
            }
            self.output.push("".to_string());
        }
    }

    fn new_label(&mut self) -> String {
        let label = format!("L{}", self.label_counter);
        self.label_counter += 1;
        label
    }

    fn compile_expression(&mut self, expr: &Expression) -> Result<ValueType, String> {
        match expr {
            Expression::IntegerLiteral { value } => {
                self.output.push(format!("    mov x0, #{}", value));
                Ok(ValueType::Integer)
            }

            Expression::FloatLiteral { value } => {
                let label = self.new_label();
                self.float_literals.insert(label.clone(), value.to_string());
                self.output.push(format!("    adrp x1, {}@PAGE", label));
                self.output.push(format!("    add x1, x1, {}@PAGEOFF", label));
                self.output.push("    ldr d0, [x1]".to_string());
                Ok(ValueType::Float)
            }

            Expression::StringLiteral { value } => {
                let label = self.new_label();
                let escaped = self.escape_string(value);
                self.string_literals.insert(label.clone(), escaped);
                self.output.push(format!("    adrp x0, {}@PAGE", label));
                self.output.push(format!("    add x0, x0, {}@PAGEOFF", label));
                Ok(ValueType::String)
            }

            Expression::BooleanLiteral { value } => {
                self.output.push(format!("    mov x0, #{}", if *value { 1 } else { 0 }));
                Ok(ValueType::Boolean)
            }

            Expression::NullLiteral => {
                self.output.push("    mov x0, #0".to_string());
                Ok(ValueType::Null)
            }

            Expression::BinaryOperation { operator, left, right } => {
                if matches!(operator, BinaryOperator::Assign) {
                    return self.compile_assignment(left, right);
                }

                // Evaluate left operand (result in x0)
                let _left_type = self.compile_expression(left)?;
                self.output.push("    str x0, [sp, #-16]!    ; Save left operand".to_string());

                // Evaluate right operand (result in x0)
                let _right_type = self.compile_expression(right)?;
                self.output.push("    mov x1, x0              ; Right in x1".to_string());
                self.output.push("    ldr x0, [sp], #16      ; Left in x0".to_string());

                let result_type = match operator {
                    BinaryOperator::Add => {
                        self.output.push("    add x0, x0, x1".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Subtract => {
                        self.output.push("    sub x0, x0, x1".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Multiply => {
                        self.output.push("    mul x0, x0, x1".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Divide => {
                        self.output.push("    sdiv x0, x0, x1".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Modulo => {
                        self.output.push("    sdiv x2, x0, x1".to_string());
                        self.output.push("    msub x0, x2, x1, x0".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::Exponentiate => {
                        // Integer power implementation for ARM64
                        self.output.push("    mov x2, x1              ; exponent in x2".to_string());
                        self.output.push("    mov x3, x0              ; base in x3".to_string());
                        self.output.push("    mov x0, #1              ; result = 1".to_string());
                        self.output.push("    cmp x2, #0".to_string());
                        self.output.push("    ble Lpow_done".to_string());
                        self.output.push("Lpow_loop:".to_string());
                        self.output.push("    mul x0, x0, x3          ; result *= base".to_string());
                        self.output.push("    sub x2, x2, #1".to_string());
                        self.output.push("    cmp x2, #0".to_string());
                        self.output.push("    bgt Lpow_loop".to_string());
                        self.output.push("Lpow_done:".to_string());
                        ValueType::Integer
                    }
                    BinaryOperator::And => {
                        // Logical AND for ARM64
                        self.output.push("    cmp x0, #0".to_string());
                        self.output.push("    cset x0, ne             ; x0 = (x0 != 0)".to_string());
                        self.output.push("    cmp x1, #0".to_string());
                        self.output.push("    cset x1, ne             ; x1 = (x1 != 0)".to_string());
                        self.output.push("    and x0, x0, x1          ; x0 = x0 && x1".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::Or => {
                        // Logical OR for ARM64
                        self.output.push("    cmp x0, #0".to_string());
                        self.output.push("    cset x0, ne             ; x0 = (x0 != 0)".to_string());
                        self.output.push("    cmp x1, #0".to_string());
                        self.output.push("    cset x1, ne             ; x1 = (x1 != 0)".to_string());
                        self.output.push("    orr x0, x0, x1          ; x0 = x0 || x1".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::Concatenate => {
                        return Err("String concatenation (..) not yet supported in native compilation. Use VM mode for this feature.".to_string());
                    }
                    BinaryOperator::Equal => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, eq".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::NotEqual => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, ne".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::LessThan => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, lt".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::GreaterThan => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, gt".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::LessThanOrEqual => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, le".to_string());
                        ValueType::Boolean
                    }
                    BinaryOperator::GreaterThanOrEqual => {
                        self.output.push("    cmp x0, x1".to_string());
                        self.output.push("    cset x0, ge".to_string());
                        ValueType::Boolean
                    }
                    _ => return Err(format!("Unsupported binary operator: {:?}", operator)),
                };
                Ok(result_type)
            }

            Expression::Identifier { value } => {
                let hash = self.hash_variable_name(value);
                self.output.push(format!("    adrp x1, var_storage@PAGE"));
                self.output.push(format!("    add x1, x1, var_storage@PAGEOFF"));
                self.output.push(format!("    ldr x0, [x1, #{}]", hash * 8));
                Ok(ValueType::Integer)
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
            let value_type = self.compile_expression(right)?;

            let hash = self.hash_variable_name(name);
            self.output.push("    adrp x1, var_storage@PAGE".to_string());
            self.output.push("    add x1, x1, var_storage@PAGEOFF".to_string());
            self.output.push(format!("    str x0, [x1, #{}]", hash * 8));
            Ok(value_type)
        } else {
            Err("Left side of assignment must be an identifier".to_string())
        }
    }

    fn compile_call(&mut self, function: &Expression, arguments: &[Expression]) -> Result<ValueType, String> {
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
                for (i, arg) in arguments.iter().enumerate() {
                    let arg_type = self.compile_expression(arg)?;

                    let fmt = match arg_type {
                        ValueType::Integer => "fmt_int",
                        ValueType::Float => "fmt_float",
                        ValueType::String => "fmt_str",
                        ValueType::Boolean => {
                            self.output.push("    cmp x0, #0".to_string());
                            self.output.push("    adrp x0, fmt_bool_false@PAGE".to_string());
                            self.output.push("    add x0, x0, fmt_bool_false@PAGEOFF".to_string());
                            self.output.push("    adrp x1, fmt_bool_true@PAGE".to_string());
                            self.output.push("    add x1, x1, fmt_bool_true@PAGEOFF".to_string());
                            self.output.push("    csel x0, x1, x0, ne".to_string());
                            self.output.push("    bl _printf".to_string());

                            if i < arguments.len() - 1 {
                                self.output.push("    adrp x0, fmt_space@PAGE".to_string());
                                self.output.push("    add x0, x0, fmt_space@PAGEOFF".to_string());
                                self.output.push("    bl _printf".to_string());
                            }
                            continue;
                        }
                        ValueType::Null => "fmt_null",
                    };

                    // Print value
                    self.output.push("    mov x1, x0              ; Value to print".to_string());
                    self.output.push(format!("    adrp x0, {}@PAGE", fmt));
                    self.output.push(format!("    add x0, x0, {}@PAGEOFF", fmt));
                    self.output.push("    bl _printf".to_string());

                    if i < arguments.len() - 1 {
                        self.output.push("    adrp x0, fmt_space@PAGE".to_string());
                        self.output.push("    add x0, x0, fmt_space@PAGEOFF".to_string());
                        self.output.push("    bl _printf".to_string());
                    }
                }

                // Print newline
                self.output.push("    adrp x0, fmt_newline@PAGE".to_string());
                self.output.push("    add x0, x0, fmt_newline@PAGEOFF".to_string());
                self.output.push("    bl _printf".to_string());
                Ok(ValueType::Null)
            }
            "printf" => {
                for arg in arguments {
                    let arg_type = self.compile_expression(arg)?;

                    let fmt = match arg_type {
                        ValueType::Integer => "fmt_int",
                        ValueType::Float => "fmt_float",
                        ValueType::String => "fmt_str",
                        ValueType::Boolean => {
                            self.output.push("    cmp x0, #0".to_string());
                            self.output.push("    adrp x0, fmt_bool_false@PAGE".to_string());
                            self.output.push("    add x0, x0, fmt_bool_false@PAGEOFF".to_string());
                            self.output.push("    adrp x1, fmt_bool_true@PAGE".to_string());
                            self.output.push("    add x1, x1, fmt_bool_true@PAGEOFF".to_string());
                            self.output.push("    csel x0, x1, x0, ne".to_string());
                            self.output.push("    bl _printf".to_string());
                            continue;
                        }
                        ValueType::Null => "fmt_null",
                    };

                    self.output.push("    mov x1, x0".to_string());
                    self.output.push(format!("    adrp x0, {}@PAGE", fmt));
                    self.output.push(format!("    add x0, x0, {}@PAGEOFF", fmt));
                    self.output.push("    bl _printf".to_string());
                }
                Ok(ValueType::Null)
            }
            _ => Err(format!("Unknown io function: {}", function_name)),
        }
    }

    fn hash_variable_name(&self, name: &str) -> usize {
        let mut hash: usize = 0;
        for byte in name.bytes() {
            hash = hash.wrapping_mul(31).wrapping_add(byte as usize);
        }
        hash % 1000
    }

    fn escape_string(&self, s: &str) -> String {
        let mut result = String::from("\"");
        for ch in s.chars() {
            match ch {
                '\n' => result.push_str("\\n"),
                '\t' => result.push_str("\\t"),
                '"' => result.push_str("\\\""),
                '\\' => result.push_str("\\\\"),
                _ => result.push(ch),
            }
        }
        result.push('"');
        result
    }
}

impl Default for AArch64MacOSCodeGen {
    fn default() -> Self {
        Self::new()
    }
}
