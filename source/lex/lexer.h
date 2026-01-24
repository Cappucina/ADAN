enum Tokens
{
    // Special
    TOKEN_EOF,
    TOKEN_IDENTIFIER,

    // Keywords
    TOKEN_PROGRAM,
    TOKEN_INCLUDE,
    TOKEN_STRUCT,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    // Types
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_BOOL_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_VOID_LITERAL,

    // Symbols
    TOKEN_OPEN_PAREN, // (
    TOKEN_CLOSE_PAREN, // )
    TOKEN_OPEN_BRACE, // {
    TOKEN_CLOSE_BRACE, // }
    TOKEN_SEMICOLON, // ;
    TOKEN_EQUAL, // = (Used for ASSIGNMENT, not comparison!)
    TOKEN_EQUALITY, // ==
    TOKEN_NOT, // !
    TOKEN_NOT_EQUALS, // !==
    TOKEN_TYPE_DECL, // ::
    TOKEN_MUL, // *
    TOKEN_DIV, // /
    TOKEN_SUB, // -
    TOKEN_ADD, // +
    TOKEN_MOD, // %
    // TOKEN_POINTER, // * (Maybe?)
    TOKEN_AND, // && 
    TOKEN_OR, // ||
    TOKEN_MUL_EQUALS, // *=
    TOKEN_DIV_EQUALS, // /=
    TOKEN_SUB_EQUALS, // -=
    TOKEN_ADD_EQUALS, // +=
    TOKEN_MOD_EQUALS, // %=
    TOKEN_COMMA, // ,
    TOKEN_PERIOD, // .
    TOKEN_GREATER, // >
    TOKEN_LESS, // <
    TOKEN_GREATER_EQUALS, // >=
    TOKEN_LESS_EQUALS, // <=
    TOKEN_ADD_ADD, // ++
    TOKEN_SUB_SUB, // --
    TOKEN_OPEN_BRACKET, // [
    TOKEN_CLOSE_BRACKET, // ]
};