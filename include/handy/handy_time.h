/*
 * handy_time.h - monotonic time + sleep, Windows + POSIX.
 *
 *   uint64_t t0 = hs_now_ns();
 *   ... work ...
 *   uint64_t elapsed_ns = hs_now_ns() - t0;
 *   double  elapsed_ms  = hs_ns_to_ms(elapsed_ns);
 *
 * Header-only; the Windows path uses QueryPerformanceCounter and the POSIX
 * path uses clock_gettime(CLOCK_MONOTONIC).
 */
#ifndef HANDY_TIME_H
#define HANDY_TIME_H

#include <stdint.h>

#include "handy/handy_config.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

HANDY_BEGIN_DECLS

static inline uint64_t hs_now_ns(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) { QueryPerformanceFrequency(&freq); }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    /* (counter * 1e9) / freq, computed in a way that avoids 64-bit overflow
     * for typical freq values (~10 MHz). */
    uint64_t whole_secs = (uint64_t)counter.QuadPart / (uint64_t)freq.QuadPart;
    uint64_t remainder  = (uint64_t)counter.QuadPart % (uint64_t)freq.QuadPart;
    return whole_secs * 1000000000ull + (remainder * 1000000000ull) / (uint64_t)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
#endif
}

static inline double hs_ns_to_ms(uint64_t ns) { return (double)ns / 1.0e6; }
static inline double hs_ns_to_s(uint64_t ns)  { return (double)ns / 1.0e9; }

static inline void hs_sleep_ms(uint32_t ms) {
#if defined(_WIN32)
    Sleep(ms);
#else
    struct timespec req;
    req.tv_sec = (time_t)(ms / 1000u);
    req.tv_nsec = (long)((ms % 1000u) * 1000000u);
    (void)nanosleep(&req, NULL);
#endif
}

/* Cheap scoped timer using __attribute__((cleanup)) where available. */
#if defined(__GNUC__) || defined(__clang__)
static inline void hs_timer_end_(uint64_t *start) {
    /* User reads *start themselves; this hook is just a place to put a
     * cleanup callback if desired in a wrapper macro. */
    (void)start;
}
#define HS_SCOPED_TIMER(name) \
    uint64_t name __attribute__((cleanup(hs_timer_end_))) = hs_now_ns()
#endif

HANDY_END_DECLS

#endif
