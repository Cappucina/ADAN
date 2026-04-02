#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* llvm_type_to_string(IRType* t)
{
	if (!t)
	{
		return NULL;
	}
	switch (t->kind)
	{
		case IR_T_VOID:
			return strdup("void");
		case IR_T_I1:
		case IR_T_BOOL:
			return strdup("i1");
		case IR_T_I8:
		case IR_T_U8:
			return strdup("i8");
		case IR_T_I16:
		case IR_T_U16:
			return strdup("i16");
		case IR_T_I32:
		case IR_T_U32:
			return strdup("i32");
		case IR_T_I64:
		case IR_T_U64:
		case IR_T_INTPTR:
		case IR_T_UINTPTR:
			return strdup("i64");
		case IR_T_F32:
			return strdup("float");
		case IR_T_F64:
			return strdup("double");
		case IR_T_PTR:
		{
			char* pointee_str = llvm_type_to_string(t->pointee);
			if (!pointee_str)
			{
				return strdup("i64*");
			}
			size_t len = strlen(pointee_str) + 2;
			char* result = (char*)malloc(len);
			if (!result)
			{
				free(pointee_str);
				return NULL;
			}
			snprintf(result, len, "%s*", pointee_str);
			free(pointee_str);
			return result;
		}
		default:
			return NULL;
	}
}

char* llvm_type_mangle(IRType* t)
{
	if (!t)
	{
		return NULL;
	}
	switch (t->kind)
	{
		case IR_T_VOID:
			return strdup("v");
		case IR_T_I1:
			return strdup("i1");
		case IR_T_BOOL:
			return strdup("b");
		case IR_T_I8:
			return strdup("i8");
		case IR_T_U8:
			return strdup("u8");
		case IR_T_I16:
			return strdup("i16");
		case IR_T_U16:
			return strdup("u16");
		case IR_T_I32:
			return strdup("i32");
		case IR_T_U32:
			return strdup("u32");
		case IR_T_I64:
			return strdup("i64");
		case IR_T_U64:
			return strdup("u64");
		case IR_T_INTPTR:
			return strdup("isize");
		case IR_T_UINTPTR:
			return strdup("usize");
		case IR_T_F32:
			return strdup("f32");
		case IR_T_F64:
			return strdup("f64");
		case IR_T_PTR:
		{
			char* pointee_str = llvm_type_mangle(t->pointee);
			if (!pointee_str)
			{
				return strdup("Pi64");
			}
			size_t len = strlen(pointee_str) + 2;
			char* result = (char*)malloc(len);
			if (!result)
			{
				free(pointee_str);
				return NULL;
			}
			snprintf(result, len, "P%s", pointee_str);
			free(pointee_str);
			return result;
		}
		default:
			return NULL;
	}
}