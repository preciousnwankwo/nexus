#include "symbol_table.h"
#include <stdio.h>
#include <string.h>

void symbol_table_init(SymbolTable *st, Arena *arena) {
    st->arena = arena;
    st->had_error = 0;
    st->current_scope = NULL;
    scope_push(st);
}

Scope *scope_push(SymbolTable *st) {
    Scope *scope = arena_alloc_zero(st->arena, sizeof(Scope));
    scope->parent = st->current_scope;
    scope->capacity = 16;
    scope->symbols = arena_alloc_zero(st->arena, sizeof(Symbol) * scope->capacity);
    scope->count = 0;
    st->current_scope = scope;
    return scope;
}

void scope_pop(SymbolTable *st) {
    if (st->current_scope) {
        st->current_scope = st->current_scope->parent;
    }
}

int symbol_define(SymbolTable *st, const char *name, SymbolKind kind, AstNode *type_node, AstNode *decl_node, int is_mut) {
    Scope *scope = st->current_scope;
    if (!scope) return 0;

    for (size_t i = 0; i < scope->count; i++) {
        if (scope->symbols[i].name && strcmp(scope->symbols[i].name, name) == 0) {
            fprintf(stderr, "error: '%s' already defined in this scope\n", name);
            st->had_error = 1;
            return 0;
        }
    }

    if (scope->count >= scope->capacity) {
        scope->capacity *= 2;
        Symbol *new_buf = arena_alloc_zero(st->arena, sizeof(Symbol) * scope->capacity);
        memcpy(new_buf, scope->symbols, sizeof(Symbol) * scope->count);
        scope->symbols = new_buf;
    }

    Symbol *sym = &scope->symbols[scope->count++];
    sym->name = name;
    sym->kind = kind;
    sym->type_node = type_node;
    sym->decl_node = decl_node;
    sym->is_mut = is_mut;
    sym->is_initialized = 0;
    return 1;
}

Symbol *symbol_lookup_current(SymbolTable *st, const char *name) {
    Scope *scope = st->current_scope;
    while (scope) {
        for (size_t i = 0; i < scope->count; i++) {
            if (scope->symbols[i].name && strcmp(scope->symbols[i].name, name) == 0) {
                return &scope->symbols[i];
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

Symbol *symbol_lookup(SymbolTable *st, const char *name) {
    return symbol_lookup_current(st, name);
}
