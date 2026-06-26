#ifndef HANDY_SAFETY_H
#define HANDY_SAFETY_H

#include <stddef.h>

#include "handy/handy_config.h"

#if RUNTIME_MEMSAFE && HANDY_HAVE_STDIO
#include <stdio.h>
#include <stdlib.h>
#endif

HANDY_BEGIN_DECLS

#if RUNTIME_MEMSAFE
#if HANDY_HAVE_STDIO
#define HS_FAIL(msg) \
    do { \
        fprintf(stderr, "[HS_FAIL] %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        abort(); \
    } while (0)
#else
/* Freestanding: caller must provide handy_panic(const char*). */
extern HS_NORETURN void handy_panic(const char *msg);
#define HS_FAIL(msg) handy_panic((msg))
#endif
#define HS_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            HS_FAIL((msg)); \
        } \
    } while (0)
#else
#define HS_ASSERT(cond, msg) do { (void)sizeof(cond); } while (0)
#endif

#define HS_FOR_BOUNDED(i, start, end, max_iters) \
    for (size_t i = (start), _hs_end = (end), _hs_n = 0u; \
         (i) < _hs_end && _hs_n < (max_iters); \
         ++(i), ++_hs_n)

typedef enum {
    HS_OK = 0,
    HS_ERR_BOUNDS,
    HS_ERR_ARG,
    HS_ERR_CAP
} hs_status;

static inline HS_NODISCARD hs_status hs_check_index(size_t index, size_t length) {
    if (index >= length) {
        return HS_ERR_BOUNDS;
    }
    return HS_OK;
}

static inline HS_NODISCARD hs_status hs_check_len_cap(size_t length, size_t capacity) {
    if (length > capacity) {
        return HS_ERR_CAP;
    }
    return HS_OK;
}

HANDY_END_DECLS

#endif
