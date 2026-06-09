#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NexusString nexus_string_new(const char *str) {
    NexusString s;
    s.len = strlen(str);
    s.capacity = s.len + 1;
    s.data = malloc(s.capacity);
    memcpy(s.data, str, s.len);
    s.data[s.len] = '\0';
    return s;
}

NexusString nexus_string_from_raw(char *str, size_t len) {
    NexusString s;
    s.len = len;
    s.capacity = len + 1;
    s.data = malloc(s.capacity);
    memcpy(s.data, str, len);
    s.data[len] = '\0';
    return s;
}

void nexus_string_free(NexusString *s) {
    if (s && s->data) {
        free(s->data);
        s->data = NULL;
        s->len = 0;
        s->capacity = 0;
    }
}

int nexus_string_eq(NexusString a, NexusString b) {
    if (a.len != b.len) return 0;
    return memcmp(a.data, b.data, a.len) == 0;
}

void nexus_print(NexusString s) {
    printf("%.*s", (int)s.len, s.data);
}

void nexus_println(NexusString s) {
    printf("%.*s\n", (int)s.len, s.data);
}

void nexus_eprintln(NexusString s) {
    fprintf(stderr, "%.*s\n", (int)s.len, s.data);
}

void nexus_panic(NexusString msg) {
    fprintf(stderr, "panic: %.*s\n", (int)msg.len, msg.data);
    exit(1);
}

void nexus_print_int(int n) {
    printf("%d", n);
}

void nexus_println_int(int n) {
    printf("%d\n", n);
}

void nexus_print_float(double f) {
    printf("%g", f);
}

void nexus_println_float(double f) {
    printf("%g\n", f);
}

NexusSlice nexus_slice_new(size_t elem_size, size_t capacity) {
    NexusSlice s;
    s.elem_size = elem_size;
    s.len = 0;
    s.capacity = capacity;
    s.data = malloc(elem_size * capacity);
    return s;
}

void nexus_slice_free(NexusSlice *s) {
    if (s && s->data) {
        free(s->data);
        s->data = NULL;
        s->len = 0;
        s->capacity = 0;
    }
}

void nexus_slice_push(NexusSlice *s, void *elem) {
    if (s->len >= s->capacity) {
        s->capacity *= 2;
        s->data = realloc(s->data, s->elem_size * s->capacity);
    }
    void *dest = (char *)s->data + (s->len * s->elem_size);
    memcpy(dest, elem, s->elem_size);
    s->len++;
}

void *nexus_slice_get(NexusSlice *s, size_t index) {
    if (index >= s->len) {
        fprintf(stderr, "index out of bounds: %zu >= %zu\n", index, s->len);
        exit(1);
    }
    return (char *)s->data + (index * s->elem_size);
}
