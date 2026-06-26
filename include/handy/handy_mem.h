/*
 * handy_mem.h - optional bounded memory helpers.
 *
 * Pulled out of handy_safety.h so the foundation header doesn't drag in
 * <string.h>. Only include this when you actually need bounded copies.
 */
#ifndef HANDY_MEM_H
#define HANDY_MEM_H

#include <stddef.h>
#include <string.h>

#include "handy/handy_safety.h"

HANDY_BEGIN_DECLS

static inline HS_NODISCARD hs_status hs_memcpy_s(void *dst, size_t dst_capacity,
                                                 const void *src, size_t bytes) {
    if (dst == NULL || src == NULL) {
        return HS_ERR_ARG;
    }
    if (bytes > dst_capacity) {
        return HS_ERR_BOUNDS;
    }
    (void)memcpy(dst, src, bytes);
    return HS_OK;
}

static inline HS_NODISCARD hs_status hs_memmove_s(void *dst, size_t dst_capacity,
                                                  const void *src, size_t bytes) {
    if (dst == NULL || src == NULL) {
        return HS_ERR_ARG;
    }
    if (bytes > dst_capacity) {
        return HS_ERR_BOUNDS;
    }
    (void)memmove(dst, src, bytes);
    return HS_OK;
}

static inline HS_NODISCARD hs_status hs_memset_s(void *dst, size_t dst_capacity,
                                                 int value, size_t bytes) {
    if (dst == NULL) {
        return HS_ERR_ARG;
    }
    if (bytes > dst_capacity) {
        return HS_ERR_BOUNDS;
    }
    (void)memset(dst, value, bytes);
    return HS_OK;
}

HANDY_END_DECLS

#endif
