#include "typecheck.h"
#include <stdio.h>
#include <string.h>

static void typecheck_error(TypeChecker *tc, AstNode *node, const char *msg) {
    fprintf(stderr, "%s:%d:%d: error: %s\n",
            node->loc.file ? node->loc.file : "<input>",
            node->loc.line, node->loc.col, msg);
    tc->had_error = 1;
}

static int types_match(AstNode *a, AstNode *b) {
    if (!a || !b) return 1;
    if (a->kind != b->kind) return 0;

    switch (a->kind) {
        case NODE_TYPE_PATH:
            return a->data.type_path.name && b->data.type_path.name &&
                   strcmp(a->data.type_path.name, b->data.type_path.name) == 0;
        case NODE_TYPE_PTR:
            return types_match(a->data.type_ptr.pointee, b->data.type_ptr.pointee);
        case NODE_TYPE_REF:
            return types_match(a->data.type_ref.referent, b->data.type_ref.referent);
        case NODE_TYPE_MUT_REF:
            return types_match(a->data.type_mut_ref.inner, b->data.type_mut_ref.inner);
        case NODE_TYPE_OWN:
            return types_match(a->data.type_own.inner, b->data.type_own.inner);
        default:
            return 1;
    }
}

static void typecheck_statement(TypeChecker *tc, AstNode *node);
static AstNode *typecheck_expression(TypeChecker *tc, AstNode *node);

static void typecheck_block(TypeChecker *tc, AstNode *block) {
    if (!block || block->kind != NODE_BLOCK) return;

    scope_push(tc->symbols);
    for (size_t i = 0; i < block->data.block.item_count; i++) {
        typecheck_statement(tc, block->data.block.items[i]);
    }
    scope_pop(tc->symbols);
}

static AstNode *typecheck_expression(TypeChecker *tc, AstNode *node) {
    if (!node) return NULL;

    AstNode *result = NULL;

    switch (node->kind) {
        case NODE_INT_LIT: {
            AstNode *type = arena_alloc_zero(tc->arena, sizeof(AstNode));
            type->kind = NODE_TYPE_PATH;
            type->data.type_path.name = "i32";
            result = type;
            break;
        }
        case NODE_FLOAT_LIT: {
            AstNode *type = arena_alloc_zero(tc->arena, sizeof(AstNode));
            type->kind = NODE_TYPE_PATH;
            type->data.type_path.name = "f64";
            result = type;
            break;
        }
        case NODE_STRING_LIT: {
            AstNode *type = arena_alloc_zero(tc->arena, sizeof(AstNode));
            type->kind = NODE_TYPE_PATH;
            type->data.type_path.name = "string";
            result = type;
            break;
        }
        case NODE_BOOL_LIT: {
            AstNode *type = arena_alloc_zero(tc->arena, sizeof(AstNode));
            type->kind = NODE_TYPE_PATH;
            type->data.type_path.name = "bool";
            result = type;
            break;
        }
        case NODE_NIL_LIT: {
            result = NULL;
            break;
        }
        case NODE_IDENTIFIER: {
            Symbol *sym = symbol_lookup(tc->symbols, node->data.identifier.name);
            if (!sym) {
                typecheck_error(tc, node, "undefined variable");
                result = NULL;
                break;
            }
            result = sym->type_node;
            break;
        }
        case NODE_BINARY_EXPR: {
            AstNode *left_type = typecheck_expression(tc, node->data.binary.left);
            AstNode *right_type = typecheck_expression(tc, node->data.binary.right);
            if (left_type && right_type && !types_match(left_type, right_type)) {
                typecheck_error(tc, node, "type mismatch in binary expression");
            }
            result = left_type;
            break;
        }
        case NODE_UNARY_EXPR: {
            result = typecheck_expression(tc, node->data.unary.operand);
            break;
        }
        case NODE_CALL_EXPR: {
            // Typecheck all arguments first
            for (size_t i = 0; i < node->data.call.arg_count; i++) {
                typecheck_expression(tc, node->data.call.args[i]);
            }
            if (node->data.call.callee->kind == NODE_IDENTIFIER) {
                Symbol *sym = symbol_lookup(tc->symbols, node->data.call.callee->data.identifier.name);
                if (!sym) {
                    typecheck_error(tc, node, "undefined function");
                    result = NULL;
                    break;
                }
                result = sym->type_node;
                break;
            }
            result = NULL;
            break;
        }
        case NODE_IF_EXPR: {
            typecheck_expression(tc, node->data.if_expr.cond);
            typecheck_block(tc, node->data.if_expr.then_block);
            if (node->data.if_expr.else_block) {
                if (node->data.if_expr.else_block->kind == NODE_BLOCK) {
                    typecheck_block(tc, node->data.if_expr.else_block);
                } else {
                    typecheck_expression(tc, node->data.if_expr.else_block);
                }
            }
            result = NULL;
            break;
        }
        case NODE_WHILE_EXPR: {
            typecheck_expression(tc, node->data.while_expr.cond);
            typecheck_block(tc, node->data.while_expr.body);
            result = NULL;
            break;
        }
        case NODE_FOR_EXPR: {
            AstNode *i32_type = arena_alloc_zero(tc->arena, sizeof(AstNode));
            i32_type->kind = NODE_TYPE_PATH;
            i32_type->data.type_path.name = "i32";
            scope_push(tc->symbols);
            symbol_define(tc->symbols, node->data.for_expr.var,
                         SYM_VARIABLE, i32_type, NULL, 0);
            typecheck_expression(tc, node->data.for_expr.iter);
            typecheck_block(tc, node->data.for_expr.body);
            scope_pop(tc->symbols);
            result = NULL;
            break;
        }
        case NODE_MATCH_EXPR: {
            typecheck_expression(tc, node->data.match_expr.value);
            for (size_t i = 0; i < node->data.match_expr.arm_count; i++) {
                AstNode *arm = node->data.match_expr.arms[i];
                scope_push(tc->symbols);
                typecheck_expression(tc, arm->data.match_arm.pattern);
                typecheck_expression(tc, arm->data.match_arm.body);
                scope_pop(tc->symbols);
            }
            result = NULL;
            break;
        }
        default: {
            result = NULL;
            break;
        }
    }

    node->type_node = result;
    return result;
}

static void typecheck_statement(TypeChecker *tc, AstNode *node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_LET_STMT: {
            AstNode *init_type = NULL;
            if (node->data.let_stmt.init) {
                init_type = typecheck_expression(tc, node->data.let_stmt.init);
            }

            AstNode *var_type = node->data.let_stmt.type;
            if (var_type && init_type && !types_match(var_type, init_type)) {
                typecheck_error(tc, node, "type mismatch in let statement");
            }

            if (!var_type) var_type = init_type;
            node->type_node = var_type;

            symbol_define(tc->symbols, node->data.let_stmt.name,
                         SYM_VARIABLE, var_type, node, node->data.let_stmt.is_mut);
            break;
        }
        case NODE_RETURN_STMT: {
            if (node->data.return_stmt.value) {
                typecheck_expression(tc, node->data.return_stmt.value);
            }
            break;
        }
        case NODE_FN_DECL: {
            Symbol *existing = symbol_lookup_current(tc->symbols, node->data.fn_decl.name);
            if (!existing) {
                symbol_define(tc->symbols, node->data.fn_decl.name,
                             SYM_FUNCTION, node->data.fn_decl.return_type, node, 0);
            }

            scope_push(tc->symbols);

            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                AstNode *param = node->data.fn_decl.params[i];
                symbol_define(tc->symbols, param->data.param.name,
                             SYM_VARIABLE, param->data.param.type, param, 0);
            }

            typecheck_block(tc, node->data.fn_decl.body);
            scope_pop(tc->symbols);
            break;
        }
        case NODE_STRUCT_DECL: {
            symbol_define(tc->symbols, node->data.struct_decl.name,
                         SYM_STRUCT, NULL, node, 0);
            break;
        }
        case NODE_ENUM_DECL: {
            symbol_define(tc->symbols, node->data.enum_decl.name,
                         SYM_ENUM, NULL, node, 0);
            break;
        }
        case NODE_PACKET_DECL: {
            scope_push(tc->symbols);
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                typecheck_statement(tc, node->data.fn_decl.params[i]);
            }
            scope_pop(tc->symbols);
            break;
        }
        case NODE_EXPR_STMT: {
            typecheck_expression(tc, node->data.expr_stmt.expr);
            break;
        }
        default:
            break;
    }
}

void typecheck_init(TypeChecker *tc, Arena *arena, StringTable *strings, SymbolTable *symbols) {
    tc->arena = arena;
    tc->strings = strings;
    tc->symbols = symbols;
    tc->had_error = 0;
}

static void typecheck_forward_declare(TypeChecker *tc, AstNode *program) {
    // Built-in functions
    AstNode *void_type = arena_alloc_zero(tc->arena, sizeof(AstNode));
    void_type->kind = NODE_TYPE_PATH;
    void_type->data.type_path.name = "void";

    symbol_define(tc->symbols, "println", SYM_FUNCTION, void_type, NULL, 0);
    symbol_define(tc->symbols, "print", SYM_FUNCTION, void_type, NULL, 0);
    symbol_define(tc->symbols, "eprintln", SYM_FUNCTION, void_type, NULL, 0);
    symbol_define(tc->symbols, "panic", SYM_FUNCTION, void_type, NULL, 0);

    for (size_t i = 0; i < program->data.program.stmt_count; i++) {
        AstNode *node = program->data.program.stmts[i];
        if (node->kind == NODE_FN_DECL) {
            symbol_define(tc->symbols, node->data.fn_decl.name,
                         SYM_FUNCTION, node->data.fn_decl.return_type, node, 0);
        }
    }
}

int typecheck_program(TypeChecker *tc, AstNode *program) {
    if (!program || program->kind != NODE_PROGRAM) return 0;

    typecheck_forward_declare(tc, program);

    for (size_t i = 0; i < program->data.program.stmt_count; i++) {
        typecheck_statement(tc, program->data.program.stmts[i]);
    }

    return tc->had_error ? 1 : 0;
}
