#ifndef NEXUS_ARENA_H
#define NEXUS_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_BLOCK_SIZE 8192

typedef struct ArenaBlock {
    struct ArenaBlock *next;
    size_t capacity;
    size_t used;
    uint8_t data[];
} ArenaBlock;

typedef struct {
    ArenaBlock *blocks;
    ArenaBlock *current;
    size_t total_allocated;
} Arena;

void arena_init(Arena *arena);
void arena_free(Arena *arena);
void *arena_alloc(Arena *arena, size_t size);
void *arena_alloc_zero(Arena *arena, size_t size);
char *arena_strdup(Arena *arena, const char *str);
char *arena_strndup(Arena *arena, const char *str, size_t len);

#endif
