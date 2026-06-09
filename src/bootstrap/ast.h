#ifndef NEXUS_AST_H
#define NEXUS_AST_H

#include "lexer.h"
#include "arena.h"

typedef enum {
    NODE_PROGRAM,
    NODE_FN_DECL,
    NODE_STRUCT_DECL,
    NODE_ENUM_DECL,
    NODE_IMPORT_DECL,
    NODE_PACKET_DECL,

    NODE_BLOCK,
    NODE_LET_STMT,
    NODE_RETURN_STMT,
    NODE_EXPR_STMT,

    NODE_IF_EXPR,
    NODE_MATCH_EXPR,
    NODE_MATCH_ARM,

    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_INDEX_EXPR,
    NODE_FIELD_ACCESS,
    NODE_CAST_EXPR,

    NODE_IDENTIFIER,
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_STRING_LIT,
    NODE_CHAR_LIT,
    NODE_BOOL_LIT,
    NODE_NIL_LIT,
    NODE_ARRAY_LIT,
    NODE_TUPLE_LIT,

    NODE_TYPE_PATH,
    NODE_TYPE_PTR,
    NODE_TYPE_REF,
    NODE_TYPE_ARRAY,
    NODE_TYPE_SLICE,
    NODE_TYPE_FN,
    NODE_TYPE_TUPLE,

    NODE_PARAM,
    NODE_FIELD,
    NODE_VARIANT,
    NODE_VARIANT_FIELD,
    NODE_ARG,

    NODE_AGENT_EXPR,
    NODE_PARALLEL_EXPR,
    NODE_AWAIT_EXPR,
    NODE_UNSAFE_EXPR,
    NODE_ARENA_EXPR,
} NodeKind;

typedef struct AstNode {
    NodeKind kind;
    SourceLocation loc;
    union {
        struct {
            struct AstNode **stmts;
            size_t stmt_count;
        } program;

        struct {
            const char *name;
            struct AstNode **params;
            size_t param_count;
            struct AstNode *return_type;
            struct AstNode *body;
        } fn_decl;

        struct {
            const char *name;
            struct AstNode **fields;
            size_t field_count;
        } struct_decl;

        struct {
            const char *name;
            struct AstNode **variants;
            size_t variant_count;
        } enum_decl;

        struct {
            struct AstNode **items;
            size_t item_count;
        } block;

        struct {
            const char *name;
            struct AstNode *type;
            struct AstNode *init;
            int is_mut;
        } let_stmt;

        struct {
            struct AstNode *value;
        } return_stmt;

        struct {
            struct AstNode *expr;
        } expr_stmt;

        struct {
            struct AstNode *cond;
            struct AstNode *then_block;
            struct AstNode *else_block;
        } if_expr;

        struct {
            struct AstNode *value;
            struct AstNode **arms;
            size_t arm_count;
        } match_expr;

        struct {
            struct AstNode *pattern;
            struct AstNode *body;
        } match_arm;

        struct {
            struct AstNode *left;
            TokenKind op;
            struct AstNode *right;
        } binary;

        struct {
            TokenKind op;
            struct AstNode *operand;
        } unary;

        struct {
            struct AstNode *callee;
            struct AstNode **args;
            size_t arg_count;
        } call;

        struct {
            struct AstNode *collection;
            struct AstNode *index;
        } index_expr;

        struct {
            struct AstNode *object;
            const char *field;
        } field_access;

        struct {
            struct AstNode *value;
            struct AstNode *target_type;
        } cast_expr;

        struct {
            const char *name;
        } identifier;

        struct {
            const char *value;
            size_t len;
        } int_lit;

        struct {
            const char *value;
            size_t len;
        } float_lit;

        struct {
            const char *value;
            size_t len;
        } string_lit;

        struct {
            char value;
        } char_lit;

        struct {
            int value;
        } bool_lit;

        struct {
            struct AstNode **elements;
            size_t element_count;
        } array_lit;

        struct {
            struct AstNode **elements;
            size_t element_count;
        } tuple_lit;

        struct {
            const char *name;
            struct AstNode **generics;
            size_t generic_count;
        } type_path;

        struct {
            struct AstNode *pointee;
        } type_ptr;

        struct {
            struct AstNode *referent;
        } type_ref;

        struct {
            struct AstNode *element_type;
            struct AstNode *size;
        } type_array;

        struct {
            struct AstNode *element_type;
        } type_slice;

        struct {
            struct AstNode **param_types;
            size_t param_count;
            struct AstNode *return_type;
        } type_fn;

        struct {
            struct AstNode **element_types;
            size_t element_count;
        } type_tuple;

        struct {
            const char *name;
            struct AstNode *type;
            int is_mut;
        } param;

        struct {
            const char *name;
            struct AstNode *type;
        } field;

        struct {
            const char *name;
            struct AstNode **fields;
            size_t field_count;
        } variant;

        struct {
            const char *name;
            struct AstNode *type;
        } variant_field;

        struct {
            struct AstNode *value;
        } arg;

        struct {
            struct AstNode *callee;
            struct AstNode **args;
            size_t arg_count;
        } agent_expr;

        struct {
            struct AstNode **blocks;
            size_t block_count;
        } parallel_expr;

        struct {
            struct AstNode *expr;
        } await_expr;

        struct {
            struct AstNode *expr;
        } unsafe_expr;

        struct {
            struct AstNode *expr;
        } arena_expr;
    } data;
} AstNode;

AstNode *ast_new(NodeKind kind, Arena *arena);
AstNode *ast_new_lit(NodeKind kind, const char *value, size_t len, Arena *arena);
AstNode *ast_new_identifier(const char *name, Arena *arena);
AstNode *ast_new_binary(TokenKind op, AstNode *left, AstNode *right, Arena *arena);
AstNode *ast_new_unary(TokenKind op, AstNode *operand, Arena *arena);

#endif
