use inkwell::{context::Context, builder::Builder, module::Module};

pub struct CodeGenContext<'ctx> {
    pub context: &'ctx Context,
    pub builder: Builder<'ctx>,
    pub module: Module<'ctx>,
}

impl<'ctx> CodeGenContext<'ctx> {
    pub fn new(context: &'ctx Context, name: &str) -> Self {
        let module = context.create_module(name);
        let builder = context.create_builder();

        Self { context, builder, module }
    }
}
