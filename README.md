# handy

> Small, drop-in C99 helpers for logging, debugging, correctness, and
> handle-based collections. One static library + a generator script.

`handy` is meant to be vendored or installed into other C projects so you stop
re-writing the same `assert`, `Option`, bit-twiddling, and bounds-checked
container code in every codebase.

## Highlights

- **C99 only.** No `stdbool.h`, no compiler-specific syntax in public headers.
- **Two cheap toggles.** Everything is gated by `HANDY_STUFF_DEBUG` and
  `RUNTIME_MEMSAFE`; both default to `0` so including handy in a release build
  is essentially free.
- **One umbrella header.** `#include "handy/handy.h"` gives you the whole API,
  or pull just the sub-headers you need.
- **Tiny.** One translation unit (`src/handy_log.c`); the rest is headers.
- **Generator.** `tools/generate_handle_array_header.py` emits a type-specific
  handle-pool/array header per value type, with optional auto-aggregation.

## Layout

```
include/handy/
  handy.h            umbrella header
  handy_config.h     central toggles + portability macros
  handy_version.h    HANDY_VERSION_* + HANDY_VERSION_STRING
  handy_safety.h     HS_ASSERT, HS_FAIL, hs_status, HS_FOR_BOUNDED
  handy_mem.h        hs_memcpy_s / hs_memmove_s / hs_memset_s (optional, pulls string.h)
  handy_static.h     HS_STATIC_ASSERT, HS_ARRAY_COUNT, HS_OFFSET_OF, HS_ALIGN_OF
  handy_assert.h     value/pointer-comparison asserts
  handy_contract.h   HS_REQUIRE/ENSURE/INVARIANT/GUARD_OR_RETURN
  handy_bits.h       u8/u16/u32/u64 + i8/i16/i32/i64 bit helpers
  handy_checked.h    overflow-checked add/sub/mul (u32/u64/size, i32/i64)
  handy_option.h     HS_OPTION_DECLARE + Some/None/OptionIsSome/...
  handy_str.h        hs_str slice + eq/starts_with/trim/split
  handy_arena.h      bump (linear) allocator
  handy_strbuilder.h growable buffer backed by an arena
  handy_time.h       hs_now_ns + hs_sleep_ms (Windows + POSIX)
  handy_log.h        leveled logger
src/handy_log.c
tools/               header generator + amalgamator + compile_commands gen
tests/               handy_test.h harness + test_basics.c
examples/            example_handle_array.c
CMakeLists.txt  LICENSE  CHANGELOG.md  .clang-format  .editorconfig
```

## Build

### CMake (recommended)

```sh
cmake -S . -B build -DHANDY_BUILD_TESTS=ON -DHANDY_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Options:

| Option                    | Default | Effect                                  |
|---------------------------|---------|-----------------------------------------|
| `HANDY_BUILD_TESTS`       | `ON`    | Build & register CTest tests            |
| `HANDY_BUILD_EXAMPLES`    | `OFF`   | Build `examples/`                       |
| `HANDY_STUFF_DEBUG`       | `OFF`   | Compile `HS_LOG_DEBUG` paths            |
| `HANDY_RUNTIME_MEMSAFE`   | `OFF`   | Compile `HS_ASSERT`/`HS_FAIL` paths     |

### As a unity build

If you don't use CMake, just compile `src/handy_log.c` with
`-Iinclude -std=c99` and link the resulting object into your binary.

### Vendoring

Drop the `include/handy/`, `src/`, and (optionally) `tools/` directories into
your project; add `include/` to your include path and `src/handy_log.c` to
your sources.

## Configuration

`include/handy/handy_config.h` holds every toggle. Override via build flags
(`-DHANDY_STUFF_DEBUG=1`) or by including a project-specific header before
`handy/handy.h`.

| Macro                | Default | Purpose                                |
|----------------------|---------|----------------------------------------|
| `HANDY_STUFF_DEBUG`  | `0`     | Enable `HS_LOG_DEBUG`                  |
| `RUNTIME_MEMSAFE`    | `0`     | Enable `HS_ASSERT` / `HS_FAIL`         |
| `HANDY_HAVE_STDIO`   | `1`     | Allow `stdio`/`stdlib` in safety/log   |

For freestanding builds, set `HANDY_HAVE_STDIO=0` and provide
`void handy_panic(const char *msg)`.

## Quick tour

```c
#include "handy/handy.h"

HS_OPTION_DECLARE(i32, int32_t)

int main(void) {
    hs_log_set_level(HS_LOG_LEVEL_INFO);
    HS_LOG_INFO("app", "handy %s ready", HANDY_VERSION_STRING);

    Option(i32) maybe = Some(i32, 42);
    int32_t value = OptionUnwrapOr(i32, maybe, -1);

    uint32_t flags = hs_set_bit_u32(0u, 3u);
    flags = hs_set_bit_u32(flags, 7u);

    HS_STATIC_ASSERT(sizeof(int32_t) == 4, i32_is_4_bytes);
    return value;
}
```

To avoid the short `Option/Some/None` macro names colliding with other code
(notably Windows headers that define `None`), define
`HANDY_OPTION_NO_SHORT_NAMES` before including `handy_option.h` and use the
fully-qualified `hs_option_*` API.

## Generator + tools

- `tools/generate_handle_array_header.py` — per-type handle pool/array
  generator. See `tools/README.md`.
- `tools/amalgamate.py` — emit a single-header (STB-style) build of the
  entire library:

  ```sh
  python tools/amalgamate.py --out-dir amalgamated
  # then in your project:
  #   #include "handy_all.h"                     (everywhere)
  #   #define HANDY_IMPLEMENTATION               (in exactly one .c)
  #   #include "handy_all.h"
  ```

  Pass `--two-files` for the legacy `handy_all.h` + `handy_all.c` split.

- `tools/generate_container_header.py` — emit a type-specific container
  header (`vec`, `ring`, `map_u64`, or `map_str`):

  ```sh
  python tools/generate_container_header.py \
      --kind vec --type-name int32 --value-type int32_t \
      --output include/handy_generated/int32_vec.h
  ```

- `tools/generate_compile_commands.py` — produce `compile_commands.json`
  for clangd/clang-tidy when you use a unity build:

  ```sh
  python tools/generate_compile_commands.py \
      --unity src/all.c -- gcc -std=c99 -Iinclude
  ```

## Versioning

Semantic. Current version `0.1.0` — the library is pre-1.0 and APIs may still
shift. See [`CHANGELOG.md`](CHANGELOG.md).

## License

MIT. See [`LICENSE`](LICENSE).
