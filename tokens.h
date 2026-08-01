#ifndef _tokenspecs
#define _tokenspecs

typedef enum {
    COMPILER_NORMAL,
    COMPILER_TYPEDECL
} CompilerState;

typedef enum {
    STRING_LIT,
    INT_LIT,
    DOUBLE_LIT,
    IDENTIFIER,
    TYPE_NAME,
    TERMINATOR,
    KEYWORD,
    OPERATOR,
} TokenType;

typedef struct{
    TokenType type;
    char* contents;
} Token;

void token_print(Token* token);
void token_free(void* token);
Token* token_make(TokenType type, char* contents);


#endif