#ifndef HANDY_BITS_H
#define HANDY_BITS_H

#include <stdint.h>

#include "handy/handy_safety.h"

#define HS_BITS_DECL(SUFFIX, TYPE, WIDTH, ALL_ONES) \
    static inline TYPE hs_bit_##SUFFIX(unsigned bit) { \
        HS_ASSERT(bit < (WIDTH), "bit out of range"); \
        return (TYPE)1u << bit; \
    } \
    static inline uint8_t hs_test_bit_##SUFFIX(TYPE value, unsigned bit) { \
        return (uint8_t)((value & hs_bit_##SUFFIX(bit)) != (TYPE)0u); \
    } \
    static inline TYPE hs_set_bit_##SUFFIX(TYPE value, unsigned bit) { \
        return (TYPE)(value | hs_bit_##SUFFIX(bit)); \
    } \
    static inline TYPE hs_clear_bit_##SUFFIX(TYPE value, unsigned bit) { \
        return (TYPE)(value & (TYPE)(~hs_bit_##SUFFIX(bit))); \
    } \
    static inline TYPE hs_toggle_bit_##SUFFIX(TYPE value, unsigned bit) { \
        return (TYPE)(value ^ hs_bit_##SUFFIX(bit)); \
    } \
    static inline TYPE hs_mask_##SUFFIX(unsigned width, unsigned lsb) { \
        TYPE base_mask; \
        HS_ASSERT(width > 0u && width <= (WIDTH), "invalid width"); \
        HS_ASSERT(lsb < (WIDTH) && (lsb + width) <= (WIDTH), "invalid lsb/width"); \
        if (width == (WIDTH)) { \
            return (ALL_ONES); \
        } \
        base_mask = (TYPE)(((TYPE)1u << width) - (TYPE)1u); \
        return (TYPE)(base_mask << lsb); \
    } \
    static inline TYPE hs_extract_field_##SUFFIX(TYPE value, unsigned width, unsigned lsb) { \
        return (TYPE)((value & hs_mask_##SUFFIX(width, lsb)) >> lsb); \
    } \
    static inline TYPE hs_insert_field_##SUFFIX(TYPE dst, TYPE field, unsigned width, unsigned lsb) { \
        TYPE mask = hs_mask_##SUFFIX(width, lsb); \
        TYPE shifted = (TYPE)((field << lsb) & mask); \
        return (TYPE)((dst & (TYPE)(~mask)) | shifted); \
    }

HS_BITS_DECL(u8, uint8_t, 8u, UINT8_MAX)
HS_BITS_DECL(u16, uint16_t, 16u, UINT16_MAX)
HS_BITS_DECL(u32, uint32_t, 32u, UINT32_MAX)
HS_BITS_DECL(u64, uint64_t, 64u, UINT64_MAX)

#define HS_BITS_DECL_SIGNED(SUFFIX, STYPE, UTYPE, USUFFIX) \
    static inline STYPE hs_bit_##SUFFIX(unsigned bit) { \
        return (STYPE)hs_bit_##USUFFIX(bit); \
    } \
    static inline uint8_t hs_test_bit_##SUFFIX(STYPE value, unsigned bit) { \
        return hs_test_bit_##USUFFIX((UTYPE)value, bit); \
    } \
    static inline STYPE hs_set_bit_##SUFFIX(STYPE value, unsigned bit) { \
        UTYPE uv = (UTYPE)value; \
        return (STYPE)hs_set_bit_##USUFFIX(uv, bit); \
    } \
    static inline STYPE hs_clear_bit_##SUFFIX(STYPE value, unsigned bit) { \
        UTYPE uv = (UTYPE)value; \
        return (STYPE)hs_clear_bit_##USUFFIX(uv, bit); \
    } \
    static inline STYPE hs_toggle_bit_##SUFFIX(STYPE value, unsigned bit) { \
        UTYPE uv = (UTYPE)value; \
        return (STYPE)hs_toggle_bit_##USUFFIX(uv, bit); \
    } \
    static inline STYPE hs_mask_##SUFFIX(unsigned width, unsigned lsb) { \
        return (STYPE)hs_mask_##USUFFIX(width, lsb); \
    } \
    static inline STYPE hs_extract_field_##SUFFIX(STYPE value, unsigned width, unsigned lsb) { \
        UTYPE uv = (UTYPE)value; \
        return (STYPE)hs_extract_field_##USUFFIX(uv, width, lsb); \
    } \
    static inline STYPE hs_insert_field_##SUFFIX(STYPE dst, STYPE field, unsigned width, unsigned lsb) { \
        UTYPE udst = (UTYPE)dst; \
        UTYPE ufield = (UTYPE)field; \
        return (STYPE)hs_insert_field_##USUFFIX(udst, ufield, width, lsb); \
    }

HS_BITS_DECL_SIGNED(i8, int8_t, uint8_t, u8)
HS_BITS_DECL_SIGNED(i16, int16_t, uint16_t, u16)
HS_BITS_DECL_SIGNED(i32, int32_t, uint32_t, u32)
HS_BITS_DECL_SIGNED(i64, int64_t, uint64_t, u64)

#undef HS_BITS_DECL_SIGNED
#undef HS_BITS_DECL

#endif
