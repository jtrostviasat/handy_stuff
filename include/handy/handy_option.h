#ifndef HANDY_OPTION_H
#define HANDY_OPTION_H

#include "handy/handy_config.h"

/*
 * Tagged-option helpers. Each user-defined option type is declared once via
 * HS_OPTION_DECLARE(name, value_type) and the macros below give a small
 * Rust-like surface (Some, None, OptionIsSome, etc.).
 *
 * The tag is `unsigned char` (not uint8_t) so this header has zero standard
 * library dependencies.
 */

#define HS_OPTION_TYPE(TName) hs_option_##TName

#define HS_OPTION_DECLARE(TName, TValueType) \
    typedef struct { \
        unsigned char is_some; \
        TValueType value; \
    } HS_OPTION_TYPE(TName); \
    static inline HS_OPTION_TYPE(TName) hs_option_some_##TName(TValueType value) { \
        HS_OPTION_TYPE(TName) option; \
        option.is_some = 1u; \
        option.value = value; \
        return option; \
    } \
    static inline HS_OPTION_TYPE(TName) hs_option_none_##TName(void) { \
        HS_OPTION_TYPE(TName) option; \
        option.is_some = 0u; \
        return option; \
    } \
    static inline unsigned char hs_option_is_some_##TName(HS_OPTION_TYPE(TName) option) { \
        return option.is_some; \
    } \
    static inline unsigned char hs_option_is_none_##TName(HS_OPTION_TYPE(TName) option) { \
        return (unsigned char)!option.is_some; \
    } \
    static inline TValueType hs_option_unwrap_or_##TName(HS_OPTION_TYPE(TName) option, TValueType default_value) { \
        return option.is_some ? option.value : default_value; \
    }

/*
 * Short aliases. These are opt-in: define HANDY_OPTION_NO_SHORT_NAMES before
 * including this header to skip them and avoid namespace pollution.
 */
#ifndef HANDY_OPTION_NO_SHORT_NAMES
#define Option(TName) HS_OPTION_TYPE(TName)
#define Some(TName, value) hs_option_some_##TName((value))
#define None(TName) hs_option_none_##TName()
#define OptionIsSome(TName, option) hs_option_is_some_##TName((option))
#define OptionIsNone(TName, option) hs_option_is_none_##TName((option))
#define OptionUnwrapOr(TName, option, default_value) hs_option_unwrap_or_##TName((option), (default_value))
#endif

#endif
