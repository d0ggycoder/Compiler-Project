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

char* cpystr(char* s){
    char* c = allocator_alloc(nodeAllocator,strlen(s));
    strcpy(c,s);
    return c;
}

bool isType(){
    return 1;
}

void panic_f(){
    exit(EXIT_FAILURE);
}

AstValueNode* makeNode(AstValueKind k){
    AstValueNode* n = allocator_alloc(nodeAllocator,sizeof(AstValueNode));
    n->kind = k;
}

// Parse rules definitions
AstValueNode* parseExpression(Queue* q, int minbp);

typedef AstValueNode* (*PrefixFn)(Queue*);
typedef AstValueNode* (*InfixFn)(Queue*, AstValueNode*);

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_TERM_L,
    PREC_TERM_R,
    PREC_FACTOR_L,
    PREC_FACTOR_R
} BindingPower;

typedef struct {
    PrefixFn prefix;
    InfixFn infix;
    BindingPower rbp;
    BindingPower lbp;
} ParseRule;


ParseRule parseRules[];
OperatorKind tokToInfixOp(TokenType t){
    switch(t){
        case TOK_PLUS: return BINARY_ADDITION;
        case TOK_MINUS: return BINARY_SUBTRACTION;
        case TOK_STAR: return BINARY_MULTIPLICATION;
        case TOK_SLASH: return BINARY_DIVISION;
        default: return -1;
    }
}
// Value construction
AstValueNode* makeLitNode(Queue* q){
    Token* t = (Token*) queue_consume(q);
    AstValueNode* lit;
    switch(t->type){
        case TOK_INT_LIT:
            lit = makeNode(ast_integer_lit);
            lit->integer_lit.value = t->intVal;
            break;
        case TOK_DOUBLE_LIT:
            lit = makeNode(ast_double_lit);
            lit->double_lit.value = t->doubleVal;
            break;
        case TOK_STRING_LIT:
            lit = makeNode(ast_string_lit);
            lit->string_lit.value = cpystr(t->contents);
        case TOK_CHAR_LIT:
            lit = makeNode(ast_char_lit);
            lit->char_lit.value = t->charVal;
        default:
            return NULL;
    }
}

AstValueNode* newBinaryNode(OperatorKind op, AstValueNode* lhs, AstValueNode* rhs){
    AstValueNode* node = makeNode(ast_binary_op);
    node->binary_op.opkind = op;
    node->binary_op.left = lhs;
    node->binary_op.right = rhs;
    return node;
}

AstValueNode* makeBinaryNode(Queue* q, AstValueNode* lhs){
    Token* op = (Token*)queue_consume(q);
    int prec = parseRules[op->type].rbp;

    AstValueNode* rhs = parseExpression(q, prec);
    return newBinaryNode(tokToInfixOp(op->type), lhs, rhs);
}

ParseRule parseRules[] = {
    [TOK_INT_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_DOUBLE_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_CHAR_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_STRING_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},

    [TOK_PLUS] = {NULL, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},
    [TOK_MINUS] = {NULL, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},

    [TOK_STAR] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},
    [TOK_STAR] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R}
};


AstValueNode* parseExpression(Queue* q, int minbp){
    Token* t = (Token*)queue_peek(q);
    PrefixFn prefix = parseRules[t->type].prefix;
    if(prefix==NULL) panic_f();

    AstValueNode* left = prefix(q);

    while(minbp < parseRules[((Token*)queue_peek(q))->type].lbp){
        Token* token = (Token*)queue_peek(q);

        InfixFn infix = parseRules[token->type].infix;
        left = infix(q, left);
    }
    return left;
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
    
    // Token* curToken;
    // while((curToken = (Token*)queue_consume(tokens))->type!= TOK_FILE_END){
    //     token_print(curToken);
    // }
    AstValueNode* expr = parseExpression(tokens,0);
    
    return 0;
}