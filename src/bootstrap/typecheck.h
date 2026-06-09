#ifndef NEXUS_TYPECHECK_H
#define NEXUS_TYPECHECK_H

#include "ast.h"
#include "symbol_table.h"
#include "arena.h"
#include "string_table.h"

typedef struct {
    Arena *arena;
    StringTable *strings;
    SymbolTable *symbols;
    int had_error;
} TypeChecker;

void typecheck_init(TypeChecker *tc, Arena *arena, StringTable *strings, SymbolTable *symbols);
int typecheck_program(TypeChecker *tc, AstNode *program);

#endif
