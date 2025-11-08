use crate::parser::ast::{Expr, Literal, Operation};
use super::CodeGenContext;
use inkwell::values::{BasicValueEnum, FloatValue};
use inkwell::AddressSpace;

// todo: finish ./builder for `ctx.builder`

fn build_float_mod<'ctx>(ctx: &mut CodeGenContext<'ctx>, lhs: FloatValue<'ctx>, rhs: FloatValue<'ctx>) -> FloatValue<'ctx> {
    let f64_type = ctx.context.f64_type();
    let fmod_fn = ctx
        .module
        .get_function("fmod")
        .unwrap_or_else(|| {
            let fn_type = f64_type.fn_type(&[f64_type.into(), f64_type.into()], false);
            
            ctx.module.add_function("fmod", fn_type, None)
        });
    
    let result = ctx.builder.build_call(fmod_fn, &[lhs.into(), rhs.into()], "fmodtmp");
    result.try_as_basic_value().left().unwrap().into_float_value()
}

// Translate various expression types -> LLVM types.
// +, /, -, *, %
pub fn codegen_expressions<'ctx>(ctx: &mut CodeGenContext<'ctx>, expr: &Expr) -> BasicValueEnum<'ctx> {
    match expr {
        Expr::Literal(lit) => match lit {
            Literal::Number(n) => ctx.context.f64_type().const_float(*n).into(),
            Literal::Bool(b) => ctx.context.bool_type().const_int(*b as u64, false).into(),
        
            _ => unimplemented!(),
        },

        Expr::Unary { op, right } => {
            match op {
                Operation::Negate => {
                    let r = codegen_expressions(ctx, right).into_float_value();

                    ctx.builder.build_float_neg(r, "negtmp").into() // Flips x -> -x
                },

                Operation::Not => {
                    let r = codegen_expressions(ctx, right).into_int_value();

                    ctx.builder.build_not(r, "nottmp").into() // Flips true -> false, false -> true
                },
            }
        },

        Expr::Assign { name, value } => {
            let val = code_expressions(ctx, value);
            let var_pointer = ctx.variables.get(name).expect("Variable not declared");

            ctx.builder.build_store(*var_pointer, val);
            val
        },

        Expr::FCall { callee, args } => {
            let func = ctx.function.get_function(callee).expect("Function not defined");
            let arg_vals: Vec<BasicValueEnum> = args
                .iter()
                .map(|arg| codegen_expressions(ctx, arg))
                .collect();

            let call = ctx.builder.build_call(func, &arg_vals, "calltmp");
            call.try_as_basic_value().left().unwrap_or_else(|| {
                panic!("Function did not return a value");
            })
        },

        Expr::Variable(var_name) => {
            let var_pointer = ctx.variables.get(var_name).expect("Variable not declared");

            ctx.builder.build_load(*var_pointer, var_name)
        } 

        Expr::Binary { left, op, right } => {
            let l = codegen_expressions(ctx, left).into_float_value();
            let r = codegen_expressions(ctx, right).into_float_value();
            
            match op {
                Operation::Add => ctx.builder.build_float_add(l, r, "addtmp").into(),
                Operation::Subtract => ctx.builder.build_float_sub(l, r, "subtmp").into(),
                Operation::Multiply => ctx.builder.build_float_mul(l, r, "multmp").into(),
                Operation::Divide => ctx.builder.build_float_div(l, r, "divtmp").into(),
                Operation::Modulo => build_float_mod(ctx, l, r).into(),
             
                _ => unimplemented!(),
            }
        }
        _ => unimplemented!(),
    }
}
