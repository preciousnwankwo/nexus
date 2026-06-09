#ifndef NEXUS_RUNTIME_H
#define NEXUS_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} NexusString;

typedef struct {
    void *data;
    size_t len;
    size_t capacity;
    size_t elem_size;
} NexusSlice;

typedef struct {
    int tag;
    union {
        void *ok;
        void *err;
    } data;
} NexusResult;

NexusString nexus_string_new(const char *str);
NexusString nexus_string_from_raw(char *str, size_t len);
void nexus_string_free(NexusString *s);
int nexus_string_eq(NexusString a, NexusString b);
void nexus_print(NexusString s);
void nexus_println(NexusString s);
void nexus_eprintln(NexusString s);
void nexus_panic(NexusString msg);

NexusSlice nexus_slice_new(size_t elem_size, size_t capacity);
void nexus_slice_free(NexusSlice *s);
void nexus_slice_push(NexusSlice *s, void *elem);
void *nexus_slice_get(NexusSlice *s, size_t index);

#endif
