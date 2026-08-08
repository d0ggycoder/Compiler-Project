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

// Global Variables
Allocator* nodeAllocator;

// Utility functions
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
    exit(2);
}

bool matchToken(Queue* q, TokenType t){
    if(((Token*)queue_peek(q))->type != t){
        return 0;
    }
    queue_consume(q);
    return 1;
}

bool isEnd(Queue* q){
    return ((Token*)queue_peek(q))->type == TOK_FILE_END;
}

AstValueNode* newNode(AstValueKind k){
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
    PREC_TERN_R,
    PREC_TERN_L,
    PREC_TERM_L,
    PREC_TERM_R,
    PREC_FACTOR_L,
    PREC_FACTOR_R,
    PREC_PREFIX,
    PREC_POSTFIX,
    PREC_CALL,
    PREC_PRIMARY
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
            lit = newNode(ast_integer_lit);
            lit->integer_lit.value = t->intVal;
            break;
        case TOK_DOUBLE_LIT:
            lit = newNode(ast_double_lit);
            lit->double_lit.value = t->doubleVal;
            break;
        case TOK_STRING_LIT:
            lit = newNode(ast_string_lit);
            lit->string_lit.value = cpystr(t->contents);
        case TOK_CHAR_LIT:
            lit = newNode(ast_char_lit);
            lit->char_lit.value = t->charVal;
        default:
            return NULL;
    }
}

AstValueNode* makeIdentifierNode(Queue* q){
    AstValueNode* node = newNode(ast_variable);
    node->variable.id = cpystr(((Token*)queue_consume(q))->contents);
    return node;
}

AstValueNode* newBinaryNode(OperatorKind op, AstValueNode* lhs, AstValueNode* rhs){
    AstValueNode* node = newNode(ast_binary_op);
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

AstValueNode* makeParensExpr(Queue* q){
    queue_consume(q); // Consume open parens
    AstValueNode* expr = parseExpression(q,PREC_NONE);
    matchToken(q,TOK_CLOSE_PARENS);
    return expr;
}

Vector* parseParams(Queue* q){
    queue_consume(q); // Consume open parens
    Vector* vec = vector_new(sizeof(AstValueNode));
    if(matchToken(q,TOK_CLOSE_PARENS)) return vec;
    while(!isEnd(q)){
        vector_append(vec,parseExpression(q,0.0));
        if(matchToken(q,TOK_CLOSE_PARENS)) break;
        if(!matchToken(q,TOK_COMMA)) panic_f(); 
    }
    return vec;
}

AstValueNode* makeFuncCall(Queue* q, AstValueNode* callee){
    AstValueNode* call = newNode(ast_function_call);
    call->function_call.callee = callee;
    call->function_call.parameters = parseParams(q);
    return call;
}

AstValueNode* makeTernaryExpr(Queue* q, AstValueNode* lhs){
    AstValueNode* tern = newNode(ast_cond_expr);
    tern->cond_expr.condition = lhs;
    matchToken(q,TOK_QUESTION); // Consume ?
    tern->cond_expr.then = parseExpression(q,PREC_TERN_R);
    matchToken(q,TOK_COLON);
    tern->cond_expr.otherwise = parseExpression(q,PREC_TERN_R);
    return tern;
}

ParseRule parseRules[] = {
    [TOK_IDENTIFIER] = {makeIdentifierNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_INT_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_DOUBLE_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_CHAR_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_STRING_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},

    [TOK_QUESTION] = {NULL, makeTernaryExpr, PREC_TERN_L, PREC_TERN_R},

    [TOK_PLUS] = {NULL, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},
    [TOK_MINUS] = {NULL, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},

    [TOK_STAR] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},
    [TOK_STAR] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},

    [TOK_OPEN_PARENS] = {makeParensExpr, makeFuncCall, PREC_PRIMARY, PREC_CALL},
    [TOK_CLOSE_PARENS] = {NULL, NULL, PREC_NONE, PREC_NONE},
    [TOK_COMMA] = {NULL, NULL, PREC_NONE, PREC_NONE},
    [TOK_TERMINATOR] = {NULL, NULL, PREC_NONE, PREC_NONE},
};


AstValueNode* parseExpression(Queue* q, int minbp){
    Token* t = (Token*)queue_peek(q);
    PrefixFn prefix = parseRules[t->type].prefix;
    if(prefix==NULL) {
        panic_f();
    }

    AstValueNode* left = prefix(q);

    while(minbp < parseRules[((Token*)queue_peek(q))->type].lbp){
        Token* token = (Token*)queue_peek(q);

        InfixFn infix = parseRules[token->type].infix;
        left = infix(q, left);
    }
    return left;
}

// Ast prints
void printSpaces(int indent){
    while(indent--){
        printf("  ");
    }
}

const char* opStr[]={
    [BINARY_ADDITION]="+",
    [BINARY_SUBTRACTION]="-",
    [BINARY_MULTIPLICATION]="*",
    [BINARY_DIVISION]="/",
    [ASSIGNMENT]="="
};

void printAst(AstValueNode* ast, int depth){
    switch(ast->kind){
        case ast_integer_lit:
            printSpaces(depth);
            printf("Int lit: %d\n",ast->integer_lit.value);
            break;
        case ast_double_lit:
            printSpaces(depth);
            printf("Double lit: %f\n",ast->double_lit.value);
            break;
        case ast_char_lit:
            printSpaces(depth);
            printf("Char lit: %c\n",ast->char_lit.value);
            break;
        case ast_string_lit:
            printSpaces(depth);
            printf("String lit: %s\n",ast->string_lit.value);
            break;
        case ast_variable:
            printSpaces(depth);
            printf("Variable: %s\n",ast->variable.id);
            break;
        case ast_binary_op:
            printSpaces(depth);
            printf("Binary Operation\n");
            printSpaces(depth);
            printf("| Operand: %s\n",opStr[ast->binary_op.opkind]);
            printSpaces(depth);
            printf("| Left: \n");
            printAst(ast->binary_op.left,depth+1);
            printSpaces(depth);
            printf("| Right: \n");
            printAst(ast->binary_op.right,depth+1);
            break;
        case ast_function_call:
            printSpaces(depth);
            printf("Function Call\n");
            printSpaces(depth);
            printf("| Callee: \n");
            printAst(ast->function_call.callee,depth+1);
            printSpaces(depth);
            printf("| Parameters: \n");
            for(int i=0;i<vector_size(ast->function_call.parameters);i++){
                printAst((AstValueNode*)vector_get(ast->function_call.parameters,i),depth+1);
            }
            break;
        case ast_cond_expr:
            printSpaces(depth);
            printf("Conditional Expression\n");
            printSpaces(depth);
            printf("| Condition: \n");
            printAst(ast->cond_expr.condition,depth+1);
            printSpaces(depth);
            printf("| Then: \n");
            printAst(ast->cond_expr.then,depth+1);
            printSpaces(depth);
            printf("| Else: \n");
            printAst(ast->cond_expr.otherwise,depth+1);
    }
}

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
    printf("Parsing successful! :D\n");
    printAst(expr,0);
    return 0;
}