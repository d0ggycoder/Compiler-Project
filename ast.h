#ifndef _ast
#define _ast
#include "simplevector.h"

typedef enum {
    BINARY_ADDITION,
    BINARY_SUBTRACTION,
    BINARY_MULTIPLICATION,
    BINARY_DIVISION,
    ASSIGNMENT
} OperatorKind;

typedef struct AstValueNode AstValueNode;

typedef enum {
    ast_integer_lit,
    ast_string_lit,
    ast_double_lit,
    ast_char_lit,
    ast_variable,
    ast_binary_op,
    ast_unary_op,
    ast_cond_expr,
    ast_block_expr,
    ast_assignment,
    ast_function_call
} AstValueKind;

struct AstValueNode{
    AstValueKind kind;
    union {
        struct {
            int value;
        } integer_lit;
        struct {
            char* value;
        } string_lit;
        struct {
            double value;
        } double_lit;
        struct {
            char value;
        } char_lit;
        struct {
            char* id;
        } variable;
        struct {
            AstValueNode* left;
            OperatorKind opkind;
            AstValueNode* right;
        } binary_op;
        struct {
            OperatorKind opkind;
            AstValueNode* operand;
        } unary_op;
        struct {
            Vector* statements;
            AstValueNode* result;
        } block_expr;
        struct {
            AstValueNode* condition;
            AstValueNode* then;
            AstValueNode* otherwise;
        } cond_expr;
        struct {
            AstValueNode* id;
            AstValueNode* value;
        } assignment;
        struct {
            AstValueNode* callee;
            Vector* parameters;
        } function_call;
    };
};

typedef struct AstStatementNode AstStatementNode;

typedef enum {
    ast_declaration
} AstStatementKind;

struct AstStatementNode{
    AstStatementKind kind;
    union {
        struct {
            char* type;
            char* id;
            AstValueNode* initValue;
        } variable_declaration;
    };
};

typedef struct AstNode AstNode;

typedef enum {
    ast_statement_node,
    ast_value_node
} AstNodeKind;

struct AstNode{
    AstNodeKind kind;
    union {
        AstStatementNode statement;
        AstValueNode value;
    };
};

#endif