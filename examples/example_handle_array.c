/*
 * example_handle_array.c - minimal usage of the handy library.
 *
 * Build (from repo root):
 *   cmake -S . -B build -DHANDY_BUILD_EXAMPLES=ON
 *   cmake --build build
 */
#include "handy/handy.h"

#include <stdio.h>

HS_OPTION_DECLARE(i32, int32_t)

int main(void) {
    hs_log_set_level(HS_LOG_LEVEL_INFO);
    HS_LOG_INFO("handy", "handy %s starting", HANDY_VERSION_STRING);

    Option(i32) maybe_value = Some(i32, 7);
    int32_t resolved = OptionUnwrapOr(i32, maybe_value, -1);
    printf("resolved value = %d\n", resolved);

    uint32_t flags = 0u;
    flags = hs_set_bit_u32(flags, 1u);
    flags = hs_set_bit_u32(flags, 4u);
    printf("flags = 0x%08x\n", flags);

    return 0;
}
