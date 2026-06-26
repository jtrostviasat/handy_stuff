/*
 * handy_str.h - non-owning string slice (pointer + length).
 *
 * No null-termination required. Cheap to pass by value. Pairs naturally
 * with handy_strbuilder.h for building, and handy_arena.h for owning storage.
 *
 *   hs_str hello = HS_STR_LIT("hello, world");
 *   if (hs_str_starts_with(hello, HS_STR_LIT("hello"))) { ... }
 */
#ifndef HANDY_STR_H
#define HANDY_STR_H

#include <stddef.h>

#include "handy/handy_config.h"

HANDY_BEGIN_DECLS

typedef struct {
    const char *data;
    size_t len;
} hs_str;

#define HS_STR_LIT(s) ((hs_str){ (s), sizeof(s) - 1 })
#define HS_STR_NULL   ((hs_str){ NULL, 0 })

static inline hs_str hs_str_from_cstr(const char *cstr) {
    hs_str s; s.data = cstr; s.len = 0;
    if (cstr != NULL) {
        while (cstr[s.len] != '\0') { s.len++; }
    }
    return s;
}

static inline hs_str hs_str_from_bytes(const char *data, size_t len) {
    hs_str s; s.data = data; s.len = len; return s;
}

static inline unsigned char hs_str_is_empty(hs_str s) {
    return (unsigned char)(s.len == 0);
}

static inline unsigned char hs_str_eq(hs_str a, hs_str b) {
    if (a.len != b.len) { return 0; }
    for (size_t i = 0; i < a.len; ++i) {
        if (a.data[i] != b.data[i]) { return 0; }
    }
    return 1;
}

static inline unsigned char hs_str_starts_with(hs_str s, hs_str prefix) {
    if (prefix.len > s.len) { return 0; }
    for (size_t i = 0; i < prefix.len; ++i) {
        if (s.data[i] != prefix.data[i]) { return 0; }
    }
    return 1;
}

static inline unsigned char hs_str_ends_with(hs_str s, hs_str suffix) {
    if (suffix.len > s.len) { return 0; }
    size_t off = s.len - suffix.len;
    for (size_t i = 0; i < suffix.len; ++i) {
        if (s.data[off + i] != suffix.data[i]) { return 0; }
    }
    return 1;
}

/* Returns SIZE_MAX when not found. */
static inline size_t hs_str_find_char(hs_str s, char c) {
    for (size_t i = 0; i < s.len; ++i) {
        if (s.data[i] == c) { return i; }
    }
    return (size_t)-1;
}

static inline hs_str hs_str_slice(hs_str s, size_t start, size_t end) {
    if (start > s.len) { start = s.len; }
    if (end > s.len)   { end = s.len; }
    if (end < start)   { end = start; }
    hs_str r; r.data = s.data + start; r.len = end - start; return r;
}

static inline hs_str hs_str_trim_left(hs_str s) {
    size_t i = 0;
    while (i < s.len && (s.data[i] == ' ' || s.data[i] == '\t' ||
                         s.data[i] == '\n' || s.data[i] == '\r')) { i++; }
    return hs_str_slice(s, i, s.len);
}

static inline hs_str hs_str_trim_right(hs_str s) {
    size_t end = s.len;
    while (end > 0) {
        char c = s.data[end - 1];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') { break; }
        end--;
    }
    return hs_str_slice(s, 0, end);
}

static inline hs_str hs_str_trim(hs_str s) {
    return hs_str_trim_left(hs_str_trim_right(s));
}

/* Splits at first occurrence of sep. Returns the part before sep in `head`
 * and the part after in `tail`. If sep is absent, head=s, tail=empty, returns 0. */
static inline unsigned char hs_str_split_char(hs_str s, char sep, hs_str *head, hs_str *tail) {
    size_t idx = hs_str_find_char(s, sep);
    if (idx == (size_t)-1) {
        if (head) *head = s;
        if (tail) *tail = HS_STR_NULL;
        return 0;
    }
    if (head) *head = hs_str_slice(s, 0, idx);
    if (tail) *tail = hs_str_slice(s, idx + 1, s.len);
    return 1;
}

HANDY_END_DECLS

#endif
