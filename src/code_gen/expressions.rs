use crate::parser::ast::{Expr, Literal, Operation};
use crate::code_gen::builder::CodeGenContext;
use crate::code_gen::statements::{codegen_function, NativeRegisterFn};
use crate::code_gen::builder::NativeFunc;
use inkwell::values::*;
use inkwell::AddressSpace;
use std::collections::HashMap;

fn build_float_mod<'ctx>(ctx: &mut CodeGenContext<'ctx>, lhs: FloatValue<'ctx>, rhs: FloatValue<'ctx>) -> Result<FloatValue<'ctx>, String> {
    let f64_type = ctx.context.f64_type();
    let fmod_fn = ctx.module.get_function("fmod").unwrap_or_else(|| {
        let fn_type = f64_type.fn_type(&[f64_type.into(), f64_type.into()], false);
        ctx.module.add_function("fmod", fn_type, None)
    });

    let call_site = ctx.builder
        .build_call(fmod_fn, &[lhs.into(), rhs.into()], "fmodtmp")
        .map_err(|e| format!("call fmod failed: {:?}", e))?;
    let valkind = unsafe { std::mem::transmute::<_, BasicValueEnum>(call_site.try_as_basic_value()) };
    Ok(valkind.into_float_value())
}

pub fn codegen_expressions<'ctx>(ctx: &mut CodeGenContext<'ctx>, expr: &Expr, registry: &HashMap<String, NativeRegisterFn<'ctx>>) -> Result<BasicValueEnum<'ctx>, String> {
    match expr {
        Expr::Literal(lit) => match lit {
            Literal::Number(n) => Ok(ctx.context.f64_type().const_float(*n).into()),
            Literal::Bool(b) => Ok(ctx.context.bool_type().const_int(*b as u64, false).into()),
            Literal::Nil => Ok(ctx.context.ptr_type(AddressSpace::from(0)).const_null().into()),
            Literal::String(s) => {
                let array = ctx.context.const_string(s.as_bytes(), true);
                let global = ctx.module.add_global(array.get_type(), None, "str");

                global.set_initializer(&array);
                Ok(global.as_pointer_value().into())
            }
            _ => Err(format!("Literal kind not implemented: {:?}", lit)),
        },

        Expr::Unary { op, right } => {
            match op {
                Operation::Negate => {
                    let r = codegen_expressions(ctx, right, registry)?.into_float_value();
                    let fv = ctx.builder
                        .build_float_neg(r, "negtmp")
                        .map_err(|e| format!("float neg failed: {:?}", e))?;
                    Ok(fv.into())
                }
                Operation::Not => {
                    let r = codegen_expressions(ctx, right, registry)?.into_int_value();
                    let iv = ctx.builder
                        .build_not(r, "nottmp")
                        .map_err(|e| format!("not op failed: {:?}", e))?;
                    Ok(iv.into())
                }
                _ => Err(format!("Unary op not implemented: {:?}", op)),
            }
        }

        Expr::Assign { name, value } => {
            let val = codegen_expressions(ctx, value, registry)?;
            let var_pointer = ctx.variables
                .get(name)
                .ok_or_else(|| format!("Variable not declared: {}", name))?;
            ctx.builder
                .build_store(*var_pointer, val)
                .map_err(|e| format!("store assign failed: {:?}", e))?;
            Ok(val)
        }

        Expr::FCall { callee, args } => {
            let parts: Vec<&str> = callee.split('.').collect();
            let (module_name, func_name) = if parts.len() > 1 {
                (parts[..parts.len() - 1].join("."), parts.last().unwrap())
            } else {
                ("".to_string(), &parts[0])
            };

            if !ctx.modules.contains_key(&module_name) {
                if let Some(register_fn) = registry.get(&module_name) {
                    register_fn(ctx);
                    //println!("Native module '{}' auto-registered", module_name);
                }
            }

            let func_opt = ctx.modules.get(&module_name).and_then(|m| m.get_function(func_name)).cloned();
            match func_opt {
                Some(NativeFunc::AdanFunction(adan_func)) => {
                    let llvm_fn = codegen_function(ctx, &adan_func, registry).map_err(|e| format!("codegen ADAN function failed: {:?}", e))?;
                    let arg_vals: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| codegen_expressions(ctx, a, registry)).collect::<Result<_, _>>()?;
                    let metadata_args: Vec<BasicMetadataValueEnum> = arg_vals.iter().map(|v| (*v).into()).collect();
                    let call_site = ctx.builder.build_call(llvm_fn, &metadata_args, "calltmp").map_err(|e| format!("call failed: {:?}", e))?;
                    let valkind = unsafe { std::mem::transmute::<_, BasicValueEnum>(call_site.try_as_basic_value()) };
                    Ok(valkind)
                }
        
                Some(NativeFunc::NativeFn(native_fn)) => {
                    let arg_vals: Vec<BasicValueEnum<'ctx>> = args.iter().map(|a| codegen_expressions(ctx, a, registry)).collect::<Result<_, _>>()?;
                    Ok(native_fn(ctx, arg_vals))
                }
        
                None => Err(format!("Function '{}' not defined in module '{}'", func_name, module_name)),
            }
        }

        Expr::Variable { var_name, var_type } => {
            let var_pointer = ctx.variables
                .get(var_name)
                .ok_or_else(|| format!("Variable not declared: {}", var_name))?;
            
            let llvm_type = match var_type {
                Some(t) => ctx.get_llvm_type(*t),
                None => ctx.string_type.into(),
            };

            let loaded = ctx.builder
                .build_load(llvm_type, *var_pointer, "loadtmp")
                .map_err(|e| format!("load failed for variable '{}': {:?}", var_name, e))?;
            
            Ok(loaded)
        }

        Expr::Binary { left, op, right } => {
            let l_val = codegen_expressions(ctx, left, registry)?;
            let r_val = codegen_expressions(ctx, right, registry)?;

            match (l_val, r_val) {
                (BasicValueEnum::FloatValue(lf), BasicValueEnum::FloatValue(rf)) => {
                    let res = match op {
                        Operation::Add => ctx.builder.build_float_add(lf, rf, "addtmp").map_err(|e| e.to_string())?,
                        Operation::Subtract => ctx.builder.build_float_sub(lf, rf, "subtmp").map_err(|e| e.to_string())?,
                        Operation::Multiply => ctx.builder.build_float_mul(lf, rf, "multmp").map_err(|e| e.to_string())?,
                        Operation::Divide => ctx.builder.build_float_div(lf, rf, "divtmp").map_err(|e| e.to_string())?,
                        Operation::Modulo => build_float_mod(ctx, lf, rf).map_err(|e| e.to_string())?,
                        _ => return Err(format!("Unsupported float binary op {:?}", op)),
                    };
                    Ok(res.into())
                }
                (BasicValueEnum::IntValue(li), BasicValueEnum::IntValue(ri)) => {
                    let res = match op {
                        Operation::Add => ctx.builder.build_int_add(li, ri, "addtmp").map_err(|e| e.to_string())?,
                        Operation::Subtract => ctx.builder.build_int_sub(li, ri, "subtmp").map_err(|e| e.to_string())?,
                        Operation::Multiply => ctx.builder.build_int_mul(li, ri, "multmp").map_err(|e| e.to_string())?,
                        Operation::Divide => ctx.builder.build_int_signed_div(li, ri, "divtmp").map_err(|e| e.to_string())?,
                        Operation::Modulo => ctx.builder.build_int_signed_rem(li, ri, "modtmp").map_err(|e| e.to_string())?,
                        _ => return Err(format!("Unsupported int binary op {:?}", op)),
                    };
                    Ok(res.into())
                }
                (BasicValueEnum::PointerValue(lp), BasicValueEnum::PointerValue(rp)) => {
                    let strcmp_fn = ctx.module.get_function("strcmp").unwrap_or_else(|| {
                        let i8_ptr_type = ctx.context.i8_type().ptr_type(AddressSpace::from(0));
                        let fn_type = ctx.context.i32_type().fn_type(&[i8_ptr_type.into(), i8_ptr_type.into()], false);
        
                        ctx.module.add_function("strcmp", fn_type, None)
                    });

                    let i8_ptr_type = ctx.context.i8_type().ptr_type(AddressSpace::from(0));
                    let lp_cast = ctx.builder.build_bit_cast(lp, i8_ptr_type, "cast_lp").unwrap().into_pointer_value();
                    let rp_cast = ctx.builder.build_bit_cast(rp, i8_ptr_type, "cast_rp").unwrap().into_pointer_value();

                    let call = ctx.builder.build_call(strcmp_fn, &[lp_cast.into(), rp_cast.into()], "strcmpcall").map_err(|e| format!("strcmp call failed: {:?}", e))?; 
                    let valkind = unsafe { std::mem::transmute::<_, BasicValueEnum>(call.try_as_basic_value()) };
                    let int_res = match valkind {
                        BasicValueEnum::IntValue(v) => v,
                        _ => return Err("Expected IntValue from strcmp".to_string()),
                    };  

                    let cond = ctx.builder.build_int_compare(inkwell::IntPredicate::NE, int_res, ctx.context.i32_type().const_zero(), "strcmp_cond").unwrap();
                    //let float_result = ctx.builder.build_unsigned_int_to_float(int_res, ctx.context.f64_type(), "strcmd_float").unwrap();

                    Ok(BasicValueEnum::IntValue(cond))
                }

                _ => Err(format!("Type mismatch in binary operation")),
            }
        }

        _ => Err(format!("Expression variant not implemented: {:?}", expr)),
    }
}
