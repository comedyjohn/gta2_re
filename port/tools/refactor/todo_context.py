#!/usr/bin/env python3
"""Shows what a person needs to name the symbols that propose.py could not (the TODO rows).

    python3 port/tools/refactor/todo_context.py SUBSYSTEM KIND [--lines N] [--uses N]

For each TODO: the declaration line, for a function its first lines of body, and a few lines where the name is used.
"""
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.realpath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, HERE)
import propose  # noqa: E402


def arg(name, default):
    return int(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default


def main():
    sub, kind = sys.argv[1], sys.argv[2]
    nlines, nuses = arg("--lines", 14), arg("--uses", 4)
    syms = json.load(open(propose.SYMBOLS))
    by_key = {}
    for s in syms:
        by_key.setdefault(propose.key_of(s), s)
    rows = [r for r in propose.propose(sub, kind, syms) if not r[1]]
    for key, _, why, name in rows:
        s = by_key[key]
        path = os.path.join(ROOT, "port", "game", s["file"])
        lines = open(path, errors="replace").read().split("\n")
        i = s["line"] - 1
        print(f"### {name}  [{why}]  {s['file']}:{s['line']} ({s['kind']})")
        end = min(len(lines), i + (nlines if kind == "functions" else 2))
        print("\n".join(lines[max(0, i - 2) : end]))
        uses = subprocess.run(["grep", "-rnw", "--include=*.cpp", "--include=*.hpp", "-m", "2", s["name"],
                               os.path.join(ROOT, "port", "game")], capture_output=True, text=True).stdout.split("\n")
        uses = [u.replace(ROOT + "/port/game/", "") for u in uses if u and f"{s['file']}:{s['line']}:" not in u][:nuses]
        if uses:
            print("  -- used:")
            for u in uses:
                print("  " + u[:200])
        print()


if __name__ == "__main__":
    main()
