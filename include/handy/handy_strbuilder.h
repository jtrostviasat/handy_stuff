/*
 * handy_strbuilder.h - growable string buffer backed by a handy_arena.
 *
 * Why arena-backed? It sidesteps the "who owns this, when do I free it"
 * problem entirely. Build a string into an arena, hand out an hs_str, free
 * everything at once when the arena resets.
 *
 *   hs_arena a; hs_arena_init(&a, buf, sizeof buf);
 *   hs_strbuilder sb; hs_sb_init(&sb, &a);
 *   hs_sb_append_cstr(&sb, "user=");
 *   hs_sb_append_u64(&sb, user_id);
 *   hs_str msg = hs_sb_to_str(&sb);
 */
#ifndef HANDY_STRBUILDER_H
#define HANDY_STRBUILDER_H

#include <stddef.h>
#include <stdint.h>

#include "handy/handy_arena.h"
#include "handy/handy_config.h"
#include "handy/handy_safety.h"
#include "handy/handy_str.h"

HANDY_BEGIN_DECLS

typedef struct {
    hs_arena *arena;
    char *data;
    size_t len;
    size_t capacity;
} hs_strbuilder;

static inline void hs_sb_init(hs_strbuilder *sb, hs_arena *arena) {
    sb->arena = arena;
    sb->data = NULL;
    sb->len = 0;
    sb->capacity = 0;
}

static inline HS_NODISCARD hs_status hs_sb_reserve(hs_strbuilder *sb, size_t needed) {
    if (needed <= sb->capacity) { return HS_OK; }
    size_t new_cap = (sb->capacity == 0) ? 64 : sb->capacity;
    while (new_cap < needed) { new_cap *= 2; }
    /* Bump-allocate fresh region and copy. Old region remains live in arena
     * (no individual free), which is intentional. */
    char *new_data = (char *)hs_arena_alloc_aligned(sb->arena, new_cap, 1);
    if (new_data == NULL) { return HS_ERR_CAP; }
    for (size_t i = 0; i < sb->len; ++i) { new_data[i] = sb->data[i]; }
    sb->data = new_data;
    sb->capacity = new_cap;
    return HS_OK;
}

static inline HS_NODISCARD hs_status hs_sb_append_bytes(hs_strbuilder *sb,
                                                        const char *bytes, size_t n) {
    hs_status s = hs_sb_reserve(sb, sb->len + n + 1);
    if (s != HS_OK) { return s; }
    for (size_t i = 0; i < n; ++i) { sb->data[sb->len + i] = bytes[i]; }
    sb->len += n;
    sb->data[sb->len] = '\0';
    return HS_OK;
}

static inline HS_NODISCARD hs_status hs_sb_append_cstr(hs_strbuilder *sb, const char *cstr) {
    size_t n = 0;
    if (cstr != NULL) { while (cstr[n] != '\0') { n++; } }
    return hs_sb_append_bytes(sb, cstr, n);
}

static inline HS_NODISCARD hs_status hs_sb_append_str(hs_strbuilder *sb, hs_str s) {
    return hs_sb_append_bytes(sb, s.data, s.len);
}

static inline HS_NODISCARD hs_status hs_sb_append_char(hs_strbuilder *sb, char c) {
    return hs_sb_append_bytes(sb, &c, 1);
}

static inline HS_NODISCARD hs_status hs_sb_append_u64(hs_strbuilder *sb, uint64_t v) {
    char tmp[24]; size_t i = 0;
    if (v == 0) { tmp[i++] = '0'; }
    while (v != 0) { tmp[i++] = (char)('0' + (v % 10u)); v /= 10u; }
    /* reverse */
    for (size_t a = 0, b = i; a < b; ++a) { b--; char t = tmp[a]; tmp[a] = tmp[b]; tmp[b] = t; }
    return hs_sb_append_bytes(sb, tmp, i);
}

static inline HS_NODISCARD hs_status hs_sb_append_i64(hs_strbuilder *sb, int64_t v) {
    if (v < 0) {
        hs_status s = hs_sb_append_char(sb, '-');
        if (s != HS_OK) { return s; }
        return hs_sb_append_u64(sb, (uint64_t)(-(v + 1)) + 1u);
    }
    return hs_sb_append_u64(sb, (uint64_t)v);
}

static inline hs_str hs_sb_to_str(const hs_strbuilder *sb) {
    hs_str r; r.data = sb->data; r.len = sb->len; return r;
}

static inline const char *hs_sb_cstr(const hs_strbuilder *sb) {
    return sb->data != NULL ? sb->data : "";
}

HANDY_END_DECLS

#endif
