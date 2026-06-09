#include "ast.h"
#include <stdlib.h>

AstNode *ast_new(NodeKind kind, Arena *arena) {
    AstNode *node = arena_alloc_zero(arena, sizeof(AstNode));
    node->kind = kind;
    return node;
}

AstNode *ast_new_lit(NodeKind kind, const char *value, size_t len, Arena *arena) {
    AstNode *node = ast_new(kind, arena);
    switch (kind) {
        case NODE_INT_LIT:
            node->data.int_lit.value = value;
            node->data.int_lit.len = len;
            break;
        case NODE_FLOAT_LIT:
            node->data.float_lit.value = value;
            node->data.float_lit.len = len;
            break;
        case NODE_STRING_LIT:
            node->data.string_lit.value = value;
            node->data.string_lit.len = len;
            break;
        case NODE_CHAR_LIT:
            node->data.char_lit.value = value[0];
            break;
        default:
            break;
    }
    return node;
}

AstNode *ast_new_identifier(const char *name, Arena *arena) {
    AstNode *node = ast_new(NODE_IDENTIFIER, arena);
    node->data.identifier.name = name;
    return node;
}

AstNode *ast_new_binary(TokenKind op, AstNode *left, AstNode *right, Arena *arena) {
    AstNode *node = ast_new(NODE_BINARY_EXPR, arena);
    node->data.binary.left = left;
    node->data.binary.op = op;
    node->data.binary.right = right;
    return node;
}

AstNode *ast_new_unary(TokenKind op, AstNode *operand, Arena *arena) {
    AstNode *node = ast_new(NODE_UNARY_EXPR, arena);
    node->data.unary.op = op;
    node->data.unary.operand = operand;
    return node;
}
