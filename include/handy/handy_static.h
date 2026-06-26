#ifndef HANDY_STATIC_H
#define HANDY_STATIC_H

#include <stddef.h>

#define HS_JOIN2(a, b) a##b
#define HS_JOIN(a, b) HS_JOIN2(a, b)

/* C99-compatible static assert. message must be identifier-safe token text.
 * The line number is folded in so two asserts in the same file can share a
 * message token without colliding. */
#define HS_STATIC_ASSERT(cond, message_token) \
    typedef char HS_JOIN(HS_JOIN(hs_static_assert_, message_token), HS_JOIN(_line_, __LINE__))[(cond) ? 1 : -1]

#define HS_ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#define HS_OFFSET_OF(type, member) offsetof(type, member)
#define HS_ALIGN_OF(type) HS_OFFSET_OF(struct { char c; type member; }, member)

#define HS_ASSERT_SIZE(type, expected_size, message_token) \
    HS_STATIC_ASSERT(sizeof(type) == (expected_size), message_token)

#define HS_ASSERT_ALIGN(type, expected_align, message_token) \
    HS_STATIC_ASSERT(HS_ALIGN_OF(type) == (expected_align), message_token)

#endif
