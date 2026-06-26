#ifndef HANDY_CONTRACT_H
#define HANDY_CONTRACT_H

#include "handy/handy_assert.h"

#define HS_REQUIRE(cond, msg) HS_ASSERT_TRUE((cond), (msg))
#define HS_ENSURE(cond, msg) HS_ASSERT_TRUE((cond), (msg))
#define HS_INVARIANT(cond, msg) HS_ASSERT_TRUE((cond), (msg))

#define HS_GUARD_OR_RETURN(cond, msg, retval) \
    do { \
        if (!(cond)) { \
            HS_ASSERT(0, (msg)); \
            return (retval); \
        } \
    } while (0)

#define HS_GUARD_OR_RETURN_VOID(cond, msg) \
    do { \
        if (!(cond)) { \
            HS_ASSERT(0, (msg)); \
            return; \
        } \
    } while (0)

#endif
