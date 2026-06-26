/*
 * handy_arena.h - linear (bump) allocator.
 *
 * Arenas turn "lots of small malloc/free pairs" into "one big buffer,
 * reset when done." They are the single biggest ergonomics win in C
 * memory management.
 *
 * Two flavors:
 *   - User-backed:   hs_arena_init(&a, buffer, capacity)
 *   - Heap-backed:   hs_arena_init_heap(&a, capacity)  (requires HANDY_HAVE_MALLOC)
 *
 * Usage:
 *   hs_arena arena;
 *   char backing[64 * 1024];
 *   hs_arena_init(&arena, backing, sizeof backing);
 *   MyThing *t = HS_ARENA_NEW(&arena, MyThing);
 *   int *arr   = HS_ARENA_NEW_ARRAY(&arena, int, 128);
 *   hs_arena_reset(&arena);   // free everything in O(1)
 *
 * Save/restore lets you roll back to a checkpoint:
 *   size_t mark = hs_arena_mark(&arena);
 *   ... allocate scratch ...
 *   hs_arena_restore(&arena, mark);
 */
#ifndef HANDY_ARENA_H
#define HANDY_ARENA_H

#include <stddef.h>
#include <stdint.h>

#include "handy/handy_config.h"
#include "handy/handy_safety.h"

#ifndef HANDY_HAVE_MALLOC
#define HANDY_HAVE_MALLOC 1
#endif

#if HANDY_HAVE_MALLOC
#include <stdlib.h>
#endif

HANDY_BEGIN_DECLS

typedef struct {
    unsigned char *base;
    size_t capacity;
    size_t offset;
    unsigned char owns_backing;
} hs_arena;

static inline void hs_arena_init(hs_arena *arena, void *backing, size_t capacity) {
    arena->base = (unsigned char *)backing;
    arena->capacity = capacity;
    arena->offset = 0;
    arena->owns_backing = 0;
}

#if HANDY_HAVE_MALLOC
static inline HS_NODISCARD hs_status hs_arena_init_heap(hs_arena *arena, size_t capacity) {
    void *mem = malloc(capacity);
    if (mem == NULL) {
        return HS_ERR_ARG;
    }
    hs_arena_init(arena, mem, capacity);
    arena->owns_backing = 1;
    return HS_OK;
}

static inline void hs_arena_destroy(hs_arena *arena) {
    if (arena->owns_backing) {
        free(arena->base);
    }
    arena->base = NULL;
    arena->capacity = 0;
    arena->offset = 0;
    arena->owns_backing = 0;
}
#endif

static inline size_t hs_arena_align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

static inline void *hs_arena_alloc_aligned(hs_arena *arena, size_t bytes, size_t alignment) {
    size_t aligned_offset = hs_arena_align_up(arena->offset, alignment);
    if (aligned_offset > arena->capacity || bytes > arena->capacity - aligned_offset) {
        return NULL;
    }
    void *result = arena->base + aligned_offset;
    arena->offset = aligned_offset + bytes;
    return result;
}

static inline void *hs_arena_alloc(hs_arena *arena, size_t bytes) {
    return hs_arena_alloc_aligned(arena, bytes, sizeof(void *));
}

static inline void hs_arena_reset(hs_arena *arena) {
    arena->offset = 0;
}

static inline size_t hs_arena_mark(const hs_arena *arena) {
    return arena->offset;
}

static inline void hs_arena_restore(hs_arena *arena, size_t mark) {
    HS_ASSERT(mark <= arena->offset, "arena mark must not move forward");
    arena->offset = mark;
}

static inline size_t hs_arena_used(const hs_arena *arena) {
    return arena->offset;
}

static inline size_t hs_arena_remaining(const hs_arena *arena) {
    return arena->capacity - arena->offset;
}

#define HS_ARENA_NEW(arena, T) \
    ((T *)hs_arena_alloc_aligned((arena), sizeof(T), sizeof(void *)))

#define HS_ARENA_NEW_ARRAY(arena, T, count) \
    ((T *)hs_arena_alloc_aligned((arena), sizeof(T) * (count), sizeof(void *)))

HANDY_END_DECLS

#endif
