#!/usr/bin/env python3
"""
amalgamate.py - emit a single-header (STB-style) build of the handy library.

By default produces ONE file: `handy_all.h`. Use it like stb_image.h —
include it anywhere for declarations, and in exactly one translation unit
define `HANDY_IMPLEMENTATION` before the include to compile the bodies:

    // in every TU that uses handy:
    #include "handy_all.h"

    // in exactly one TU, e.g. handy_impl.c:
    #define HANDY_IMPLEMENTATION
    #include "handy_all.h"

Pass `--two-files` to also emit a sidecar `handy_all.c` (legacy layout).

Usage:
    python tools/amalgamate.py --out-dir amalgamated
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
INCLUDE_DIR = REPO_ROOT / "include" / "handy"
SRC_DIR = REPO_ROOT / "src"

# Order matters: anything depended on must come first.
HEADER_ORDER = [
    "handy_config.h",
    "handy_version.h",
    "handy_safety.h",
    "handy_mem.h",
    "handy_static.h",
    "handy_assert.h",
    "handy_contract.h",
    "handy_bits.h",
    "handy_checked.h",
    "handy_option.h",
    "handy_str.h",
    "handy_arena.h",
    "handy_strbuilder.h",
    "handy_time.h",
    "handy_log.h",
]

HANDY_INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"handy/[^"]+"\s*$')


def strip_handy_includes(text: str) -> str:
    """Drop `#include "handy/..."` lines; we are inlining them."""
    return "\n".join(
        line for line in text.splitlines() if not HANDY_INCLUDE_RE.match(line)
    ) + "\n"


def strip_header_guard(text: str) -> str:
    """Remove the outermost #ifndef/#define ... #endif guard so the merged
    file has exactly one guard."""
    lines = text.splitlines()
    start = None
    for i, line in enumerate(lines):
        if re.match(r"^\s*#\s*ifndef\s+\w+", line):
            start = i
            break
    if start is None:
        return text
    if start + 1 >= len(lines) or not re.match(r"^\s*#\s*define\s+\w+", lines[start + 1]):
        return text
    end = None
    for i in range(len(lines) - 1, -1, -1):
        if re.match(r"^\s*#\s*endif\b", lines[i]):
            end = i
            break
    if end is None or end <= start + 1:
        return text
    return "\n".join(lines[start + 2 : end]) + "\n"


def banner(name: str) -> str:
    return f"\n/* ===== {name} ===== */\n"


def render_headers() -> str:
    parts = []
    for name in HEADER_ORDER:
        text = (INCLUDE_DIR / name).read_text(encoding="utf-8")
        text = strip_handy_includes(text)
        text = strip_header_guard(text)
        parts.append(banner(name))
        parts.append(text)
    return "".join(parts)


def render_sources() -> str:
    parts = []
    for c in sorted(SRC_DIR.glob("*.c")):
        text = c.read_text(encoding="utf-8")
        text = strip_handy_includes(text)
        parts.append(banner(c.name))
        parts.append(text)
    return "".join(parts)


def build_single_header() -> str:
    return "".join([
        "/* handy_all.h - amalgamated single-header build of the handy library.\n",
        " *\n",
        " * STB-style: include this header anywhere for declarations. In exactly\n",
        " * one translation unit, #define HANDY_IMPLEMENTATION before including it\n",
        " * to compile the implementation bodies.\n",
        " */\n",
        "#ifndef HANDY_ALL_H\n",
        "#define HANDY_ALL_H\n",
        render_headers(),
        "\n#endif /* HANDY_ALL_H */\n",
        "\n",
        "#ifdef HANDY_IMPLEMENTATION\n",
        "#ifndef HANDY_ALL_IMPLEMENTATION_DEFINED\n",
        "#define HANDY_ALL_IMPLEMENTATION_DEFINED\n",
        render_sources(),
        "#endif /* HANDY_ALL_IMPLEMENTATION_DEFINED */\n",
        "#endif /* HANDY_IMPLEMENTATION */\n",
    ])


def build_legacy_header() -> str:
    return "".join([
        "/* handy_all.h - amalgamated declarations (legacy two-file layout). */\n",
        "#ifndef HANDY_ALL_H\n#define HANDY_ALL_H\n",
        render_headers(),
        "\n#endif /* HANDY_ALL_H */\n",
    ])


def build_legacy_source(header_name: str) -> str:
    return "".join([
        "/* handy_all.c - amalgamated implementation (legacy two-file layout). */\n",
        f'#include "{header_name}"\n',
        render_sources(),
    ])


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out-dir", default="amalgamated", type=Path)
    ap.add_argument("--header-name", default="handy_all.h")
    ap.add_argument("--source-name", default="handy_all.c")
    ap.add_argument("--two-files", action="store_true",
                    help="Emit a legacy two-file split (handy_all.h + handy_all.c) "
                         "instead of the default single-header (STB) form.")
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    header_path = args.out_dir / args.header_name

    if args.two_files:
        header_path.write_text(build_legacy_header(), encoding="utf-8")
        source_path = args.out_dir / args.source_name
        source_path.write_text(build_legacy_source(args.header_name), encoding="utf-8")
        print(f"wrote {header_path}")
        print(f"wrote {source_path}")
    else:
        header_path.write_text(build_single_header(), encoding="utf-8")
        print(f"wrote {header_path}")
        print('To compile bodies: in exactly one .c, '
              '`#define HANDY_IMPLEMENTATION` before `#include "handy_all.h"`.')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

