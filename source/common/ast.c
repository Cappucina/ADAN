#include "ast.h"

#include <errno.h>

#include "common.h"

void free_ast(ASTNode* node)
{
    if (node == NULL)
    {
        return;
    }

    switch (node->type)
    {
        case AST_IDENT:
            free(node->data.ident.name);
            break;
        case AST_STRING_LITERAL:
            free(node->data.string_literal.value);
            break;
        case AST_BINARY_OP:
            free(node->data.binary.op);
            free_ast(node->data.binary.left);
            free_ast(node->data.binary.right);
            break;
        case AST_UNARY_OP:
            free(node->data.unary.op);
            free_ast(node->data.unary.operand);
            break;
        case AST_BLOCK:
            if (node->data.block.statements)
            {
                for (size_t i = 0; i < node->data.block.count; ++i)
                {
                    free_ast(node->data.block.statements[i]);
                }
                free(node->data.block.statements);
            }
            break;
        case AST_PARAM_LIST:
            if (node->data.param_list.params)
            {
                for (size_t i = 0; i < node->data.param_list.count; ++i)
                {
                    free_ast(node->data.param_list.params[i]);
                }
                free(node->data.param_list.params);
            }
            break;
        case AST_STRUCT:
            free(node->data.struct_decl.name);
            if (node->data.struct_decl.members)
            {
                for (size_t i = 0; i < node->data.struct_decl.count; ++i)
                {
                    free_ast(node->data.struct_decl.members[i]);
                }
                free(node->data.struct_decl.members);
            }
            break;
        case AST_RETURN:
            free_ast(node->data.return_stmt.value);
            break;
        default:
            break;
    }

    free(node);
}

ASTNode* create_ast_node(ASTNodeType type)
{
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));

    if (node == NULL)
    {
        return NULL;
    }

    node->type = type;
    node->line = 0;
    node->column = 0;
    node->file_name = NULL;

    return node;
}

ASTNode* create_ident_node(const char* name)
{
    ASTNode* node = create_ast_node(AST_IDENT);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.ident.name = strdup(name);
    if (node->data.ident.name == NULL)
    {
        free_ast(node);
        return NULL;
    }

    return node;
}

ASTNode* create_string_literal_node(const char* value)
{
    ASTNode* node = create_ast_node(AST_STRING_LITERAL);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.string_literal.value = strdup(value);
    if (node->data.string_literal.value == NULL)
    {
        free_ast(node);
        return NULL;
    }

    return node;
}

ASTNode* create_binary_node(const char* op, ASTNode* left, ASTNode* right)
{
    ASTNode* node = create_ast_node(AST_BINARY_OP);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.binary.op = strdup(op);
    if (node->data.binary.op == NULL)
    {
        free_ast(node);
        return NULL;
    }

    node->data.binary.left = left;
    node->data.binary.right = right;

    return node;
}

ASTNode* create_unary_node(const char* op, ASTNode* operand)
{
    ASTNode* node = create_ast_node(AST_UNARY_OP);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.unary.op = strdup(op);
    if (node->data.unary.op == NULL)
    {
        free_ast(node);
        return NULL;
    }

    node->data.unary.operand = operand;

    return node;
}

ASTNode* create_block_node(ASTNode** statements, size_t count)
{
    ASTNode* node = create_ast_node(AST_BLOCK);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.block.statements = statements;
    node->data.block.count = count;

    return node;
}

ASTNode* create_param_list_node(ASTNode** params, size_t count)
{
    ASTNode* node = create_ast_node(AST_PARAM_LIST);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.param_list.params = params;
    node->data.param_list.count = count;

    return node;
}

ASTNode* create_struct_decl_node(const char* name, ASTNode** members, size_t count)
{
    ASTNode* node = create_ast_node(AST_STRUCT);
    if (node == NULL)
    {
        return NULL;
    }

    node->data.struct_decl.name = strdup(name);
    if (node->data.struct_decl.name == NULL)
    {
        free_ast(node);
        return NULL;
    }

    node->data.struct_decl.members = members;
    node->data.struct_decl.count = count;

    return node;
}