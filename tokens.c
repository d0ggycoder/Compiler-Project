#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"

const char* tokenTypeStrings[] = {
    "STRING_LIT",
    "INT_LIT",
    "DOUBLE_LIT",
    "IDENTIFIER",
    "TYPE_NAME",
    "TERMINATOR",
    "KEYWORD",
    "OPERATOR"
};

void token_print(Token* token){
    printf("%s: \"%s\"\n",tokenTypeStrings[token->type],token->contents);
}

void token_free(void* _token){
    Token* token = (Token*) _token;
    free(token->contents);
    free(token);
}

Token* token_make(TokenType type, char* contents){
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->contents = contents;
    return token;
}