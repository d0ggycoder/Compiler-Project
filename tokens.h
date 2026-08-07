#ifndef _tokenspecs
#define _tokenspecs

typedef enum {
    COMPILER_NORMAL,
    COMPILER_TYPEDECL
} CompilerState;

typedef enum {
    TOK_FILE_END,
    TOK_IDENTIFIER,
    TOK_TERMINATOR,
    TOK_KEYWORD,

    // Literals
    TOK_STRING_LIT,
    TOK_INT_LIT,
    TOK_CHAR_LIT,
    TOK_DOUBLE_LIT,

    // Opeators

    // Simple assignment
    TOK_EQUAL,

    // Value comparisons
    TOK_EQUAL_EQUAL,
    TOK_BANG_EQUAL,
    TOK_LT_EQUAL,
    TOK_GT_EQUAL,
    TOK_GT,
    TOK_LT,
    TOK_AND_AND,
    TOK_PIPE_PIPE,
    
    // Simple arithmetic
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_MOD,

    // Bitwise
    TOK_GT_GT,
    TOK_LT_LT,
    TOK_CARAT,
    TOK_AND,
    TOK_PIPE,
    TOK_TILDE,

    // Compound assignments
    TOK_PLUS_EQUAL,
    TOK_MINUS_EQUAL,
    TOK_STAR_EQUAL,
    TOK_SLASH_EQUAL,
    TOK_MOD_EQUAL,
    TOK_GT_GT_EQUAL,
    TOK_LT_LT_EQUAL,
    TOK_CARAT_EQUAL,
    TOK_AND_EQUAL,
    TOK_PIPE_EQUAL,
    TOK_TILDE_EQUAL,
    TOK_PLUS_PLUS,
    TOK_MINUS_MINUS,

    // Accesses
    TOK_DOT,
    TOK_MINUS_GT, // Pointer member access
    TOK_DOT_DOT, // Cascade operator

    // Ternary
    TOK_QUESTION,
    TOK_COLON,
    
    // Groupings
    TOK_OPEN_PARENS,
    TOK_CLOSE_PARENS,
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_OPEN_BRACES,
    TOK_CLOSE_BRACES
} TokenType;

typedef struct{
    TokenType type;
    union{
        char* contents;
        int intVal;
        double doubleVal;
        char charVal;
    };
    // For error message purposes
    int row; 
    int col; 
} Token;

void token_print(Token* token);
void token_free(void* token);
Token* token_make(TokenType type, char* contents);
Token* token_new(TokenType type);

unsigned char token_isOp(TokenType t);
unsigned char token_isLit(TokenType t);

#endif