use std::collections::HashMap;
use std::fs;
use std::path::Path;
use inkwell::{
    builder::Builder,
    context::Context,
    module::Module,
    values::{BasicValueEnum, PointerValue},
    types::{FloatType, IntType, ArrayType, StructType, PointerType, BasicTypeEnum},
};
use crate::parser::ast::FunctionDecl;
use inkwell::AddressSpace;
use crate::lexer::token::Types;

pub struct CodeGenContext<'ctx> {
    pub context: &'ctx Context,
    pub builder: Builder<'ctx>,
    pub module: Module<'ctx>,

    pub f64_type: FloatType<'ctx>,
    pub bool_type: IntType<'ctx>,
    pub i8_type: IntType<'ctx>,
    pub i32_type: IntType<'ctx>,
    pub i64_type: IntType<'ctx>,
    pub u8_type: IntType<'ctx>,
    pub u32_type: IntType<'ctx>,
    pub u64_type: IntType<'ctx>,
    pub array_of_i32: ArrayType<'ctx>,
    pub my_object_type: StructType<'ctx>,
    pub string_type: PointerType<'ctx>,

    pub variables: HashMap<String, PointerValue<'ctx>>,
    pub modules: HashMap<String, ModuleValue<'ctx>>,
}

#[derive(Clone)]
pub enum NativeFunc<'ctx> {
    AdanFunction(FunctionDecl),
    NativeFn(fn(&mut CodeGenContext<'ctx>, Vec<BasicValueEnum<'ctx>>) -> BasicValueEnum<'ctx>),
}

pub struct ModuleValue<'ctx> {
    pub functions: HashMap<String, NativeFunc<'ctx>>,
    pub variables: HashMap<String, PointerValue<'ctx>>,
}

impl<'ctx> ModuleValue<'ctx> {
    pub fn get_function(&self, name: &str) -> Option<&NativeFunc<'ctx>> {
        self.functions.get(name)
    }
}

impl<'ctx> CodeGenContext<'ctx> {
    pub fn new(context: &'ctx Context, name: &str) -> Self {
        let module = context.create_module(name);
        let builder = context.create_builder();

        Self {
            context,
            builder,
            module,
            f64_type: context.f64_type(),
            bool_type: context.bool_type(),
            i8_type: context.i8_type(),
            i32_type: context.i32_type(),
            i64_type: context.i64_type(),
            u8_type: context.i8_type(),
            u32_type: context.i32_type(),
            u64_type: context.i64_type(),
            string_type: context.i8_type().ptr_type(AddressSpace::from(0)),
            array_of_i32: context.i32_type().array_type(10),
            my_object_type: context.struct_type(&[
                context.i32_type().into(),
                context.i8_type().ptr_type(AddressSpace::from(0)).into()
            ], false),
            variables: HashMap::new(),
            modules: HashMap::new(),
        }
    }

    pub fn register_native_fn(&mut self, module_name: &str, fn_name: &str, func: fn(&mut CodeGenContext<'ctx>, Vec<BasicValueEnum<'ctx>>) -> BasicValueEnum<'ctx>) {
        let module = self.modules.entry(module_name.to_string())
            .or_insert(ModuleValue {
                functions: HashMap::new(),
                variables: HashMap::new(),
            });

        //println!("Registered function '{}' in module '{}'", fn_name, module_name);

        module.functions.insert(fn_name.to_string(), NativeFunc::NativeFn(func));
    }

    pub fn load_native_modules(&mut self, native_dir: &str) {
        let paths = fs::read_dir(native_dir).expect("Failed to read native modules folder");

        for entry in paths {
            let entry = entry.expect("Failed to read entry");
            let path = entry.path();
            if path.is_file() && path.extension().map(|s| s == "rs").unwrap_or(false) {
                let module_name = path.file_stem().unwrap().to_string_lossy();

                match module_name.as_ref() {
                    "io" => crate::native::io::register_native(self),
                    _ => {}
                }
            }
        }
    }

    pub fn build_alloca(&self, name: &str) -> Result<PointerValue<'ctx>, inkwell::builder::BuilderError> {
        let function = self.builder.get_insert_block().unwrap().get_parent().unwrap();
        let entry = function.get_first_basic_block().unwrap();
        let builder = self.context.create_builder();

        builder.position_at_end(entry);
        builder.build_alloca(self.f64_type, name)
    }

    pub fn build_return(&self, value: Option<BasicValueEnum<'ctx>>) {
        match value {
            Some(v) => { let _ = self.builder.build_return(Some(&v)); },
            None => { let _ = self.builder.build_return(None); },
        };
    }

    pub fn get_llvm_type(&self, var_type: Types) -> BasicTypeEnum<'ctx> {
        match var_type {
            Types::i8 => self.i8_type.into(),
            Types::i32 => self.i32_type.into(),
            Types::i64 => self.i64_type.into(),
            Types::u8 => self.u8_type.into(),
            Types::u32 => self.u32_type.into(),
            Types::u64 => self.u64_type.into(),
            Types::f32 => self.context.f32_type().into(),
            Types::f64 => self.f64_type.into(),
            Types::Boolean => self.bool_type.into(),
            Types::Char => self.i8_type.into(),
            Types::String => self.string_type.into(),
            Types::Array => self.array_of_i32.into(),
            Types::Object => self.my_object_type.into(),
        }
    }
}
