#include <stdlib.h>
#include <string.h>
#include "simplevector.h"
#include "simplearenaallocator.h"
#include "simplequeue.h"
#include "lexer.h"
#include "tokens.h"
#include "ast.h"

#define bool unsigned char
#define PRIMITIVE_COUNT 5
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
Allocator* nodeAllocator;

int scmp(void* a, void* b){
    return !strcmp(*(char**) a, (char*) b);
}

bool isType(){
    return 1;
}

int getPrecedence(char c, bool isLHS){
    
}

AstValueNode* parseExpression(Queue* tokenQueue, int min_bp){
    Token* t = (Token*)queue_consume(tokenQueue);

}

// AstValueNode* parseExpressionNud(Token* token){
//     if(token->type == INT_LIT){
//         AstValueNode* node = (AstValueNode*) allocator_alloc(nodeAllocator, sizeof(AstValueNode));
//     }
// }

// /*
// Decl -> TYPE ID ('=' EXPR)?
// */
// AstStatementNode* makeDeclarationNode(Queue* tokenQueue){
//     AstStatementNode* anode = (AstStatementNode*)allocator_alloc(nodeAllocator,sizeof(AstStatementNode));
//     anode->kind = ast_declaration;
//     Token* curToken = (Token*)queue_ahead(tokenQueue,0);
//     if(curToken->type == IDENTIFIER && isType(curToken->contents)){
//         anode->variable_declaration.type = curToken->contents;
//     } else return NULL;
//     curToken = (Token*)queue_ahead(tokenQueue,1);
//     if(curToken->type == IDENTIFIER){
//         anode->variable_declaration.id = curToken->contents;
//     } else return NULL;
//     curToken = (Token*)queue_ahead(tokenQueue,2);

//     if(curToken->type != OPERATOR || curToken->contents[0] != '=') {
//         if(curToken->type != TERMINATOR) return NULL;
//         queue_consume(tokenQueue);
//         queue_consume(tokenQueue);
//         queue_consume(tokenQueue);
//         anode->variable_declaration.initValue=NULL;
//         return anode;
//     }
//     queue_consume(tokenQueue);
//     queue_consume(tokenQueue);
//     queue_consume(tokenQueue);
    
//     anode->variable_declaration.initValue = parseExpression(tokenQueue, 0);

//     return NULL;
// }

int main(int argc, char** argv){
    const char* basicTypes[PRIMITIVE_COUNT] = {
        "int",
        "float",
        "string",
        "char",
        "bool"
    };
    Vector* typeRegistry = vector_from(sizeof(char*),PRIMITIVE_COUNT,basicTypes);

    Queue* tokens = queue_new(lexer_lexFile(argv[1]));
    nodeAllocator = allocator_new(128);
    
    Token* curToken;
    while((curToken = (Token*)queue_consume(tokens))->type!= TOK_FILE_END){
        token_print(curToken);
    }

    return 0;
}