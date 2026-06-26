#!/usr/bin/env python3
"""
generate_compile_commands.py - emit a compile_commands.json for unity
builds (or any project where the real compile invocation lists only one
file but you want clangd to understand all of them).

Two modes:

1) Unity mode: you compile a single "all.c" that #includes everything.
   We point clangd at that single command but materialize one entry per
   #include'd .c file so jump-to-def, diagnostics, and refactors work in
   all of them.

       python tools/generate_compile_commands.py \
           --unity src/all.c -- gcc -std=c99 -Iinclude

2) List mode: you give us .c files explicitly.

       python tools/generate_compile_commands.py \
           --files src/a.c src/b.c -- gcc -std=c99 -Iinclude

In both modes everything after `--` is the raw compiler command minus the
input file (which we splice per-entry).

Writes compile_commands.json into --out-dir (default: cwd).
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+\.c)"\s*$', re.MULTILINE)


def discover_unity_inputs(unity_path: Path) -> list[Path]:
    """Walk #include "..." directives recursively to find every .c TU rolled
    into a unity build."""
    seen: set[Path] = set()
    pending: list[Path] = [unity_path.resolve()]
    while pending:
        cur = pending.pop()
        if cur in seen or not cur.exists():
            continue
        seen.add(cur)
        for m in INCLUDE_RE.finditer(cur.read_text(encoding="utf-8", errors="replace")):
            candidate = (cur.parent / m.group(1)).resolve()
            if candidate.exists() and candidate.suffix == ".c":
                pending.append(candidate)
    return sorted(seen)


def make_entry(directory: Path, source: Path, compiler_args: list[str]) -> dict:
    return {
        "directory": str(directory),
        "file": str(source),
        "arguments": list(compiler_args) + [str(source)],
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--unity", type=Path, help="Path to the unity .c file")
    g.add_argument("--files", nargs="+", type=Path, help="Explicit list of .c files")
    ap.add_argument("--out-dir", type=Path, default=Path.cwd())
    ap.add_argument("compiler_command", nargs=argparse.REMAINDER,
                    help="-- <compiler> <flags...> (input file appended per entry)")
    args = ap.parse_args()

    cmd = args.compiler_command
    if cmd and cmd[0] == "--":
        cmd = cmd[1:]
    if not cmd:
        print("error: provide the compiler command after '--'", file=sys.stderr)
        return 2

    if args.unity:
        inputs = discover_unity_inputs(args.unity)
        if not inputs:
            print(f"error: no .c files discovered from {args.unity}", file=sys.stderr)
            return 2
    else:
        inputs = [p.resolve() for p in args.files]

    cwd = args.out_dir.resolve()
    entries = [make_entry(cwd, src, cmd) for src in inputs]

    out_path = cwd / "compile_commands.json"
    out_path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {out_path} ({len(entries)} entries)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
