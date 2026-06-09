#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"
#include "string_table.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s'\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static void print_ast(AstNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->kind) {
        case NODE_PROGRAM:
            printf("Program (%zu stmts)\n", node->data.program.stmt_count);
            for (size_t i = 0; i < node->data.program.stmt_count; i++) {
                print_ast(node->data.program.stmts[i], depth + 1);
            }
            break;
        case NODE_FN_DECL:
            printf("FnDecl(%s)\n", node->data.fn_decl.name);
            for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                print_ast(node->data.fn_decl.params[i], depth + 1);
            }
            print_ast(node->data.fn_decl.body, depth + 1);
            break;
        case NODE_STRUCT_DECL:
            printf("StructDecl(%s)\n", node->data.struct_decl.name);
            for (size_t i = 0; i < node->data.struct_decl.field_count; i++) {
                print_ast(node->data.struct_decl.fields[i], depth + 1);
            }
            break;
        case NODE_ENUM_DECL:
            printf("EnumDecl(%s)\n", node->data.enum_decl.name);
            break;
        case NODE_PACKET_DECL:
            printf("PacketDecl(%s)\n", node->data.fn_decl.name);
            break;
        case NODE_IMPORT_DECL:
            printf("ImportDecl\n");
            break;
        case NODE_BLOCK:
            printf("Block (%zu stmts)\n", node->data.block.item_count);
            for (size_t i = 0; i < node->data.block.item_count; i++) {
                print_ast(node->data.block.items[i], depth + 1);
            }
            break;
        case NODE_LET_STMT:
            printf("Let(%s)\n", node->data.let_stmt.name);
            break;
        case NODE_RETURN_STMT:
            printf("Return\n");
            break;
        case NODE_EXPR_STMT:
            printf("ExprStmt\n");
            break;
        case NODE_BINARY_EXPR:
            printf("Binary(%s)\n", token_kind_name(node->data.binary.op));
            break;
        case NODE_IDENTIFIER:
            printf("Ident(%s)\n", node->data.identifier.name);
            break;
        case NODE_INT_LIT:
            printf("IntLit(%.*s)\n", (int)node->data.int_lit.len, node->data.int_lit.value);
            break;
        case NODE_PARAM:
            printf("Param(%s)\n", node->data.param.name);
            break;
        default:
            printf("Node(%d)\n", node->kind);
            break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: nexus-bootstrap <source.nx>\n");
        return 1;
    }

    char *source = read_file(argv[1]);

    Arena arena;
    arena_init(&arena);

    StringTable strings;
    string_table_init(&strings, &arena);

    Lexer lex;
    lexer_init(&lex, source, strlen(source), &arena, &strings);

    Parser parser;
    parser_init(&parser, &lex, &arena, &strings);

    AstNode *program = parser_parse(&parser);

    if (parser.had_error) {
        fprintf(stderr, "nexus-bootstrap: parse errors in %s\n", argv[1]);
        free(source);
        arena_free(&arena);
        return 1;
    }

    printf("nexus-bootstrap: %s — parsed successfully\n", argv[1]);
    print_ast(program, 0);

    free(source);
    arena_free(&arena);
    return 0;
}
