#ifndef HANDY_ASSERT_H
#define HANDY_ASSERT_H

#include <stdint.h>

#include "handy/handy_safety.h"

#define HS_ASSERT_TRUE(cond, msg) HS_ASSERT((cond), (msg))
#define HS_ASSERT_FALSE(cond, msg) HS_ASSERT(!(cond), (msg))

#define HS_ASSERT_EQ_U32(lhs, rhs, msg) \
    do { \
        uint32_t _hs_l = (uint32_t)(lhs); \
        uint32_t _hs_r = (uint32_t)(rhs); \
        HS_ASSERT(_hs_l == _hs_r, (msg)); \
    } while (0)

#define HS_ASSERT_NE_U32(lhs, rhs, msg) \
    do { \
        uint32_t _hs_l = (uint32_t)(lhs); \
        uint32_t _hs_r = (uint32_t)(rhs); \
        HS_ASSERT(_hs_l != _hs_r, (msg)); \
    } while (0)

#define HS_ASSERT_EQ_PTR(lhs, rhs, msg) \
    do { \
        const void *_hs_l = (const void *)(lhs); \
        const void *_hs_r = (const void *)(rhs); \
        HS_ASSERT(_hs_l == _hs_r, (msg)); \
    } while (0)

#define HS_ASSERT_NE_PTR(lhs, rhs, msg) \
    do { \
        const void *_hs_l = (const void *)(lhs); \
        const void *_hs_r = (const void *)(rhs); \
        HS_ASSERT(_hs_l != _hs_r, (msg)); \
    } while (0)

#define HS_UNREACHABLE(msg) HS_ASSERT(0, (msg))

#endif
