#include "handy/handy_log.h"

#include <stdarg.h>

/*
 * Thread-safety: this minimal logger uses unsynchronized global state for
 * level + stream. It is safe for single-threaded code and for "set once at
 * startup, read from many threads" usage. Use an external mutex if your code
 * reconfigures the logger from multiple threads.
 */
static hs_log_level g_hs_log_level = HS_LOG_LEVEL_DEBUG;
static FILE *g_hs_log_stream = NULL;

static const char *hs_log_level_name(hs_log_level level) {
    switch (level) {
        case HS_LOG_LEVEL_DEBUG: return "DEBUG";
        case HS_LOG_LEVEL_INFO: return "INFO";
        case HS_LOG_LEVEL_WARN: return "WARN";
        case HS_LOG_LEVEL_ERROR: return "ERROR";
        case HS_LOG_LEVEL_NONE: return "NONE";
        default: return "UNKNOWN";
    }
}

void hs_log_set_level(hs_log_level level) {
    g_hs_log_level = level;
}

hs_log_level hs_log_get_level(void) {
    return g_hs_log_level;
}

void hs_log_set_stream(FILE *stream) {
    g_hs_log_stream = stream;
}

void hs_log_message(hs_log_level level, const char *tag, const char *fmt, ...) {
    va_list args;
    FILE *stream;

    if (level < g_hs_log_level || level == HS_LOG_LEVEL_NONE || fmt == NULL) {
        return;
    }

    stream = g_hs_log_stream == NULL ? stderr : g_hs_log_stream;
    fprintf(stream, "[%s]", hs_log_level_name(level));
    if (tag != NULL && tag[0] != '\0') {
        fprintf(stream, "[%s] ", tag);
    } else {
        fputc(' ', stream);
    }

    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
    fputc('\n', stream);
}
