#include <stdlib.h>
#include <string.h>
#include "simplevector.h"
#include "lexer.h"
#include "tokens.h"
#include "ast.h"

#define bool unsigned char
/*
Let's play as the parser for a second:

class Thing {
    int a;
    int b;
    
    Thing(int a, int b){
        this.a=a;
        this.b=b;
    }
}

int main(){
    Thing a = Thing();
}
main();

Example:
int x = if(1==2) {3};


*/

// Expr* buildExpr(Vector* tokenStream, int* i){

// }

// FunctionDeclaration* buildFunctionDeclaration(Vector* tokenStream, int* i){
//     FunctionDeclaration* func = (FunctionDeclaration*) malloc(sizeof(FunctionDeclaration));
//     if(((Token*)vector_get(tokenStream,*i))->type==TYPE_NAME) {
        
//     }
// }

// IfExpr* buildIfExpr(Vector* tokenStream, int* i){
//     IfExpr* ifExpr = (IfExpr*) malloc(sizeof(IfExpr));
//     *i++;
//     Token* currentToken = (Token*)vector_get(tokenStream,*i);
//     if(currentToken->type!=GROUPING || currentToken->contents[0] != '('){
//         printf("Invalid syntax! Expected \'(\' after \'if\' on line %d column %d\n");
//         return NULL;
//     }
//     *i++;
//     if((ifExpr->conditions = buildExpr(tokenStream,*i))==NULL){
//         return NULL;
//     }

// }
int main(int argc, char** argv){
    const char* basicTypes[] = {
        "int",
        "float",
        "string",
        "char",
        "bool"
    };
    Vector* typeRegistry = vector_from(sizeof(char*),5,basicTypes);
    FILE* fptr = fopen(argv[1],"r");
    Queue* tokens = queue_make(lexer_lexFile(fptr));
    
    Token* curToken;
    while((curToken = (Token*)queue_consume(tokens))->type!= FILE_END){
        token_print(curToken);
    }
}