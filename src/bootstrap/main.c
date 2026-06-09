#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"
#include "string_table.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol_table.h"
#include "typecheck.h"
#include "codegen.h"

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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: nexus-bootstrap <source.nx> <output.c>\n");
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

    SymbolTable symbols;
    symbol_table_init(&symbols, &arena);

    TypeChecker tc;
    typecheck_init(&tc, &arena, &strings, &symbols);

    if (typecheck_program(&tc, program)) {
        fprintf(stderr, "nexus-bootstrap: type errors in %s\n", argv[1]);
        free(source);
        arena_free(&arena);
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "error: cannot open '%s' for writing\n", argv[2]);
        free(source);
        arena_free(&arena);
        return 1;
    }

    CodeGen cg;
    codegen_init(&cg, out, &arena, &strings);

    if (codegen_program(&cg, program)) {
        fprintf(stderr, "nexus-bootstrap: codegen errors in %s\n", argv[1]);
        fclose(out);
        free(source);
        arena_free(&arena);
        return 1;
    }

    fclose(out);
    printf("nexus-bootstrap: %s -> %s\n", argv[1], argv[2]);

    free(source);
    arena_free(&arena);
    return 0;
}
