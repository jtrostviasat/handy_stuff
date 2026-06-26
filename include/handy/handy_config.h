/*
 * handy_config.h - central compile-time configuration for the handy library.
 *
 * All toggles live here so a project can set them in one place (or override
 * via build flags). Every other handy header includes this first.
 *
 *   HANDY_STUFF_DEBUG     1 = compile debug logging paths
 *   RUNTIME_MEMSAFE       1 = compile runtime assertion/abort paths
 *   HANDY_HAVE_STDIO      1 = handy_log + HS_FAIL may use stdio/stdlib
 *
 * Defaults are conservative (off) so the library is cheap to include.
 */
#ifndef HANDY_CONFIG_H
#define HANDY_CONFIG_H

#ifndef HANDY_STUFF_DEBUG
#define HANDY_STUFF_DEBUG 0
#endif

#ifndef RUNTIME_MEMSAFE
#define RUNTIME_MEMSAFE 0
#endif

#ifndef HANDY_HAVE_STDIO
#define HANDY_HAVE_STDIO 1
#endif

#if defined(__cplusplus)
#define HANDY_BEGIN_DECLS extern "C" {
#define HANDY_END_DECLS }
#else
#define HANDY_BEGIN_DECLS
#define HANDY_END_DECLS
#endif

#if defined(__GNUC__) || defined(__clang__)
#define HS_NODISCARD __attribute__((warn_unused_result))
#define HS_UNUSED    __attribute__((unused))
#define HS_NORETURN  __attribute__((noreturn))
#else
#define HS_NODISCARD
#define HS_UNUSED
#define HS_NORETURN
#endif

#endif
