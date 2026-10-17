#!/usr/bin/env python3
"""Is a subsystem finished with the naming step? (the loop exit check of EXECUTOR.md R2.x)

    python3 port/tools/refactor/names_done.py SUBSYSTEM [--scan]

Counts, per kind, the symbols of the subsystem that propose.py would still rename (symbols.json, refreshed
with --scan), and the lines of its directory that still carry an original address or placeholder outside
comments. Exit status 0 if both are 0.
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

KINDS = ["types", "functions", "members", "enumerators", "globals", "params", "locals"]
PLACEHOLDER = re.compile(r"_[0-9A-F]{6}\b|\bfield_[0-9A-Fa-f]+|\bsub_[0-9A-Fa-f]{6}|\b(?:byte|word|dword|unk|flt|off|stru)_[0-9A-Fa-f]{6}")


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in propose.SUBSYSTEMS:
        sys.exit(__doc__)
    sub = sys.argv[1]
    if "--scan" in sys.argv:
        subprocess.check_call([sys.executable, os.path.join(HERE, "symbols.py"), os.path.join(ROOT, "build_port", "clang64")])
    syms = json.load(open(propose.SYMBOLS))
    bad = 0
    for kind in KINDS:
        n = len(propose.propose(sub, kind, syms))
        bad += n
        print(f"{kind:12} {n}")
    lines = 0
    for dirpath, _, names in os.walk(os.path.join(ROOT, "port", "game", sub)):
        for n in names:
            if not n.endswith((".cpp", ".hpp", ".h", ".inl")):
                continue
            for i, line in enumerate(open(os.path.join(dirpath, n), errors="replace"), 1):
                code = line.split("//")[0]
                if PLACEHOLDER.search(code):
                    lines += 1
    print(f"{'placeholders':12} {lines} lines in port/game/{sub}/ (names of other subsystems count until they are renamed)")
    return 0 if bad == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
