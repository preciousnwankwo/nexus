#ifndef NEXUS_SYMBOL_TABLE_H
#define NEXUS_SYMBOL_TABLE_H

#include <stddef.h>
#include "arena.h"
#include "ast.h"

typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_STRUCT,
    SYM_ENUM,
    SYM_MODULE,
    SYM_TYPE,
} SymbolKind;

typedef struct {
    const char *name;
    SymbolKind kind;
    AstNode *type_node;
    AstNode *decl_node;
    int is_mut;
    int is_initialized;
} Symbol;

typedef struct Scope {
    struct Scope *parent;
    Symbol *symbols;
    size_t count;
    size_t capacity;
} Scope;

typedef struct {
    Arena *arena;
    Scope *current_scope;
    int had_error;
} SymbolTable;

void symbol_table_init(SymbolTable *st, Arena *arena);
Scope *scope_push(SymbolTable *st);
void scope_pop(SymbolTable *st);
int symbol_define(SymbolTable *st, const char *name, SymbolKind kind, AstNode *type_node, AstNode *decl_node, int is_mut);
Symbol *symbol_lookup(SymbolTable *st, const char *name);
Symbol *symbol_lookup_current(SymbolTable *st, const char *name);

#endif
