#!/usr/bin/env python3
"""
amalgamate.py - emit single-file `handy_all.h` + `handy_all.c` from the
public headers and the lone src/handy_log.c source.

Usage:
    python tools/amalgamate.py --out-dir amalgamated

Produces:
    <out_dir>/handy_all.h
    <out_dir>/handy_all.c

The result drops into any project: just compile handy_all.c and include
handy_all.h. No build system required.
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
GUARD_RE = re.compile(r'^\s*#\s*(?:ifndef|define|endif)\b.*$')


def strip_handy_includes(text: str) -> str:
    """Drop `#include "handy/..."` lines; we are inlining them."""
    return "\n".join(
        line for line in text.splitlines() if not HANDY_INCLUDE_RE.match(line)
    ) + "\n"


def strip_header_guard(text: str) -> str:
    """Remove the outermost #ifndef/#define ... #endif guard so the merged
    file has exactly one guard."""
    lines = text.splitlines()
    # Find first ifndef
    start = None
    for i, line in enumerate(lines):
        if re.match(r"^\s*#\s*ifndef\s+\w+", line):
            start = i
            break
    if start is None:
        return text
    if start + 1 >= len(lines) or not re.match(r"^\s*#\s*define\s+\w+", lines[start + 1]):
        return text
    # Find matching closing #endif (the last one in the file).
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


def build_header() -> str:
    parts = [
        "/* handy_all.h - amalgamated single-header build of the handy library. */\n",
        "#ifndef HANDY_ALL_H\n#define HANDY_ALL_H\n",
    ]
    for name in HEADER_ORDER:
        path = INCLUDE_DIR / name
        text = path.read_text(encoding="utf-8")
        text = strip_handy_includes(text)
        text = strip_header_guard(text)
        parts.append(banner(name))
        parts.append(text)
    parts.append("\n#endif /* HANDY_ALL_H */\n")
    return "".join(parts)


def build_source(header_name: str) -> str:
    parts = [
        "/* handy_all.c - amalgamated implementation. */\n",
        f'#include "{header_name}"\n',
    ]
    for c in sorted(SRC_DIR.glob("*.c")):
        text = c.read_text(encoding="utf-8")
        text = strip_handy_includes(text)
        parts.append(banner(c.name))
        parts.append(text)
    return "".join(parts)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out-dir", default="amalgamated", type=Path)
    ap.add_argument("--header-name", default="handy_all.h")
    ap.add_argument("--source-name", default="handy_all.c")
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    (args.out_dir / args.header_name).write_text(build_header(), encoding="utf-8")
    (args.out_dir / args.source_name).write_text(
        build_source(args.header_name), encoding="utf-8"
    )
    print(f"wrote {args.out_dir / args.header_name}")
    print(f"wrote {args.out_dir / args.source_name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
