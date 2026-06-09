#ifndef NEXUS_CODEGEN_H
#define NEXUS_CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "arena.h"
#include "string_table.h"

typedef struct {
    FILE *output;
    Arena *arena;
    StringTable *strings;
    int indent;
    int had_error;
} CodeGen;

void codegen_init(CodeGen *cg, FILE *output, Arena *arena, StringTable *strings);
int codegen_program(CodeGen *cg, AstNode *program);

#endif
