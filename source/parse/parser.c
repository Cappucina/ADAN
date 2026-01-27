#include "parser.h"

#include "../../include/ast.h"
#include "../lex/lexer.h"

extern int yyparse(void);
extern ASTNode* root;
extern Lexer* current_lexer;

ASTNode* parse(const char* source, ErrorList* errors, const char* file)
{
    Lexer* lexer = create_lexer(source, errors, file);
    if (!lexer)
        return NULL;

    current_lexer = lexer;
    int parse_result = yyparse();
    current_lexer = NULL;
    free_lexer(lexer);

    if (parse_result != 0)
        return NULL;

    return root;
}