#include <stddef.h>

#define _POSIX_C_SOURCE 200809L
#if defined(_WIN32)
#define strdup _strdup
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/ast.h"

ASTNode* ast_empty(void)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node)
    {
        return NULL;
    }
    node->type = AST_EMPTY;
    node->data = NULL;
    return node;
}

ASTNode* ast_allowed_top_level(ASTNode* includes, ASTNode* top_levels)
{
    ASTNode* program_node = malloc(sizeof(ASTNode));
    if (!program_node)
    {
        return NULL;
    }
    program_node->type = AST_PROGRAM;

    typedef struct
    {
        ASTNode* includes;
        ASTNode* top_levels;
    } ProgramData;

    ProgramData* data = malloc(sizeof(ProgramData));
    if (!data)
    {
        free(program_node);
        return NULL;
    }
    data->includes = includes;
    data->top_levels = top_levels;

    program_node->data = data;
    program_node->next = NULL;

    return program_node;
}

ASTNode* ast_type_array(ASTNode* base, int size)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    typedef struct
    {
        ASTNode* base;
        int size;
    } ArrayTypeData;

    ArrayTypeData* data = malloc(sizeof(ArrayTypeData));
    if (!data)
    {
        free(type_node);
        return NULL;
    }
    data->base = base;
    data->size = size;

    type_node->data = data;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_append_include(ASTNode* list, ASTNode* include)
{
    if (!list)
    {
        return include;
    }

    ASTNode* current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = include;
    return list;
}

ASTNode* ast_include(const char* name, ASTNode* tail)
{
    ASTNode* include_node = malloc(sizeof(ASTNode));
    if (!include_node)
    {
        return NULL;
    }
    include_node->type = AST_INCLUDE;

    char* name_copy = strdup(name);
    if (!name_copy)
    {
        free(include_node);
        return NULL;
    }

    include_node->data = name_copy;
    include_node->next = tail;

    return include_node;
}

ASTNode* ast_include_tail(const char* name, ASTNode* tail)
{
    return ast_include(name, tail);
}

ASTNode* ast_append_top_level(ASTNode* list, ASTNode* top_level)
{
    if (!list)
    {
        return top_level;
    }

    ASTNode* current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = top_level;
    return list;
}

ASTNode* ast_append_param(ASTNode* list, ASTNode* param)
{
    if (!list)
    {
        return param;
    }

    ASTNode* current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = param;
    return list;
}

ASTNode* ast_single_param(ASTNode* param)
{
    return param;
}

ASTNode* ast_param(const char* name, ASTNode* type)
{
    ASTNode* param_node = malloc(sizeof(ASTNode));
    if (!param_node)
    {
        return NULL;
    }
    param_node->type = AST_PARAM;

    typedef struct
    {
        char* name;
        ASTNode* type;
    } ParamData;

    ParamData* data = malloc(sizeof(ParamData));
    if (!data)
    {
        free(param_node);
        return NULL;
    }
    data->name = strdup(name);
    if (!data->name)
    {
        free(data);
        free(param_node);
        return NULL;
    }
    data->type = type;

    param_node->data = data;
    param_node->next = NULL;

    return param_node;
}

ASTNode* ast_append_struct_member(ASTNode* list, ASTNode* member)
{
    if (!list)
    {
        return member;
    }

    ASTNode* current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = member;
    return list;
}

ASTNode* ast_struct(const char* name, ASTNode* members)
{
    ASTNode* struct_node = malloc(sizeof(ASTNode));
    if (!struct_node)
    {
        return NULL;
    }
    struct_node->type = AST_STRUCT_DEF;

    typedef struct
    {
        char* name;
        ASTNode* members;
    } StructData;

    StructData* data = malloc(sizeof(StructData));
    if (!data)
    {
        free(struct_node);
        return NULL;
    }
    data->name = strdup(name);
    if (!data->name)
    {
        free(data);
        free(struct_node);
        return NULL;
    }
    data->members = members;

    struct_node->data = data;
    struct_node->next = NULL;

    return struct_node;
}

ASTNode* ast_type_void(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("void");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_int(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("int");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_float(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("float");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_string(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("string");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_bool(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("bool");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_char(void)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup("char");
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_user(const char* name)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    char* type_name = strdup(name);
    if (!type_name)
    {
        free(type_node);
        return NULL;
    }

    type_node->data = type_name;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_type_pointer(ASTNode* base, ASTNode* stars)
{
    ASTNode* type_node = malloc(sizeof(ASTNode));
    if (!type_node)
    {
        return NULL;
    }
    type_node->type = AST_TYPE;

    typedef struct
    {
        ASTNode* base;
        ASTNode* stars;
    } PointerTypeData;

    PointerTypeData* data = malloc(sizeof(PointerTypeData));
    if (!data)
    {
        free(type_node);
        return NULL;
    }
    data->base = base;
    data->stars = stars;

    type_node->data = data;
    type_node->next = NULL;

    return type_node;
}

ASTNode* ast_append_pointer_star(ASTNode* stars)
{
    ASTNode* star_node = malloc(sizeof(ASTNode));
    if (!star_node)
    {
        return NULL;
    }
    star_node->type = AST_TYPE;

    char* star_symbol = strdup("*");
    if (!star_symbol)
    {
        free(star_node);
        return NULL;
    }

    star_node->data = star_symbol;
    star_node->next = stars;

    return star_node;
}

ASTNode* ast_append_stmt(ASTNode* list, ASTNode* stmt)
{
    if (!list)
    {
        return stmt;
    }

    ASTNode* current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = stmt;
    return list;
}

ASTNode* ast_code_block(ASTNode* stmts)
{
    ASTNode* block_node = malloc(sizeof(ASTNode));
    if (!block_node)
    {
        return NULL;
    }
    block_node->type = AST_CODE_BLOCK;
    block_node->data = stmts;
    block_node->next = NULL;

    return block_node;
}

ASTNode* ast_if_stmt(ASTNode* cond, ASTNode* then_block, ASTNode* else_block)
{
    ASTNode* if_node = malloc(sizeof(ASTNode));
    if (!if_node)
    {
        return NULL;
    }
    if_node->type = AST_IF;

    typedef struct
    {
        ASTNode* condition;
        ASTNode* then_block;
        ASTNode* else_block;
    } IfStmtData;

    IfStmtData* data = malloc(sizeof(IfStmtData));
    if (!data)
    {
        free(if_node);
        return NULL;
    }
    data->condition = cond;
    data->then_block = then_block;
    data->else_block = else_block;

    if_node->data = data;
    if_node->next = NULL;

    return if_node;
}

ASTNode* ast_while_stmt(ASTNode* cond, ASTNode* block)
{
    ASTNode* while_node = malloc(sizeof(ASTNode));
    if (!while_node)
    {
        return NULL;
    }
    while_node->type = AST_WHILE;

    typedef struct
    {
        ASTNode* condition;
        ASTNode* block;
    } WhileStmtData;

    WhileStmtData* data = malloc(sizeof(WhileStmtData));
    if (!data)
    {
        free(while_node);
        return NULL;
    }
    data->condition = cond;
    data->block = block;

    while_node->data = data;
    while_node->next = NULL;

    return while_node;
}

ASTNode* ast_for_stmt(ASTNode* init, ASTNode* cond, ASTNode* increment, ASTNode* block)
{
    ASTNode* for_node = malloc(sizeof(ASTNode));
    if (!for_node)
    {
        return NULL;
    }
    for_node->type = AST_FOR;

    typedef struct
    {
        ASTNode* init;
        ASTNode* condition;
        ASTNode* increment;
        ASTNode* block;
    } ForStmtData;

    ForStmtData* data = malloc(sizeof(ForStmtData));
    if (!data)
    {
        free(for_node);
        return NULL;
    }
    data->init = init;
    data->condition = cond;
    data->increment = increment;
    data->block = block;

    for_node->data = data;
    for_node->next = NULL;

    return for_node;
}

ASTNode* ast_return_stmt(ASTNode* expr)
{
    ASTNode* return_node = malloc(sizeof(ASTNode));
    if (!return_node)
    {
        return NULL;
    }
    return_node->type = AST_RETURN;
    return_node->data = expr;
    return_node->next = NULL;

    return return_node;
}

ASTNode* ast_break_stmt(void)
{
    ASTNode* break_node = malloc(sizeof(ASTNode));
    if (!break_node)
    {
        return NULL;
    }
    break_node->type = AST_BREAK;
    break_node->data = NULL;
    break_node->next = NULL;

    return break_node;
}

ASTNode* ast_continue_stmt(void)
{
    ASTNode* continue_node = malloc(sizeof(ASTNode));
    if (!continue_node)
    {
        return NULL;
    }
    continue_node->type = AST_CONTINUE;
    continue_node->data = NULL;
    continue_node->next = NULL;

    return continue_node;
}

ASTNode* ast_expr_stmt(ASTNode* expr)
{
    ASTNode* expr_stmt_node = malloc(sizeof(ASTNode));
    if (!expr_stmt_node)
    {
        return NULL;
    }
    expr_stmt_node->type = AST_EXPR_STMT;
    expr_stmt_node->data = expr;
    expr_stmt_node->next = NULL;

    return expr_stmt_node;
}

ASTNode* ast_assignment(ASTNode* lhs, ASTNode* op, ASTNode* rhs)
{
    ASTNode* assignment_node = malloc(sizeof(ASTNode));
    if (!assignment_node)
    {
        return NULL;
    }
    assignment_node->type = AST_ASSIGNMENT;

    typedef struct
    {
        ASTNode* lhs;
        ASTNode* op;
        ASTNode* rhs;
    } AssignmentData;

    AssignmentData* data = malloc(sizeof(AssignmentData));
    if (!data)
    {
        free(assignment_node);
        return NULL;
    }
    data->lhs = lhs;
    data->op = op;
    data->rhs = rhs;

    assignment_node->data = data;
    assignment_node->next = NULL;

    return assignment_node;
}

ASTNode* ast_binary_op(const char* op, ASTNode* lhs, ASTNode* rhs)
{
    ASTNode* binary_op_node = malloc(sizeof(ASTNode));
    if (!binary_op_node)
        return NULL;
    binary_op_node->type = AST_BINARY_OP;

    typedef struct
    {
        char* op;
        ASTNode* lhs;
        ASTNode* rhs;
    } BinaryOpData;

    BinaryOpData* data = malloc(sizeof(BinaryOpData));
    if (!data)
    {
        free(binary_op_node);
        return NULL;
    }
    data->op = strdup(op);
    data->lhs = lhs;
    data->rhs = rhs;

    binary_op_node->data = data;
    binary_op_node->next = NULL;
    return binary_op_node;
}

ASTNode* ast_unary_op(const char* op, ASTNode* expr)
{
    ASTNode* unary_op_node = malloc(sizeof(ASTNode));
    if (!unary_op_node)
        return NULL;
    unary_op_node->type = AST_UNARY_OP;

    typedef struct
    {
        char* op;
        ASTNode* expr;
    } UnaryOpData;

    UnaryOpData* data = malloc(sizeof(UnaryOpData));
    if (!data)
    {
        free(unary_op_node);
        return NULL;
    }
    data->op = strdup(op);
    data->expr = expr;

    unary_op_node->data = data;
    unary_op_node->next = NULL;
    return unary_op_node;
}

ASTNode* ast_postfix_op(const char* op, ASTNode* expr)
{
    ASTNode* postfix_op_node = malloc(sizeof(ASTNode));
    if (!postfix_op_node)
        return NULL;
    postfix_op_node->type = AST_POSTFIX_OP;

    typedef struct
    {
        char* op;
        ASTNode* expr;
    } PostfixOpData;

    PostfixOpData* data = malloc(sizeof(PostfixOpData));
    if (!data)
    {
        free(postfix_op_node);
        return NULL;
    }
    data->op = strdup(op);
    data->expr = expr;

    postfix_op_node->data = data;
    postfix_op_node->next = NULL;
    return postfix_op_node;
}

ASTNode* ast_identifier(const char* name)
{
    ASTNode* id_node = malloc(sizeof(ASTNode));
    if (!id_node)
    {
        return NULL;
    }
    id_node->type = AST_IDENTIFIER;

    char* name_copy = strdup(name);
    if (!name_copy)
    {
        free(id_node);
        return NULL;
    }

    id_node->data = name_copy;
    id_node->next = NULL;

    return id_node;
}

ASTNode* ast_int_literal(int value)
{
    ASTNode* int_node = malloc(sizeof(ASTNode));
    if (!int_node)
    {
        return NULL;
    }
    int_node->type = AST_INT_LITERAL;

    int* value_ptr = malloc(sizeof(int));
    if (!value_ptr)
    {
        free(int_node);
        return NULL;
    }
    *value_ptr = value;

    int_node->data = value_ptr;
    int_node->next = NULL;

    return int_node;
}

ASTNode* ast_float_literal(double value)
{
    ASTNode* float_node = malloc(sizeof(ASTNode));
    if (!float_node)
    {
        return NULL;
    }
    float_node->type = AST_FLOAT_LITERAL;

    double* value_ptr = malloc(sizeof(double));
    if (!value_ptr)
    {
        free(float_node);
        return NULL;
    }
    *value_ptr = value;

    float_node->data = value_ptr;
    float_node->next = NULL;

    return float_node;
}

ASTNode* ast_string_literal(const char* value)
{
    ASTNode* string_node = malloc(sizeof(ASTNode));
    if (!string_node)
    {
        return NULL;
    }
    string_node->type = AST_STRING_LITERAL;

    char* value_copy = strdup(value);
    if (!value_copy)
    {
        free(string_node);
        return NULL;
    }

    string_node->data = value_copy;
    string_node->next = NULL;

    return string_node;
}

ASTNode* ast_char_literal(char value)
{
    ASTNode* char_node = malloc(sizeof(ASTNode));
    if (!char_node)
    {
        return NULL;
    }
    char_node->type = AST_CHAR_LITERAL;

    char* value_ptr = malloc(sizeof(char));
    if (!value_ptr)
    {
        free(char_node);
        return NULL;
    }
    *value_ptr = value;

    char_node->data = value_ptr;
    char_node->next = NULL;

    return char_node;
}

ASTNode* ast_true(void)
{
    ASTNode* true_node = malloc(sizeof(ASTNode));
    if (!true_node)
    {
        return NULL;
    }
    true_node->type = AST_TRUE;
    true_node->data = NULL;
    true_node->next = NULL;

    return true_node;
}

ASTNode* ast_false(void)
{
    ASTNode* false_node = malloc(sizeof(ASTNode));
    if (!false_node)
    {
        return NULL;
    }
    false_node->type = AST_FALSE;
    false_node->data = NULL;
    false_node->next = NULL;

    return false_node;
}

ASTNode* ast_null(void)
{
    ASTNode* null_node = malloc(sizeof(ASTNode));
    if (!null_node)
    {
        return NULL;
    }
    null_node->type = AST_NULL;
    null_node->data = NULL;
    null_node->next = NULL;

    return null_node;
}

ASTNode* ast_variable_def(const char* name, ASTNode* type, ASTNode* initializer)
{
    ASTNode* var_def_node = malloc(sizeof(ASTNode));
    if (!var_def_node)
    {
        return NULL;
    }
    var_def_node->type = AST_VARIABLE_DEF;

    typedef struct
    {
        char* name;
        ASTNode* type;
        ASTNode* initializer;
    } VarDefData;

    VarDefData* data = malloc(sizeof(VarDefData));
    if (!data)
    {
        free(var_def_node);
        return NULL;
    }
    data->name = strdup(name);
    if (!data->name)
    {
        free(data);
        free(var_def_node);
        return NULL;
    }
    data->type = type;
    data->initializer = initializer;

    var_def_node->data = data;
    var_def_node->next = NULL;

    return var_def_node;
}

ASTNode* ast_function_def(ASTNode* return_type, const char* name, ASTNode* params, ASTNode* body)
{
    ASTNode* func_def_node = malloc(sizeof(ASTNode));
    if (!func_def_node)
    {
        return NULL;
    }
    func_def_node->type = AST_FUNCTION_DEF;

    typedef struct
    {
        ASTNode* return_type;
        char* name;
        ASTNode* params;
        ASTNode* body;
    } FuncDefData;

    FuncDefData* data = malloc(sizeof(FuncDefData));
    if (!data)
    {
        free(func_def_node);
        return NULL;
    }
    data->return_type = return_type;
    data->name = strdup(name);
    if (!data->name)
    {
        free(data);
        free(func_def_node);
        return NULL;
    }
    data->params = params;
    data->body = body;

    func_def_node->data = data;
    func_def_node->next = NULL;

    return func_def_node;
}

ASTNode* ast_single_stmt(ASTNode* stmt)
{
    return stmt;
}

ASTNode* ast_list(ASTNode* elements)
{
    ASTNode* list_node = malloc(sizeof(ASTNode));
    if (!list_node)
    {
        return NULL;
    }
    list_node->type = AST_LIST;
    list_node->data = elements;
    list_node->next = NULL;

    return list_node;
}

ASTNode* ast_assign_op(const char* op)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_ASSIGNMENT;
    node->data = strdup(op);
    node->next = NULL;
    return node;
}

ASTNode* ast_block(ASTNode* stmts)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_CODE_BLOCK;
    node->data = stmts;
    node->next = NULL;
    return node;
}

static void ast_print_internal(ASTNode* node, int indent)
{
    if (!node)
        return;
    for (int i = 0; i < indent; i++)
        printf("  ");
    switch (node->type)
    {
        case AST_EMPTY:
            printf("AST_EMPTY\n");
            break;
        case AST_PROGRAM:
            printf("AST_PROGRAM\n");
            break;
        default:
            printf("ASTNodeType: %d\n", node->type);
            break;
    }
    ast_print_internal(node->next, indent);
}

void ast_print(ASTNode* root)
{
    ast_print_internal(root, 0);
}

void ast_free(ASTNode* node)
{
    if (!node)
    {
        return;
    }

    switch (node->type)
    {
        case AST_INCLUDE: {
            free((char*)node->data);
            break;
        }
        case AST_PARAM: {
            typedef struct
            {
                char* name;
                ASTNode* type;
            } ParamData;

            ParamData* data = (ParamData*)node->data;
            free(data->name);
            ast_free(data->type);
            free(data);
            break;
        }
        case AST_STRUCT_DEF: {
            typedef struct
            {
                char* name;
                ASTNode* members;
            } StructData;

            StructData* data = (StructData*)node->data;
            free(data->name);
            ast_free(data->members);
            free(data);
            break;
        }
        default:
            break;
    }

    ast_free(node->next);
    free(node);
}

#include <stdio.h>
void yyerror(const char* s)
{
    fprintf(stderr, "Parse error: %s\n", s);
}