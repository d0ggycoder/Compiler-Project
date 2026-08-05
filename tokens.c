#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"

void token_print(Token* token){
    switch(token->type){
        case TOK_INT_LIT:
            printf("INT LIT: %d\n",token->intVal);
            break;
        case TOK_DOUBLE_LIT:
            printf("DOUBLE LIT: %f\n",token->doubleVal);
            break;
        case TOK_CHAR_LIT:
            printf("CHAR LIT: %c\n",token->charVal);
            break;
        case TOK_STRING_LIT:
            printf("STRING LIT: %s\n",token->contents);
            break;
        case TOK_IDENTIFIER:
            printf("IDENTIFIER: %s\n",token->contents);
            break;
        case TOK_TERMINATOR:
            printf("TERMINATOR\n");
            break;
        case TOK_FILE_END:
            printf("EOF\n");
            break;
        default:
            printf("OPERATOR: opcode %d\n",token->type);
    }
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

Token* token_new(TokenType type){
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    return token;
}