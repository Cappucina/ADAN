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
		case IR_T_I64:
			return strdup("i64");
		case IR_T_F64:
			return strdup("f64");
		case IR_T_PTR:
		{
			char* pointee_str = llvm_type_to_string(t->pointee);
			if (!pointee_str)
			{
				return NULL;
			}
			size_t len =
			    strlen(pointee_str) + 2;  // For the '*' and null terminator stuff
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
		case IR_T_I64:
			return strdup("i");
		case IR_T_F64:
			return strdup("f");
		case IR_T_PTR:
		{
			char* pointee_str = llvm_type_mangle(t->pointee);
			if (!pointee_str)
			{
				return NULL;
			}
			size_t len =
			    strlen(pointee_str) + 2;  // For the 'P' and null terminator stuff
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