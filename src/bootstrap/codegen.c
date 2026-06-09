#include "codegen.h"
#include <stdio.h>
#include <string.h>

static void codegen_indent(CodeGen *cg) {
    for (int i = 0; i < cg->indent; i++) {
        fprintf(cg->output, "    ");
    }
}

static const char *codegen_type(AstNode *type_node) {
    if (!type_node) return "int";

    switch (type_node->kind) {
        case NODE_TYPE_PATH:
            if (strcmp(type_node->data.type_path.name, "i32") == 0) return "int";
            if (strcmp(type_node->data.type_path.name, "i64") == 0) return "long";
            if (strcmp(type_node->data.type_path.name, "u32") == 0) return "unsigned int";
            if (strcmp(type_node->data.type_path.name, "u64") == 0) return "unsigned long";
            if (strcmp(type_node->data.type_path.name, "f32") == 0) return "float";
            if (strcmp(type_node->data.type_path.name, "f64") == 0) return "double";
            if (strcmp(type_node->data.type_path.name, "bool") == 0) return "int";
            if (strcmp(type_node->data.type_path.name, "string") == 0) return "const char*";
            if (strcmp(type_node->data.type_path.name, "char") == 0) return "char";
            if (strcmp(type_node->data.type_path.name, "nil") == 0) return "int";
            return type_node->data.type_path.name;
        case NODE_TYPE_PTR:
            return "void*";
        case NODE_TYPE_REF:
            return "const void*";
        case NODE_TYPE_SLICE:
            return "void*";
        case NODE_TYPE_ARRAY:
            return "void*";
        case NODE_TYPE_FN:
            return "void*";
        case NODE_TYPE_TUPLE:
            return "void*";
        default:
            return "void";
    }
}

static void codegen_expression(CodeGen *cg, AstNode *node);
static void codegen_statement(CodeGen *cg, AstNode *node);

static void codegen_block(CodeGen *cg, AstNode *block) {
    if (!block || block->kind != NODE_BLOCK) return;

    fprintf(cg->output, "{\n");
    cg->indent++;

    for (size_t i = 0; i < block->data.block.item_count; i++) {
        codegen_statement(cg, block->data.block.items[i]);
    }

    cg->indent--;
    codegen_indent(cg);
    fprintf(cg->output, "}");
}

static void codegen_statement_no_indent(CodeGen *cg, AstNode *node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_FN_DECL:
            fprintf(cg->output, "%s %s(", codegen_type(node->data.fn_decl.return_type), node->data.fn_decl.name);
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                if (i > 0) fprintf(cg->output, ", ");
                AstNode *param = node->data.fn_decl.params[i];
                fprintf(cg->output, "%s %s", codegen_type(param->data.param.type), param->data.param.name);
            }
            fprintf(cg->output, ") ");
            codegen_block(cg, node->data.fn_decl.body);
            fprintf(cg->output, "\n\n");
            break;
        case NODE_STRUCT_DECL:
            fprintf(cg->output, "typedef struct {\n");
            cg->indent++;
            for (size_t i = 0; i < node->data.struct_decl.field_count; i++) {
                AstNode *field = node->data.struct_decl.fields[i];
                codegen_indent(cg);
                fprintf(cg->output, "%s %s;\n", codegen_type(field->data.field.type), field->data.field.name);
            }
            cg->indent--;
            codegen_indent(cg);
            fprintf(cg->output, "} %s;\n\n", node->data.struct_decl.name);
            break;
        case NODE_ENUM_DECL:
            fprintf(cg->output, "typedef enum {\n");
            cg->indent++;
            for (size_t i = 0; i < node->data.enum_decl.variant_count; i++) {
                AstNode *variant = node->data.enum_decl.variants[i];
                codegen_indent(cg);
                fprintf(cg->output, "%s_%s", node->data.enum_decl.name, variant->data.variant.name);
                if (i < node->data.enum_decl.variant_count - 1) {
                    fprintf(cg->output, ",");
                }
                fprintf(cg->output, "\n");
            }
            cg->indent--;
            codegen_indent(cg);
            fprintf(cg->output, "} %s;\n\n", node->data.enum_decl.name);
            break;
        default:
            codegen_statement(cg, node);
            break;
    }
}

static void codegen_expression(CodeGen *cg, AstNode *node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_INT_LIT:
            fprintf(cg->output, "%.*s", (int)node->data.int_lit.len, node->data.int_lit.value);
            break;
        case NODE_FLOAT_LIT:
            fprintf(cg->output, "%.*s", (int)node->data.float_lit.len, node->data.float_lit.value);
            break;
        case NODE_STRING_LIT:
            fprintf(cg->output, "\"%.*s\"", (int)node->data.string_lit.len, node->data.string_lit.value);
            break;
        case NODE_CHAR_LIT:
            fprintf(cg->output, "'%c'", node->data.char_lit.value);
            break;
        case NODE_BOOL_LIT:
            fprintf(cg->output, "%s", node->data.bool_lit.value ? "1" : "0");
            break;
        case NODE_NIL_LIT:
            fprintf(cg->output, "NULL");
            break;
        case NODE_IDENTIFIER:
            fprintf(cg->output, "%s", node->data.identifier.name);
            break;
        case NODE_BINARY_EXPR:
            fprintf(cg->output, "(");
            codegen_expression(cg, node->data.binary.left);
            fprintf(cg->output, " %s ", token_kind_name(node->data.binary.op));
            codegen_expression(cg, node->data.binary.right);
            fprintf(cg->output, ")");
            break;
        case NODE_UNARY_EXPR:
            fprintf(cg->output, "%s", token_kind_name(node->data.unary.op));
            codegen_expression(cg, node->data.unary.operand);
            break;
        case NODE_CALL_EXPR:
            codegen_expression(cg, node->data.call.callee);
            fprintf(cg->output, "(");
            for (size_t i = 0; i < node->data.call.arg_count; i++) {
                if (i > 0) fprintf(cg->output, ", ");
                codegen_expression(cg, node->data.call.args[i]);
            }
            fprintf(cg->output, ")");
            break;
        case NODE_INDEX_EXPR:
            codegen_expression(cg, node->data.index_expr.collection);
            fprintf(cg->output, "[");
            codegen_expression(cg, node->data.index_expr.index);
            fprintf(cg->output, "]");
            break;
        case NODE_FIELD_ACCESS:
            codegen_expression(cg, node->data.field_access.object);
            fprintf(cg->output, ".%s", node->data.field_access.field);
            break;
        case NODE_IF_EXPR:
            fprintf(cg->output, "if (");
            codegen_expression(cg, node->data.if_expr.cond);
            fprintf(cg->output, ") ");
            codegen_block(cg, node->data.if_expr.then_block);
            if (node->data.if_expr.else_block) {
                fprintf(cg->output, " else ");
                if (node->data.if_expr.else_block->kind == NODE_IF_EXPR) {
                    codegen_expression(cg, node->data.if_expr.else_block);
                } else {
                    codegen_block(cg, node->data.if_expr.else_block);
                }
            }
            break;
        default:
            fprintf(cg->output, "/* TODO: %d */", node->kind);
            break;
    }
}

static void codegen_statement(CodeGen *cg, AstNode *node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_LET_STMT:
            codegen_indent(cg);
            fprintf(cg->output, "%s %s", codegen_type(node->data.let_stmt.type), node->data.let_stmt.name);
            if (node->data.let_stmt.init) {
                fprintf(cg->output, " = ");
                codegen_expression(cg, node->data.let_stmt.init);
            }
            fprintf(cg->output, ";\n");
            break;
        case NODE_RETURN_STMT:
            codegen_indent(cg);
            fprintf(cg->output, "return");
            if (node->data.return_stmt.value) {
                fprintf(cg->output, " ");
                codegen_expression(cg, node->data.return_stmt.value);
            }
            fprintf(cg->output, ";\n");
            break;
        case NODE_FN_DECL:
            codegen_indent(cg);
            fprintf(cg->output, "%s %s(", codegen_type(node->data.fn_decl.return_type), node->data.fn_decl.name);
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                if (i > 0) fprintf(cg->output, ", ");
                AstNode *param = node->data.fn_decl.params[i];
                fprintf(cg->output, "%s %s", codegen_type(param->data.param.type), param->data.param.name);
            }
            fprintf(cg->output, ") ");
            codegen_block(cg, node->data.fn_decl.body);
            fprintf(cg->output, "\n\n");
            break;
        case NODE_STRUCT_DECL:
            codegen_indent(cg);
            fprintf(cg->output, "typedef struct {\n");
            cg->indent++;
            for (size_t i = 0; i < node->data.struct_decl.field_count; i++) {
                AstNode *field = node->data.struct_decl.fields[i];
                codegen_indent(cg);
                fprintf(cg->output, "%s %s;\n", codegen_type(field->data.field.type), field->data.field.name);
            }
            cg->indent--;
            codegen_indent(cg);
            fprintf(cg->output, "} %s;\n\n", node->data.struct_decl.name);
            break;
        case NODE_ENUM_DECL:
            codegen_indent(cg);
            fprintf(cg->output, "typedef enum {\n");
            cg->indent++;
            for (size_t i = 0; i < node->data.enum_decl.variant_count; i++) {
                AstNode *variant = node->data.enum_decl.variants[i];
                codegen_indent(cg);
                fprintf(cg->output, "%s_%s", node->data.enum_decl.name, variant->data.variant.name);
                if (i < node->data.enum_decl.variant_count - 1) {
                    fprintf(cg->output, ",");
                }
                fprintf(cg->output, "\n");
            }
            cg->indent--;
            codegen_indent(cg);
            fprintf(cg->output, "} %s;\n\n", node->data.enum_decl.name);
            break;
        case NODE_PACKET_DECL:
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                codegen_statement(cg, node->data.fn_decl.params[i]);
            }
            break;
        case NODE_IMPORT_DECL:
            break;
        case NODE_EXPR_STMT:
            codegen_indent(cg);
            codegen_expression(cg, node->data.expr_stmt.expr);
            fprintf(cg->output, ";\n");
            break;
        default:
            break;
    }
}

void codegen_init(CodeGen *cg, FILE *output, Arena *arena, StringTable *strings) {
    cg->output = output;
    cg->arena = arena;
    cg->strings = strings;
    cg->indent = 0;
    cg->had_error = 0;
}

int codegen_program(CodeGen *cg, AstNode *program) {
    if (!program || program->kind != NODE_PROGRAM) return 1;

    fprintf(cg->output, "/* Generated by nexus-bootstrap */\n");
    fprintf(cg->output, "#include <stdio.h>\n");
    fprintf(cg->output, "#include <stdlib.h>\n");
    fprintf(cg->output, "#include <string.h>\n\n");

    for (size_t i = 0; i < program->data.program.stmt_count; i++) {
        codegen_statement_no_indent(cg, program->data.program.stmts[i]);
    }

    return cg->had_error ? 1 : 0;
}
