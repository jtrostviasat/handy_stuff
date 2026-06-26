#include "handy_test.h"

#include "handy/handy.h"

HS_OPTION_DECLARE(u32, uint32_t)

HS_TEST(bits_basic) {
    HS_TEST_CHECK(hs_set_bit_u32(0u, 3u) == 8u, "set bit 3 of 0");
    HS_TEST_CHECK(hs_clear_bit_u32(0xFFu, 0u) == 0xFEu, "clear bit 0");
    HS_TEST_CHECK(hs_test_bit_u32(0x10u, 4u) == 1u, "test bit 4");
    HS_TEST_CHECK(hs_toggle_bit_u32(0u, 5u) == 32u, "toggle bit 5");
}

HS_TEST(option_basic) {
    Option(u32) some_value = Some(u32, 42u);
    Option(u32) no_value = None(u32);
    HS_TEST_CHECK(OptionIsSome(u32, some_value), "some is some");
    HS_TEST_CHECK(OptionIsNone(u32, no_value), "none is none");
    HS_TEST_CHECK(OptionUnwrapOr(u32, some_value, 0u) == 42u, "unwrap some");
    HS_TEST_CHECK(OptionUnwrapOr(u32, no_value, 7u) == 7u, "unwrap none default");
}

HS_TEST(static_asserts) {
    /* Two asserts with the same token must not collide. */
    HS_STATIC_ASSERT(sizeof(int) >= 2, int_size);
    HS_STATIC_ASSERT(sizeof(long) >= 4, int_size);
    HS_TEST_CHECK(HS_ARRAY_COUNT(((int[]){1, 2, 3})) == 3u, "array count");
}

HS_TEST(arena_basic) {
    unsigned char backing[1024];
    hs_arena arena;
    hs_arena_init(&arena, backing, sizeof backing);
    int *a = HS_ARENA_NEW(&arena, int);
    int *b = HS_ARENA_NEW_ARRAY(&arena, int, 16);
    HS_TEST_CHECK(a != NULL && b != NULL, "arena allocations");
    size_t mark = hs_arena_mark(&arena);
    (void)HS_ARENA_NEW_ARRAY(&arena, int, 64);
    HS_TEST_CHECK(hs_arena_used(&arena) > mark, "arena advanced");
    hs_arena_restore(&arena, mark);
    HS_TEST_CHECK(hs_arena_used(&arena) == mark, "arena rolled back");
}

HS_TEST(str_basic) {
    hs_str s = HS_STR_LIT("hello, world");
    HS_TEST_CHECK(s.len == 12u, "literal length");
    HS_TEST_CHECK(hs_str_starts_with(s, HS_STR_LIT("hello")), "starts_with");
    HS_TEST_CHECK(hs_str_ends_with(s, HS_STR_LIT("world")), "ends_with");
    hs_str head, tail;
    HS_TEST_CHECK(hs_str_split_char(s, ',', &head, &tail), "split found");
    HS_TEST_CHECK(hs_str_eq(head, HS_STR_LIT("hello")), "split head");
    HS_TEST_CHECK(hs_str_eq(hs_str_trim(tail), HS_STR_LIT("world")), "split tail trimmed");
}

HS_TEST(strbuilder_basic) {
    unsigned char backing[1024];
    hs_arena arena;
    hs_arena_init(&arena, backing, sizeof backing);
    hs_strbuilder sb;
    hs_sb_init(&sb, &arena);
    HS_TEST_CHECK(hs_sb_append_cstr(&sb, "user=") == HS_OK, "append cstr");
    HS_TEST_CHECK(hs_sb_append_u64(&sb, 42u) == HS_OK, "append u64");
    HS_TEST_CHECK(hs_str_eq(hs_sb_to_str(&sb), HS_STR_LIT("user=42")), "built string");
}

HS_TEST(checked_basic) {
    uint32_t out;
    HS_TEST_CHECK(hs_add_overflow_u32(1u, 2u, &out) == 0 && out == 3u, "add ok");
    HS_TEST_CHECK(hs_add_overflow_u32(UINT32_MAX, 1u, &out) == 1, "add overflow");
    size_t total;
    HS_TEST_CHECK(hs_mul_overflow_size(100u, sizeof(int), &total) == 0, "mul ok");
}

HS_TEST(time_monotonic) {
    uint64_t t0 = hs_now_ns();
    hs_sleep_ms(2);
    uint64_t t1 = hs_now_ns();
    HS_TEST_CHECK(t1 >= t0, "monotonic non-decreasing");
}

int main(void) {
    HS_TEST_RUN(bits_basic);
    HS_TEST_RUN(option_basic);
    HS_TEST_RUN(static_asserts);
    HS_TEST_RUN(arena_basic);
    HS_TEST_RUN(str_basic);
    HS_TEST_RUN(strbuilder_basic);
    HS_TEST_RUN(checked_basic);
    HS_TEST_RUN(time_monotonic);
    return HS_TEST_REPORT();
}
