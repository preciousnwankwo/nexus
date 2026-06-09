#include "arena.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static ArenaBlock *arena_new_block(size_t min_size) {
    size_t cap = ARENA_BLOCK_SIZE;
    if (min_size > cap) cap = min_size;

    ArenaBlock *block = malloc(sizeof(ArenaBlock) + cap);
    if (!block) {
        fprintf(stderr, "arena: out of memory\n");
        exit(1);
    }
    block->next = NULL;
    block->capacity = cap;
    block->used = 0;
    return block;
}

void arena_init(Arena *arena) {
    arena->blocks = NULL;
    arena->current = NULL;
    arena->total_allocated = 0;
}

void arena_free(Arena *arena) {
    ArenaBlock *block = arena->blocks;
    while (block) {
        ArenaBlock *next = block->next;
        free(block);
        block = next;
    }
    arena->blocks = NULL;
    arena->current = NULL;
    arena->total_allocated = 0;
}

void *arena_alloc(Arena *arena, size_t size) {
    if (size == 0) return NULL;

    size_t align = 8;
    size = (size + align - 1) & ~(align - 1);

    if (!arena->current || arena->current->used + size > arena->current->capacity) {
        ArenaBlock *block = arena_new_block(size);
        if (arena->current) {
            arena->current->next = block;
        } else {
            arena->blocks = block;
        }
        arena->current = block;
    }

    void *ptr = arena->current->data + arena->current->used;
    arena->current->used += size;
    arena->total_allocated += size;
    return ptr;
}

void *arena_alloc_zero(Arena *arena, size_t size) {
    void *ptr = arena_alloc(arena, size);
    memset(ptr, 0, size);
    return ptr;
}

char *arena_strdup(Arena *arena, const char *str) {
    size_t len = strlen(str);
    char *dup = arena_alloc(arena, len + 1);
    memcpy(dup, str, len);
    dup[len] = '\0';
    return dup;
}

char *arena_strndup(Arena *arena, const char *str, size_t len) {
    char *dup = arena_alloc(arena, len + 1);
    memcpy(dup, str, len);
    dup[len] = '\0';
    return dup;
}
