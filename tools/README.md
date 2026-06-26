# tools/

## `generate_handle_array_header.py`

Generates a type-specific handle-pool/array header for use with the rest of
the handy library.

```sh
python tools/generate_handle_array_header.py \
    --value-type "struct telemetry_sample" \
    --type-name telemetry \
    --output include/handy_generated/telemetry_handle_array.h
```

Or via JSON config:

```sh
python tools/generate_handle_array_header.py --config tools/generate_handle_array_header.example.json
```

Each run also refreshes a sibling `generated_types.h` aggregator and a
`.generated_types_index.json` registry next to the output file, so a project
can `#include "handy_generated/generated_types.h"` to pick up every generated
type at once.

The emitted header includes `handy/handy_option.h`, `handy/handy_log.h`, and
`handy/handy_safety.h`, so make sure your build adds `-Iinclude`.

## `generate_container_header.py`

Emits a type-specific container header for one of:

- `vec` — heap-backed (or fixed-buffer non-owning) dynamic array
- `ring` — fixed-capacity ring buffer over a user-provided buffer
- `map_u64` — open-addressing hash map keyed by `uint64_t`
- `map_str` — open-addressing hash map keyed by `hs_str` (non-owning keys)

One invocation = one header = one kind for one element type.

```sh
# Dynamic array of int32_t
python tools/generate_container_header.py \
    --kind vec --type-name int32 --value-type int32_t \
    --output include/handy_generated/int32_vec.h

# Ring buffer of struct event
python tools/generate_container_header.py \
    --kind ring --type-name event --value-type "struct event" \
    --output include/handy_generated/event_ring.h

# u64-keyed map of pointers
python tools/generate_container_header.py \
    --kind map_u64 --type-name user --value-type "struct user *" \
    --output include/handy_generated/user_map.h

# string-keyed map of int (word counts)
python tools/generate_container_header.py \
    --kind map_str --type-name word_count --value-type int \
    --output include/handy_generated/word_count_map.h
```

All emitted headers depend on `handy/handy_safety.h`; `map_str` additionally
depends on `handy/handy_str.h`. `vec`, `map_u64`, and `map_str` use
`malloc`/`realloc`/`free`. `ring` is allocation-free.

## `amalgamate.py`

Emit single-file `handy_all.h` + `handy_all.c` from the headers under
`include/handy/` and `src/`:

```sh
python tools/amalgamate.py --out-dir amalgamated
```

Drop the two files into any project, compile `handy_all.c`, and `#include
"handy_all.h"`. No build system or `-Iinclude` required.

## `generate_compile_commands.py`

Produce a `compile_commands.json` so clangd/clang-tidy can understand a
unity build (a single `all.c` that `#include`s everything else):

```sh
python tools/generate_compile_commands.py \
    --unity src/all.c -- gcc -std=c99 -Iinclude
```

The generator walks the unity file's `#include "..."` graph and emits one
entry per included `.c` translation unit, all sharing the same compiler
flags. Use `--files` instead of `--unity` to pass an explicit file list.

