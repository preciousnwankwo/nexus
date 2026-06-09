#ifndef NEXUS_STRING_TABLE_H
#define NEXUS_STRING_TABLE_H

#include <stddef.h>
#include "arena.h"

#define STRING_TABLE_INITIAL_CAP 256

typedef struct {
    const char *str;
    size_t len;
    uint32_t hash;
} InternedString;

typedef struct {
    InternedString *entries;
    size_t count;
    size_t capacity;
    Arena *arena;
} StringTable;

void string_table_init(StringTable *table, Arena *arena);
void string_table_free(StringTable *table);
const char *string_table_intern(StringTable *table, const char *str, size_t len);
const char *string_table_intern_cstr(StringTable *table, const char *str);
int string_table_lookup(StringTable *table, const char *str, size_t len, const char **out);

uint32_t fnv1a_hash(const char *data, size_t len);

#endif
