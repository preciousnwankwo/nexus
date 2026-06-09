#include "string_table.h"
#include <stdlib.h>
#include <string.h>

uint32_t fnv1a_hash(const char *data, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)data[i];
        hash *= 16777619u;
    }
    return hash;
}

void string_table_init(StringTable *table, Arena *arena) {
    table->arena = arena;
    table->capacity = STRING_TABLE_INITIAL_CAP;
    table->count = 0;
    table->entries = arena_alloc_zero(arena, sizeof(InternedString) * table->capacity);
}

void string_table_free(StringTable *table) {
    (void)table;
}

static int string_table_find_slot(StringTable *table, const char *str, size_t len, uint32_t hash) {
    size_t idx = hash & (table->capacity - 1);
    for (size_t i = 0; i < table->capacity; i++) {
        InternedString *e = &table->entries[idx];
        if (e->str == NULL) return (int)idx;
        if (e->hash == hash && e->len == len && memcmp(e->str, str, len) == 0) return (int)idx;
        idx = (idx + 1) & (table->capacity - 1);
    }
    return -1;
}

static void string_table_grow(StringTable *table) {
    size_t old_cap = table->capacity;
    InternedString *old = table->entries;

    table->capacity *= 2;
    table->entries = arena_alloc_zero(table->arena, sizeof(InternedString) * table->capacity);
    table->count = 0;

    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].str) {
            int slot = string_table_find_slot(table, old[i].str, old[i].len, old[i].hash);
            table->entries[slot] = old[i];
            table->count++;
        }
    }
}

const char *string_table_intern(StringTable *table, const char *str, size_t len) {
    uint32_t hash = fnv1a_hash(str, len);
    int slot = string_table_find_slot(table, str, len, hash);
    if (slot < 0) return NULL;

    if (table->entries[slot].str) {
        return table->entries[slot].str;
    }

    if (table->count + 1 > table->capacity * 3 / 4) {
        string_table_grow(table);
        slot = string_table_find_slot(table, str, len, hash);
    }

    char *dup = arena_strndup(table->arena, str, len);
    table->entries[slot].str = dup;
    table->entries[slot].len = len;
    table->entries[slot].hash = hash;
    table->count++;
    return dup;
}

const char *string_table_intern_cstr(StringTable *table, const char *str) {
    return string_table_intern(table, str, strlen(str));
}

int string_table_lookup(StringTable *table, const char *str, size_t len, const char **out) {
    uint32_t hash = fnv1a_hash(str, len);
    int slot = string_table_find_slot(table, str, len, hash);
    if (slot < 0 || !table->entries[slot].str) return 0;
    *out = table->entries[slot].str;
    return 1;
}
