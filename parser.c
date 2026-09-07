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
// TODO: wrap into a parser struct
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


typedef enum {
    PARSEERROR_ILLEGAL_ARGUMENT_FORMAT,
    PARSEERROR_ILLEGAL_PREFIX,
    PARSEERROR_ILLEGAL_DECLARATION_FORMAT,
    PARSEERROR_UNKNOWN_STATEMENT_FORMAT,
    PARSEERROR_ILLEGAL_DECLARATION_MISSING_TYPE,
    PARSEERROR_ILLEGAL_DECLARATION_MISSING_VALUE
} ParseError;
void panic_parse(Token* tok, int kind){
    switch (kind){
        case PARSEERROR_ILLEGAL_ARGUMENT_FORMAT:
            printf("Illegal token at Line: %d Col: %d. Expected \',\'\n",tok->row,tok->col);
            break;
        case PARSEERROR_ILLEGAL_PREFIX:
            printf("Illegal placement of operator at Line: %d Col %d\n",tok->row,tok->col);
            break;
        case PARSEERROR_ILLEGAL_DECLARATION_FORMAT:
            printf("Illegal variable declaration idk");
            break;
        case PARSEERROR_UNKNOWN_STATEMENT_FORMAT:
            printf("Unknown statement format at Line: %d Col: %d\n",tok->row,tok->col);
            break;
        case PARSEERROR_ILLEGAL_DECLARATION_MISSING_TYPE:
            printf("Missing type declaration at Line: %d Col: %d\n",tok->row,tok->col);
            break;
        case PARSEERROR_ILLEGAL_DECLARATION_MISSING_VALUE:
            printf("Missing initial value in declaration at Line: %d Col: %d\n",tok->row,tok->col);
            break;        
    }
    exit(1);
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

// This uses the C precedence heirarchy
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT_R,
    PREC_ASSIGNMENT_L,
    PREC_TERN_R,
    PREC_TERN_L,
    PREC_LOGICAL_OR_L,
    PREC_LOGICAL_OR_R,
    PREC_LOGICAL_AND_L,
    PREC_LOGICAL_AND_R,
    PREC_BITWISE_OR_L,
    PREC_BITWISE_OR_R,
    PREC_BITWISE_XOR_L,
    PREC_BITWISE_XOR_R,
    PREC_BITWISE_AND_L,
    PREC_BITWISE_AND_R,
    PREC_EQUALITY_L,
    PREC_EQUALITY_R,
    PREC_RELATIONAL_L,
    PREC_RELATIONAL_R,
    PREC_BITSHIFT_L,
    PREC_BITSHIFT_R,
    PREC_TERM_L,
    PREC_TERM_R,
    PREC_FACTOR_L,
    PREC_FACTOR_R,

    // Precedence 2 grouping (All exclusively prefix r->l, so only one value is provided)
    _PREC_2,
    PREC_PREFIX=_PREC_2,
    PREC_CAST=_PREC_2,
    PREC_DEREF=_PREC_2,
    PREC_ADDR=_PREC_2,

    // Precedence 1 grouping
    _PREC_1_L,
    _PREC_1_R,
    PREC_POSTFIX=_PREC_1_L,
    PREC_CALL=_PREC_1_L,
    PREC_ACCESS_L=_PREC_1_L,
    PREC_ACCESS_R=_PREC_1_R,
    PREC_INDEX_L=_PREC_1_L,
    PREC_INDEX_R=_PREC_1_R,

    // Precedence 0 grouping
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
        case TOK_MOD: return BINARY_MOD;
        case TOK_EQUAL: return ASSIGNMENT;

        case TOK_EQUAL_EQUAL: return COMPARE_EQ;
        case TOK_BANG_EQUAL: return COMPARE_NEQ;
        case TOK_LT_EQUAL: return COMPARE_LTE;
        case TOK_GT_EQUAL: return COMPARE_GTE;
        case TOK_GT: return COMPARE_GT;
        case TOK_LT: return COMPARE_LT;
        case TOK_AND_AND: return LOGICAL_AND;
        case TOK_PIPE_PIPE: return LOGICAL_OR;
        
        case TOK_GT_GT: return BITWISE_LSHIFT;
        case TOK_LT_LT: return BITWISE_RSHIFT;
        case TOK_CARAT: return BITWISE_XOR;
        case TOK_AND: return BITWISE_AND;
        case TOK_PIPE: return BITWISE_OR;

        // Compound assignments
        case TOK_PLUS_EQUAL: return COMPOUND_ADDITION;
        case TOK_MINUS_EQUAL: return COMPOUND_SUBTRACTION;
        case TOK_STAR_EQUAL: return COMPOUND_MULTIPLICATION;
        case TOK_SLASH_EQUAL: return COMPOUND_DIVISION;
        case TOK_MOD_EQUAL: return COMPOUND_MOD;
        case TOK_GT_GT_EQUAL: return COMPOUND_LSHIFT;
        case TOK_LT_LT_EQUAL: return COMPOUND_RSHIFT;
        case TOK_CARAT_EQUAL: return COMPOUND_XOR;
        case TOK_AND_EQUAL: return COMPOUND_AND;
        case TOK_PIPE_EQUAL: return COMPOUND_OR;

        // Accesses
        case TOK_DOT: return ACCESS_DOT;
        case TOK_MINUS_GT: return ACCESS_ARROW;
        case TOK_DOT_DOT: return ACCESS_CASCADE;
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
        if(!matchToken(q,TOK_COMMA)) panic_parse((Token*)queue_consume(q), 1); 
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

AstValueNode* makePrefix(Queue* q){
    AstValueNode* node = newNode(ast_unary_op);
    Token* token = (Token*)queue_consume(q);
    switch(token->type){
        case TOK_MINUS:
            node->unary_op.opkind=UNARY_NEGATION;
            break;
        case TOK_PLUS_PLUS:
            node->unary_op.opkind=PREFIX_INCREMENT;
            break;
        case TOK_MINUS_MINUS:
            node->unary_op.opkind=POSTFIX_INCREMENT;
            break;
        case TOK_TILDE:
            node->unary_op.opkind=BITWISE_NEGATION;
            break;
        case TOK_AND:
            node->unary_op.opkind=ACCESS_ADDRESS;
            break;
        case TOK_STAR:
            node->unary_op.opkind=ACCESS_DEREF;
            break;
    } 
    node->unary_op.operand = parseExpression(q,PREC_PREFIX);
    return node;
}

AstValueNode* makePostfix(Queue* q, AstValueNode* lhs){
    AstValueNode* node = newNode(ast_unary_op);
    Token* token = (Token*)queue_consume(q);
    switch(token->type){
        case TOK_PLUS_PLUS:
            node->unary_op.opkind=POSTFIX_INCREMENT;
            break;
        case TOK_MINUS_MINUS:
            node->unary_op.opkind=POSTFIX_DEINCREMENT;
            break;
    }
    node->unary_op.operand=lhs;
    return node;
}

ParseRule parseRules[] = {
    // Identifiers/literals
    [TOK_IDENTIFIER] = {makeIdentifierNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_INT_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_DOUBLE_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_CHAR_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},
    [TOK_STRING_LIT] = {makeLitNode, NULL, PREC_NONE, PREC_NONE},

    // Equality comparisons
    [TOK_EQUAL_EQUAL] = {NULL, makeBinaryNode, PREC_EQUALITY_L, PREC_EQUALITY_R},
    [TOK_BANG_EQUAL] = {NULL, makeBinaryNode, PREC_EQUALITY_L, PREC_EQUALITY_R},
    [TOK_LT_EQUAL] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},
    [TOK_GT_EQUAL] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},
    [TOK_GT] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},
    [TOK_LT] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},
    [TOK_AND_AND] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},
    [TOK_PIPE_PIPE] = {NULL, makeBinaryNode, PREC_RELATIONAL_L, PREC_RELATIONAL_R},

    // Logical Operators
    [TOK_PIPE_PIPE] = {NULL, makeBinaryNode, PREC_LOGICAL_OR_L,PREC_LOGICAL_OR_R},
    [TOK_AND_AND] = {NULL, makeBinaryNode, PREC_LOGICAL_AND_L, PREC_LOGICAL_AND_R},

    // Bitwise Operators
    [TOK_GT_GT] = {NULL, makeBinaryNode, PREC_BITSHIFT_L, PREC_BITSHIFT_R},
    [TOK_LT_LT] = {NULL, makeBinaryNode, PREC_BITSHIFT_L, PREC_BITSHIFT_R},
    [TOK_AND] = {makePrefix, makeBinaryNode, PREC_BITWISE_AND_L, PREC_BITWISE_AND_R},
    [TOK_PIPE] = {NULL, makeBinaryNode, PREC_BITWISE_OR_L, PREC_BITWISE_OR_R},
    [TOK_CARAT] = {NULL, makeBinaryNode, PREC_BITWISE_XOR_L, PREC_BITWISE_XOR_R},
    [TOK_TILDE] = {makePrefix, NULL, PREC_PREFIX, PREC_PREFIX},

    [TOK_QUESTION] = {NULL, makeTernaryExpr, PREC_TERN_L, PREC_TERN_R},

    // Simple Arithmetic
    [TOK_PLUS] = {NULL, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},
    [TOK_MINUS] = {makePrefix, makeBinaryNode, PREC_TERM_L, PREC_TERM_R},
    [TOK_STAR] = {makePrefix, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},
    [TOK_SLASH] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},
    [TOK_MOD] = {NULL, makeBinaryNode, PREC_FACTOR_L, PREC_FACTOR_R},

    // Increment/deincrement
    [TOK_PLUS_PLUS] = {makePrefix, makePostfix, PREC_PREFIX, PREC_POSTFIX},
    [TOK_MINUS_MINUS] = {makePrefix, makePostfix, PREC_PREFIX, PREC_POSTFIX},

    // Simple and Compound Assignments
    [TOK_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_PLUS_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_MINUS_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_STAR_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_SLASH_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_MOD_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_GT_GT_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_LT_LT_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_CARAT_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_AND_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_PIPE_EQUAL] = {NULL, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_PLUS_PLUS] = {makePrefix, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},
    [TOK_MINUS_MINUS] = {makePrefix, makeBinaryNode, PREC_ASSIGNMENT_L, PREC_ASSIGNMENT_R},

    // Access
    [TOK_DOT] = {NULL, makeBinaryNode, PREC_ACCESS_L, PREC_ACCESS_R},
    [TOK_MINUS_GT] = {NULL, makeBinaryNode, PREC_ACCESS_L, PREC_ACCESS_R}, // Pointer member access
    [TOK_DOT_DOT] = {NULL, makeBinaryNode, PREC_ACCESS_L, PREC_ACCESS_R}, // Cascade operator

    [TOK_OPEN_PARENS] = {makeParensExpr, makeFuncCall, PREC_PRIMARY, PREC_CALL},
    [TOK_CLOSE_PARENS] = {NULL, NULL, PREC_NONE, PREC_NONE},
    [TOK_COMMA] = {NULL, NULL, PREC_NONE, PREC_NONE},
    [TOK_TERMINATOR] = {NULL, NULL, PREC_NONE, PREC_NONE}, // TODO: add proper token consumption
};


AstValueNode* parseExpression(Queue* q, int minbp){
    Token* t = (Token*)queue_peek(q);
    PrefixFn prefix = parseRules[t->type].prefix;
    if(prefix==NULL) {
        panic_parse(t,PARSEERROR_ILLEGAL_PREFIX);
    }

    AstValueNode* left = prefix(q);

    while(minbp < parseRules[((Token*)queue_peek(q))->type].lbp){
        Token* token = (Token*)queue_peek(q);
        InfixFn infix = parseRules[token->type].infix;
        left = infix(q, left);
    }
    return left;
}

AstStatementNode* newStatementNode(AstStatementKind k){
    AstStatementNode* s = (AstStatementNode*)allocator_alloc(nodeAllocator,sizeof(AstStatementNode));
    s->kind=k;
    return s;
}

AstStatementNode* parseDeclaration(Queue* q){
    AstStatementNode* node = newStatementNode(ast_variable_declaration);
    node->variable_declaration.type = ((Token*)queue_consume(q))->contents;
    if(!matchToken(q,TOK_COLON)){
        panic_parse((Token*)queue_peek(q),PARSEERROR_ILLEGAL_DECLARATION_MISSING_TYPE);
    }
    node->variable_declaration.id = ((Token*)queue_consume(q))->contents;
    if(matchToken(q,TOK_EQUAL)){
        node->variable_declaration.initValue = parseExpression(q,0);
    } else {
        node->variable_declaration.initValue = NULL;
    }
    matchToken(q,TOK_TERMINATOR);
    return node;
}

AstStatementNode* parseDeclarationConst(Queue* q){
    AstStatementNode* node = newStatementNode(ast_const_declaration);
    node->variable_declaration.type = ((Token*)queue_consume(q))->contents;
    if(!matchToken(q,TOK_COLON)){
        panic_parse((Token*)queue_peek(q),PARSEERROR_ILLEGAL_DECLARATION_MISSING_TYPE);
    }
    node->variable_declaration.id = ((Token*)queue_consume(q))->contents;
    if(matchToken(q,TOK_EQUAL)){
        node->variable_declaration.initValue = parseExpression(q,0);
    } else {
        panic_parse((Token*)queue_peek(q),PARSEERROR_ILLEGAL_DECLARATION_MISSING_VALUE);
    }
    matchToken(q,TOK_TERMINATOR);
    return node;
}

AstStatementNode* parseExpressionStatement(Queue* q){
    AstStatementNode* node = newStatementNode(ast_expression_statment);
    node->expression_statement.expr=parseExpression(q,0);
    matchToken(q,TOK_TERMINATOR);
    return node;
}

// Statement parsing 
AstStatementNode* parseStatement(Queue* q){
    /*
    Declaration: 
    |   [declmod] VARNAME ":" TYPE_EXPR ("=" EXPR) ";"
    

    */
    if(matchToken(q,TOK_VAR)){
        return parseDeclaration(q);
    } else if(matchToken(q,TOK_CONST)){
        return parseDeclarationConst(q);
    }
    panic_parse((Token*)queue_peek(q),PARSEERROR_UNKNOWN_STATEMENT_FORMAT);
}

AstNode* nodeFromStatement(AstStatementNode* statement){
    AstNode* node = (AstNode*)allocator_alloc(nodeAllocator,sizeof(AstNode));
    node->kind=ast_statement_node;
    node->statement=statement;
    return node;
}


// Ast prints
void printSpaces(int indent){
    while(indent--){
        printf("  ");
    }
}

const char* opStr[]={
    [UNARY_NEGATION]="-",
    [PREFIX_DEINCREMENT]="-- (before)",
    [PREFIX_INCREMENT]="++ (before)",
    [POSTFIX_DEINCREMENT]="-- (after)",
    [POSTFIX_INCREMENT] = "++ (after)",
    [BINARY_ADDITION]="+",
    [BINARY_SUBTRACTION]="-",
    [BINARY_MULTIPLICATION]="*",
    [BINARY_DIVISION]="/",
    [ASSIGNMENT]="="
};

void printAstValue(AstValueNode* ast, int depth){
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
        case ast_unary_op:
            printSpaces(depth);
            printf("Unary Operation\n");
            printSpaces(depth);
            if(opStr[ast->unary_op.opkind] == NULL){
                printf("| Operation: opcode %d\n",ast->unary_op.opkind);
            } else {
                printf("| Operation: %s\n",opStr[ast->unary_op.opkind]);
            }
            printSpaces(depth);
            printf("| Operand: \n");
            printAstValue(ast->unary_op.operand,depth+1);
            break;
        case ast_binary_op:
            printSpaces(depth);
            printf("Binary Operation\n");
            printSpaces(depth);
            if(opStr[ast->binary_op.opkind] == NULL){
                printf("| Operation: opcode %d\n",ast->binary_op.opkind);
            } else {
                printf("| Operation: %s\n",opStr[ast->binary_op.opkind]);
            }
            printSpaces(depth);
            printf("| Left: \n");
            printAstValue(ast->binary_op.left,depth+1);
            printSpaces(depth);
            printf("| Right: \n");
            printAstValue(ast->binary_op.right,depth+1);
            break;
        case ast_function_call:
            printSpaces(depth);
            printf("Function Call\n");
            printSpaces(depth);
            printf("| Callee: \n");
            printAstValue(ast->function_call.callee,depth+1);
            printSpaces(depth);
            printf("| Parameters: \n");
            for(int i=0;i<vector_size(ast->function_call.parameters);i++){
                printAstValue((AstValueNode*)vector_get(ast->function_call.parameters,i),depth+1);
            }
            break;
        case ast_cond_expr:
            printSpaces(depth);
            printf("Conditional Expression\n");
            printSpaces(depth);
            printf("| Condition: \n");
            printAstValue(ast->cond_expr.condition,depth+1);
            printSpaces(depth);
            printf("| Then: \n");
            printAstValue(ast->cond_expr.then,depth+1);
            printSpaces(depth);
            printf("| Else: \n");
            printAstValue(ast->cond_expr.otherwise,depth+1);
            break;
    }
}

void printAstStatement(AstStatementNode* ast, int depth){
    switch(ast->kind){
        case ast_const_declaration:
        case ast_variable_declaration:
            printSpaces(depth);
            printf("Variable Declaration\n");
            printSpaces(depth);
            printf("| Type Name: %s\n",ast->variable_declaration.type);
            printSpaces(depth);
            printf("| Variable name: %s\n",ast->variable_declaration.id);
            if(ast->variable_declaration.initValue!=NULL){
                printSpaces(depth);
                printf("| Initial Value: \n");
                printAstValue(ast->variable_declaration.initValue,depth+1);
            }
            break;
        case ast_expression_statment:
            printSpaces(depth);
            printf("Expression Statement\n");
            printSpaces(depth);
            printAstValue(ast->expression_statement.expr,depth+1);
            break;
    }
}

void printAstNode(AstNode* ast, int depth){
    switch(ast->kind){
        case ast_value_node:
            printAstValue(ast->value,depth+1);
            break;
        case ast_statement_node:
            printAstStatement(ast->statement,depth+1);
            break;
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

    Queue* tokens = queue_new(lexer_lexFile(argv[1]));
    nodeAllocator = allocator_new(128);
    
    while(((Token*)queue_peek(tokens))->type!=TOK_FILE_END){
        AstNode* statement = nodeFromStatement(parseStatement(tokens));
        printAstNode(statement,0);
    }
    // Token* curToken;
    // while((curToken = (Token*)queue_consume(tokens))->type!= TOK_FILE_END){
    //     token_print(curToken);
    // }
    return 0;
}