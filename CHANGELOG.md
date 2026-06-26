# Changelog

All notable changes to this project are documented here.

## [0.1.0] - 2025

Initial library layout.

### Added
- Public headers under `include/handy/`:
  - `handy.h` umbrella header
  - `handy_config.h` central toggles (`HANDY_STUFF_DEBUG`, `RUNTIME_MEMSAFE`,
    `HANDY_HAVE_STDIO`, `HANDY_HAVE_MALLOC`) and portability macros
    (`HANDY_BEGIN_DECLS`, `HS_NODISCARD`, `HS_NORETURN`, `HS_UNUSED`)
  - `handy_version.h`
  - `handy_safety.h` (`HS_ASSERT`, `HS_FAIL`, `hs_status`, `HS_FOR_BOUNDED`).
    Only depends on `<stddef.h>` (and `<stdio.h>`/`<stdlib.h>` when
    `RUNTIME_MEMSAFE && HANDY_HAVE_STDIO`).
  - `handy_mem.h` (`hs_memcpy_s`/`hs_memmove_s`/`hs_memset_s`).
  - `handy_static.h` (compile-time asserts, `HS_ARRAY_COUNT`, `HS_OFFSET_OF`,
    `HS_ALIGN_OF`); static-assert macro folds in `__LINE__` to avoid
    typedef-name collisions.
  - `handy_assert.h`, `handy_contract.h`
  - `handy_bits.h` (u8/u16/u32/u64 + i8/i16/i32/i64 helpers, no `stdbool.h`)
  - `handy_checked.h` overflow-checked add/sub/mul (u32/u64/size/i32/i64),
    using `__builtin_*_overflow` where available.
  - `handy_option.h` (no standard-library includes; tag is `unsigned char`.
    Short names opt-out via `HANDY_OPTION_NO_SHORT_NAMES`)
  - `handy_str.h` non-owning string slice + slice/trim/split/find helpers.
  - `handy_arena.h` bump allocator (user-backed or heap-backed).
  - `handy_strbuilder.h` growable string buffer backed by an arena.
  - `handy_time.h` `hs_now_ns` / `hs_sleep_ms` (Windows + POSIX).
  - `handy_log.h`
- `src/handy_log.c` (the library's only compiled unit)
- `tools/generate_handle_array_header.py` + example config
- `tools/generate_container_header.py` — type-specific `vec`/`ring`/`map_u64`/`map_str`
- `tools/amalgamate.py` — single-file `handy_all.h` + `handy_all.c` emitter
- `tools/generate_compile_commands.py` — `compile_commands.json` emitter for
  unity builds
- `tests/handy_test.h` micro test harness and `tests/test_basics.c`
- `examples/example_handle_array.c`
- `CMakeLists.txt` with `STATIC` target, install rules and CTest integration
- `.clang-format`, `.editorconfig`, `.gitignore`
