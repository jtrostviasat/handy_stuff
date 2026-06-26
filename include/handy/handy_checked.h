/*
 * handy_checked.h - overflow-checking integer arithmetic.
 *
 * Returns 1 if the operation overflows (and *out is unspecified), 0 on
 * success (and *out is set). On gcc/clang these compile to a single
 * instruction via __builtin_*_overflow; elsewhere we fall back to a
 * before-the-fact check.
 *
 *   size_t total;
 *   if (hs_mul_overflow_size(count, sizeof *p, &total)) { return HS_ERR_BOUNDS; }
 *   void *buf = malloc(total);
 */
#ifndef HANDY_CHECKED_H
#define HANDY_CHECKED_H

#include <stddef.h>
#include <stdint.h>

#include "handy/handy_config.h"

HANDY_BEGIN_DECLS

#if defined(__GNUC__) || defined(__clang__)
#define HS_HAS_BUILTIN_OVERFLOW 1
#else
#define HS_HAS_BUILTIN_OVERFLOW 0
#endif

#if HS_HAS_BUILTIN_OVERFLOW

#define HS_DEF_OVF(NAME, T) \
    static inline unsigned char hs_add_overflow_##NAME(T a, T b, T *out) { \
        return (unsigned char)__builtin_add_overflow(a, b, out); \
    } \
    static inline unsigned char hs_sub_overflow_##NAME(T a, T b, T *out) { \
        return (unsigned char)__builtin_sub_overflow(a, b, out); \
    } \
    static inline unsigned char hs_mul_overflow_##NAME(T a, T b, T *out) { \
        return (unsigned char)__builtin_mul_overflow(a, b, out); \
    }

#else

/* Portable fallbacks. Slower, but correct. */
#define HS_DEF_OVF_UNSIGNED(NAME, T, MAX) \
    static inline unsigned char hs_add_overflow_##NAME(T a, T b, T *out) { \
        if (a > (MAX) - b) { return 1; } \
        *out = (T)(a + b); return 0; \
    } \
    static inline unsigned char hs_sub_overflow_##NAME(T a, T b, T *out) { \
        if (b > a) { return 1; } \
        *out = (T)(a - b); return 0; \
    } \
    static inline unsigned char hs_mul_overflow_##NAME(T a, T b, T *out) { \
        if (a != 0 && b > (MAX) / a) { return 1; } \
        *out = (T)(a * b); return 0; \
    }

#define HS_DEF_OVF_SIGNED(NAME, T, MIN, MAX) \
    static inline unsigned char hs_add_overflow_##NAME(T a, T b, T *out) { \
        if ((b > 0 && a > (MAX) - b) || (b < 0 && a < (MIN) - b)) { return 1; } \
        *out = (T)(a + b); return 0; \
    } \
    static inline unsigned char hs_sub_overflow_##NAME(T a, T b, T *out) { \
        if ((b < 0 && a > (MAX) + b) || (b > 0 && a < (MIN) + b)) { return 1; } \
        *out = (T)(a - b); return 0; \
    } \
    static inline unsigned char hs_mul_overflow_##NAME(T a, T b, T *out) { \
        if (a > 0) { \
            if (b > 0 && a > (MAX) / b) { return 1; } \
            if (b < 0 && b < (MIN) / a) { return 1; } \
        } else if (a < 0) { \
            if (b > 0 && a < (MIN) / b) { return 1; } \
            if (b < 0 && a != 0 && b < (MAX) / a) { return 1; } \
        } \
        *out = (T)(a * b); return 0; \
    }

#endif

#if HS_HAS_BUILTIN_OVERFLOW
HS_DEF_OVF(u32,  uint32_t)
HS_DEF_OVF(u64,  uint64_t)
HS_DEF_OVF(size, size_t)
HS_DEF_OVF(i32,  int32_t)
HS_DEF_OVF(i64,  int64_t)
#else
HS_DEF_OVF_UNSIGNED(u32,  uint32_t, UINT32_MAX)
HS_DEF_OVF_UNSIGNED(u64,  uint64_t, UINT64_MAX)
HS_DEF_OVF_UNSIGNED(size, size_t,   SIZE_MAX)
HS_DEF_OVF_SIGNED(i32, int32_t, INT32_MIN, INT32_MAX)
HS_DEF_OVF_SIGNED(i64, int64_t, INT64_MIN, INT64_MAX)
#endif

HANDY_END_DECLS

#endif
