#ifndef HANDY_LOG_H
#define HANDY_LOG_H

#include <stdio.h>

#include "handy/handy_config.h"

HANDY_BEGIN_DECLS

typedef enum {
    HS_LOG_LEVEL_DEBUG = 0,
    HS_LOG_LEVEL_INFO = 1,
    HS_LOG_LEVEL_WARN = 2,
    HS_LOG_LEVEL_ERROR = 3,
    HS_LOG_LEVEL_NONE = 4
} hs_log_level;

void hs_log_set_level(hs_log_level level);
hs_log_level hs_log_get_level(void);
void hs_log_set_stream(FILE *stream);
void hs_log_message(hs_log_level level, const char *tag, const char *fmt, ...);

#if HANDY_STUFF_DEBUG
#define HS_LOG_DEBUG(...) hs_log_message(HS_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define HS_LOG_DEBUG(...) do { } while (0)
#endif

#define HS_LOG_INFO(...) hs_log_message(HS_LOG_LEVEL_INFO, __VA_ARGS__)
#define HS_LOG_WARN(...) hs_log_message(HS_LOG_LEVEL_WARN, __VA_ARGS__)
#define HS_LOG_ERROR(...) hs_log_message(HS_LOG_LEVEL_ERROR, __VA_ARGS__)

HANDY_END_DECLS

#endif
