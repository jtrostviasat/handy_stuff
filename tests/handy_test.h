/*
 * handy_test.h - dependency-free micro test harness.
 *
 *   HS_TEST(name) { ... HS_TEST_CHECK(cond, "msg"); }
 *   int main(void) { HS_TEST_RUN(name); return HS_TEST_REPORT(); }
 */
#ifndef HANDY_TEST_H
#define HANDY_TEST_H

#include <stdio.h>

static int g_hs_test_failures = 0;
static int g_hs_test_count = 0;
static const char *g_hs_test_current = "";

#define HS_TEST(name) static void hs_test_##name(void)

#define HS_TEST_CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            g_hs_test_failures++; \
            fprintf(stderr, "  FAIL %s: %s (%s:%d)\n", g_hs_test_current, (msg), __FILE__, __LINE__); \
        } \
    } while (0)

#define HS_TEST_RUN(name) \
    do { \
        g_hs_test_current = #name; \
        g_hs_test_count++; \
        printf("RUN  %s\n", #name); \
        hs_test_##name(); \
    } while (0)

#define HS_TEST_REPORT() \
    (printf("\n%d test(s), %d failure(s)\n", g_hs_test_count, g_hs_test_failures), \
     (g_hs_test_failures == 0) ? 0 : 1)

#endif
