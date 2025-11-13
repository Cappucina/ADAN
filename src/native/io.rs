use inkwell::values::{BasicValueEnum, PointerValue};
use inkwell::AddressSpace;
use crate::code_gen::builder::CodeGenContext;

static LIBRARY_NAME: &str = "io";

fn add_printf_support<'ctx>(ctx: &mut CodeGenContext<'ctx>) -> inkwell::values::FunctionValue<'ctx> {
    if let Some(func) = ctx.module.get_function("printf") {
        func
    } else {
        let i8ptr_type = ctx.context.i8_type().ptr_type(AddressSpace::from(0));
        let printf_type = ctx.context.i32_type().fn_type(&[i8ptr_type.into()], true);
       
        ctx.module.add_function("printf", printf_type, None)
    }
}

pub fn printf<'ctx>(ctx: &mut CodeGenContext<'ctx>, args: Vec<BasicValueEnum<'ctx>>) -> BasicValueEnum<'ctx> {
    let printf_fn = add_printf_support(ctx);
    let fmt_str = ctx.context.const_string(b"%s\n\0", true);
    
    let g_fmt = ctx.module.add_global(fmt_str.get_type(), None, "fmt_str");
    g_fmt.set_initializer(&fmt_str);

    let fmt_ptr = g_fmt.as_pointer_value();
    let str_ptr = args[0].into_pointer_value();
    let str_i8 = ctx.builder.build_pointer_cast(
        str_ptr,
        ctx.context.i8_type().ptr_type(AddressSpace::from(0)),
        "cast_str"
    ).expect("pointer casting failed");

    ctx.builder.build_call(
        printf_fn,
        &[fmt_ptr.into(), str_i8.into()],
        "call_printf",
    );

    args[0]
}

pub fn register_native<'ctx>(ctx: &mut CodeGenContext<'ctx>) {
    ctx.register_native_fn(LIBRARY_NAME, "printf", printf);

    // println!("Added '{}' to module registry", LIBRARY_NAME);
}

