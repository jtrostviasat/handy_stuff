#!/usr/bin/env python3
import argparse
import json
import re
from pathlib import Path


def sanitize_identifier(value: str, default_prefix: str) -> str:
    sanitized = re.sub(r"[^0-9A-Za-z_]", "_", value).strip("_")
    if not sanitized:
        sanitized = default_prefix
    if sanitized[0].isdigit():
        sanitized = f"{default_prefix}_{sanitized}"
    return sanitized


def load_config_file(path: str) -> dict:
    config_path = Path(path)
    raw = config_path.read_text(encoding="utf-8")
    parsed = json.loads(raw)
    if not isinstance(parsed, dict):
        raise ValueError("config root must be a JSON object")
    return parsed


def update_generated_types_header(output_header: Path) -> None:
    index_path = output_header.parent / ".generated_types_index.json"
    aggregate_header = output_header.parent / "generated_types.h"

    entries = []
    if index_path.exists():
        try:
            existing = json.loads(index_path.read_text(encoding="utf-8"))
            if isinstance(existing, list):
                entries = [str(x) for x in existing if isinstance(x, str)]
        except json.JSONDecodeError:
            entries = []

    rel_output = output_header.name
    if rel_output not in entries:
        entries.append(rel_output)

    # Keep only headers that still exist.
    entries = sorted([name for name in entries if (output_header.parent / name).exists()])
    index_path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")

    includes = "".join([f'#include "{name}"\n' for name in entries])
    content = (
        "#ifndef GENERATED_TYPES_H\n"
        "#define GENERATED_TYPES_H\n\n"
        f"{includes}"
        "\n#endif\n"
    )
    aggregate_header.write_text(content, encoding="ascii")


HEADER_TEMPLATE = """#ifndef {guard}
#define {guard}

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "handy/handy_option.h"
#include "handy/handy_log.h"
#include "handy/handy_safety.h"

HS_OPTION_DECLARE({option_type_name}, {value_type})

typedef struct {{
    uint32_t raw;
}} {prefix}_handle;

#define {prefix_upper}_HANDLE_INVALID (({prefix}_handle){{0u}})

typedef struct {{
    {value_type} *values;
    uint16_t *generations;
    uint8_t *occupied;
    size_t length;
    size_t capacity;
}} {prefix}_handle_array;

typedef struct {{
    size_t active_count;
    size_t free_count;
    size_t capacity;
}} {prefix}_handle_array_stats;

typedef bool (*{prefix}_for_each_fn)(size_t slot_index, {prefix}_handle handle, {value_type} *value, void *ctx);
typedef bool (*{prefix}_predicate_fn)(size_t slot_index, {prefix}_handle handle, const {value_type} *value, void *ctx);

static inline bool {prefix}_handle_decode({prefix}_handle handle, uint16_t *out_index, uint16_t *out_generation) {{
    uint32_t raw = handle.raw;
    uint16_t index = (uint16_t)(raw & 0xFFFFu);
    uint16_t generation = (uint16_t)(raw >> 16);

    if (raw == 0u || generation == 0u) {{
        return false;
    }}
    *out_index = index;
    *out_generation = generation;
    return true;
}}

static inline {prefix}_handle {prefix}_handle_encode(uint16_t index, uint16_t generation) {{
    return ({prefix}_handle){{((uint32_t)generation << 16) | (uint32_t)index}};
}}

static inline size_t {prefix}_handle_array_required_values_bytes(size_t capacity) {{
    return sizeof({value_type}) * capacity;
}}

static inline size_t {prefix}_handle_array_required_generations_bytes(size_t capacity) {{
    return sizeof(uint16_t) * capacity;
}}

static inline size_t {prefix}_handle_array_required_occupied_bytes(size_t capacity) {{
    return sizeof(uint8_t) * capacity;
}}

static inline bool {prefix}_handle_array_init(
    {prefix}_handle_array *array,
    {value_type} *values_memory,
    uint16_t *generation_memory,
    uint8_t *occupied_memory,
    size_t length,
    size_t capacity
) {{
    size_t i;

    if (array == NULL || values_memory == NULL || generation_memory == NULL || occupied_memory == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_init", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    if (hs_check_len_cap(length, capacity) != HS_OK || capacity > (size_t)UINT16_MAX) {{
        HS_LOG_DEBUG("{prefix}_handle_array_init", "invalid length/capacity length=%zu capacity=%zu", length, capacity);
        HS_ASSERT(false, "invalid length/capacity");
        return false;
    }}

    array->values = values_memory;
    array->generations = generation_memory;
    array->occupied = occupied_memory;
    array->length = length;
    array->capacity = capacity;

    for (i = 0u; i < capacity; ++i) {{
        array->generations[i] = 1u;
        array->occupied[i] = (uint8_t)(i < length ? 1u : 0u);
    }}
    return true;
}}

static inline size_t {prefix}_handle_array_length(const {prefix}_handle_array *array) {{
    return array == NULL ? 0u : array->length;
}}

static inline size_t {prefix}_handle_array_capacity(const {prefix}_handle_array *array) {{
    return array == NULL ? 0u : array->capacity;
}}

static inline bool {prefix}_handle_array_is_empty(const {prefix}_handle_array *array) {{
    return array == NULL || array->length == 0u;
}}

static inline bool {prefix}_handle_array_stats(
    const {prefix}_handle_array *array,
    {prefix}_handle_array_stats *out_stats
) {{
    if (array == NULL || out_stats == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_stats", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    out_stats->active_count = array->length;
    out_stats->capacity = array->capacity;
    out_stats->free_count = array->capacity - array->length;
    return true;
}}

static inline bool {prefix}_handle_array_create(
    {prefix}_handle_array *array,
    {value_type} initial_value,
    {prefix}_handle *out_handle
) {{
    size_t i;

    if (array == NULL || out_handle == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_create", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    if (array->length >= array->capacity) {{
        HS_LOG_DEBUG("{prefix}_handle_array_create", "array full length=%zu capacity=%zu", array->length, array->capacity);
        return false;
    }}

    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] == 0u) {{
            uint16_t generation = array->generations[i];
            if (generation == 0u) {{
                generation = 1u;
                array->generations[i] = generation;
            }}
            array->occupied[i] = 1u;
            array->values[i] = initial_value;
            array->length += 1u;
            *out_handle = {prefix}_handle_encode((uint16_t)i, generation);
            HS_LOG_DEBUG(
                "{prefix}_handle_array_create",
                "handle=0x%08X index=%u generation=%u",
                out_handle->raw,
                (unsigned)i,
                (unsigned)generation
            );
            return true;
        }}
    }}
    return false;
}}

static inline bool {prefix}_handle_array_destroy({prefix}_handle_array *array, {prefix}_handle handle) {{
    uint16_t index;
    uint16_t generation;

    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_destroy", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    if (!{prefix}_handle_decode(handle, &index, &generation)) {{
        return false;
    }}
    if (hs_check_index((size_t)index, array->capacity) != HS_OK) {{
        return false;
    }}
    if (array->occupied[index] == 0u || array->generations[index] != generation) {{
        return false;
    }}

    array->occupied[index] = 0u;
    array->length -= 1u;
    array->generations[index] = (uint16_t)(array->generations[index] == UINT16_MAX ? 1u : array->generations[index] + 1u);
    return true;
}}

static inline bool {prefix}_handle_array_get(
    const {prefix}_handle_array *array,
    {prefix}_handle handle,
    {value_type} *out_value
) {{
    uint16_t index;
    uint16_t generation;

    if (array == NULL || out_value == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_get", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    if (!{prefix}_handle_decode(handle, &index, &generation)) {{
        return false;
    }}
    if (hs_check_index((size_t)index, array->capacity) != HS_OK) {{
        return false;
    }}
    if (array->occupied[index] == 0u || array->generations[index] != generation) {{
        return false;
    }}

    *out_value = array->values[index];
    return true;
}}

static inline Option({option_type_name}) {prefix}_handle_array_get_option(
    const {prefix}_handle_array *array,
    {prefix}_handle handle
) {{
    {value_type} value;
    if ({prefix}_handle_array_get(array, handle, &value)) {{
        return Some({option_type_name}, value);
    }}
    return None({option_type_name});
}}

static inline bool {prefix}_handle_array_set(
    {prefix}_handle_array *array,
    {prefix}_handle handle,
    {value_type} value
) {{
    uint16_t index;
    uint16_t generation;

    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_set", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    if (!{prefix}_handle_decode(handle, &index, &generation)) {{
        return false;
    }}
    if (hs_check_index((size_t)index, array->capacity) != HS_OK) {{
        return false;
    }}
    if (array->occupied[index] == 0u || array->generations[index] != generation) {{
        return false;
    }}

    array->values[index] = value;
    return true;
}}

static inline bool {prefix}_handle_array_contains(
    const {prefix}_handle_array *array,
    {prefix}_handle handle
) {{
    uint16_t index;
    uint16_t generation;

    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_contains", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    if (!{prefix}_handle_decode(handle, &index, &generation)) {{
        return false;
    }}
    if (hs_check_index((size_t)index, array->capacity) != HS_OK) {{
        return false;
    }}
    return array->occupied[index] != 0u && array->generations[index] == generation;
}}

static inline bool {prefix}_handle_array_clear({prefix}_handle_array *array) {{
    size_t i;
    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_clear", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    for (i = 0u; i < array->capacity; ++i) {{
        array->occupied[i] = 0u;
        array->generations[i] = (uint16_t)(array->generations[i] == UINT16_MAX ? 1u : array->generations[i] + 1u);
    }}
    array->length = 0u;
    return true;
}}

static inline bool {prefix}_handle_array_reset(
    {prefix}_handle_array *array,
    size_t length,
    {value_type} fill_value
) {{
    size_t i;
    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_reset", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    if (hs_check_len_cap(length, array->capacity) != HS_OK) {{
        return false;
    }}

    for (i = 0u; i < array->capacity; ++i) {{
        array->generations[i] = (uint16_t)(array->generations[i] == UINT16_MAX ? 1u : array->generations[i] + 1u);
        if (i < length) {{
            array->occupied[i] = 1u;
            array->values[i] = fill_value;
        }} else {{
            array->occupied[i] = 0u;
        }}
    }}
    array->length = length;
    return true;
}}

static inline bool {prefix}_handle_array_fill({prefix}_handle_array *array, {value_type} value) {{
    size_t i;
    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_fill", "array is NULL");
        HS_ASSERT(false, "array is NULL");
        return false;
    }}
    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] != 0u) {{
            array->values[i] = value;
        }}
    }}
    return true;
}}

static inline bool {prefix}_handle_array_copy_active(
    const {prefix}_handle_array *array,
    {value_type} *out_values,
    size_t out_capacity,
    size_t *out_written
) {{
    size_t i;
    size_t written = 0u;

    if (array == NULL || out_values == NULL || out_written == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_copy_active", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    if (out_capacity < array->length) {{
        return false;
    }}

    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] != 0u) {{
            out_values[written] = array->values[i];
            written += 1u;
        }}
    }}
    *out_written = written;
    return true;
}}

static inline bool {prefix}_handle_array_for_each(
    {prefix}_handle_array *array,
    {prefix}_for_each_fn fn,
    void *ctx
) {{
    size_t i;
    if (array == NULL || fn == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_for_each", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] != 0u) {{
            {prefix}_handle h = {prefix}_handle_encode((uint16_t)i, array->generations[i]);
            if (!fn(i, h, &array->values[i], ctx)) {{
                return true;
            }}
        }}
    }}
    return true;
}}

static inline bool {prefix}_handle_array_find_first(
    const {prefix}_handle_array *array,
    {prefix}_predicate_fn pred,
    void *ctx,
    {prefix}_handle *out_handle
) {{
    size_t i;
    if (array == NULL || pred == NULL || out_handle == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_find_first", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}
    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] != 0u) {{
            {prefix}_handle h = {prefix}_handle_encode((uint16_t)i, array->generations[i]);
            if (pred(i, h, &array->values[i], ctx)) {{
                *out_handle = h;
                return true;
            }}
        }}
    }}
    return false;
}}

static inline bool {prefix}_handle_array_rebind(
    {prefix}_handle_array *array,
    {value_type} *new_values,
    uint16_t *new_generations,
    uint8_t *new_occupied,
    size_t new_capacity,
    bool copy_existing
) {{
    size_t i;
    size_t new_length = 0u;

    if (
        array == NULL ||
        new_values == NULL ||
        new_generations == NULL ||
        new_occupied == NULL ||
        new_capacity > (size_t)UINT16_MAX
    ) {{
        HS_LOG_DEBUG("{prefix}_handle_array_rebind", "invalid args");
        HS_ASSERT(false, "invalid args");
        return false;
    }}

    for (i = 0u; i < new_capacity; ++i) {{
        new_generations[i] = 1u;
        new_occupied[i] = 0u;
    }}

    if (copy_existing) {{
        if (array->length > new_capacity) {{
            return false;
        }}
        for (i = 0u; i < array->capacity; ++i) {{
            if (array->occupied[i] != 0u) {{
                if (i >= new_capacity) {{
                    return false;
                }}
                new_values[i] = array->values[i];
                new_generations[i] = array->generations[i] == 0u ? 1u : array->generations[i];
                new_occupied[i] = 1u;
                new_length += 1u;
            }}
        }}
    }}

    array->values = new_values;
    array->generations = new_generations;
    array->occupied = new_occupied;
    array->capacity = new_capacity;
    array->length = copy_existing ? new_length : 0u;
    return true;
}}

static inline bool {prefix}_handle_array_debug_validate(const {prefix}_handle_array *array) {{
    size_t i;
    size_t occupied_count = 0u;

    if (array == NULL || array->values == NULL || array->generations == NULL || array->occupied == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_debug_validate", "invalid pointers");
        return false;
    }}
    if (hs_check_len_cap(array->length, array->capacity) != HS_OK || array->capacity > (size_t)UINT16_MAX) {{
        HS_LOG_DEBUG("{prefix}_handle_array_debug_validate", "invalid length/capacity");
        return false;
    }}

    for (i = 0u; i < array->capacity; ++i) {{
        if (array->occupied[i] != 0u && array->occupied[i] != 1u) {{
            return false;
        }}
        if (array->generations[i] == 0u) {{
            return false;
        }}
        if (array->occupied[i] != 0u) {{
            occupied_count += 1u;
        }}
    }}
    return occupied_count == array->length;
}}

static inline void {prefix}_handle_array_debug_dump(const {prefix}_handle_array *array) {{
#if HANDY_STUFF_DEBUG
    size_t i;
    if (array == NULL) {{
        HS_LOG_DEBUG("{prefix}_handle_array_debug_dump", "array=NULL");
        return;
    }}
    HS_LOG_DEBUG("{prefix}_handle_array", "length=%zu capacity=%zu", array->length, array->capacity);
    for (i = 0u; i < array->capacity; ++i) {{
        HS_LOG_DEBUG(
            "{prefix}_handle_array",
            "slot[%zu]: occupied=%u generation=%u",
            i,
            (unsigned)array->occupied[i],
            (unsigned)array->generations[i]
        );
    }}
#else
    (void)array;
#endif
}}

#endif
"""


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Generate a single C header with a handle-based array API for any value type. "
            "Works with handy_option.h, handy_log.h, and handy_safety.h."
        )
    )
    parser.add_argument(
        "--config",
        help="Optional JSON config file with keys: value_type, prefix, option_type_name, output",
    )
    parser.add_argument(
        "--value-type",
        help='C value type to store (examples: int32_t, my_struct_t, "struct MyType")',
    )
    parser.add_argument(
        "--prefix",
        help="Identifier prefix for generated names (example: i32, user, widget)",
    )
    parser.add_argument(
        "--option-type-name",
        help="Token for HS_OPTION_DECLARE / Option(T). Defaults to --prefix.",
    )
    parser.add_argument(
        "--output",
        help="Output header path. Defaults to <prefix>_handle_array.h",
    )
    args = parser.parse_args()

    config = {}
    if args.config:
        try:
            config = load_config_file(args.config)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            parser.error(f"failed to load config file: {exc}")

    value_type_raw = args.value_type if args.value_type is not None else config.get("value_type")
    prefix_raw = args.prefix if args.prefix is not None else config.get("prefix")
    option_type_name_raw = (
        args.option_type_name if args.option_type_name is not None else config.get("option_type_name")
    )
    output_raw = args.output if args.output is not None else config.get("output")

    if not value_type_raw:
        parser.error("missing required value type. Provide --value-type or config key value_type.")
    if not prefix_raw:
        parser.error("missing required prefix. Provide --prefix or config key prefix.")

    prefix = sanitize_identifier(prefix_raw, "type")
    option_type_name = sanitize_identifier(option_type_name_raw or prefix, "type")
    output = Path(output_raw or f"{prefix}_handle_array.h")
    guard = f"{prefix.upper()}_HANDLE_ARRAY_H"

    content = HEADER_TEMPLATE.format(
        guard=guard,
        option_type_name=option_type_name,
        value_type=str(value_type_raw).strip(),
        prefix=prefix,
        prefix_upper=prefix.upper(),
    )
    output.write_text(content, encoding="ascii")
    update_generated_types_header(output.resolve())
    print(f"Generated {output}")
    print(f"Updated {output.parent / 'generated_types.h'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
