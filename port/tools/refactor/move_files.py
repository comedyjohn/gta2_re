#!/usr/bin/env python3
"""Moves the files of a subsystem into port/game/<subsystem>/ under snake_case names (2-names.md 2.6).

    python3 port/tools/refactor/move_files.py SUBSYSTEM            # prints the plan
    python3 port/tools/refactor/move_files.py SUBSYSTEM --apply    # git mv + every reference

The new name is the snake_case of the file's main type (`Fix16_Point.cpp` holds Fix16Point, so it becomes
`fix16_point.cpp`); a file without such a type keeps its stem in snake_case with the size suffix cut
(`ang16`). port/refactor/names/SUBSYSTEM-files.overrides.csv (`old stem,new stem`) decides the rest. With
--apply it also rewrites the `#include "..."` lines of all of port/ (one include root, port/game/, so
`#include "core/fix16.hpp"`), the stems in subsystems.py, and the file paths in address_map.csv and
tidy-baseline.csv. A move changes no code: check.sh must still be green, and the commit is only moves.
"""
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.realpath(os.path.join(HERE, "..", "..", ".."))
GAME = os.path.join(ROOT, "port", "game")
sys.path.insert(0, HERE)
from propose import snake  # noqa: E402
from subsystems import SUBSYSTEMS  # noqa: E402

EXTS = (".cpp", ".hpp", ".h", ".inl")
SIZE = re.compile(r"_(?:0x[0-9A-Fa-f]+|[0-9A-F]{1,5})$")


def current_files(stem):
    """The files with this stem under port/game (anywhere below it, but not 3rdParty)."""
    out = []
    for dirpath, dirs, names in os.walk(GAME):
        dirs[:] = [d for d in dirs if d != "3rdParty"]
        for n in names:
            if n.endswith(EXTS) and os.path.splitext(n)[0] == stem:
                out.append(os.path.relpath(os.path.join(dirpath, n), GAME))
    return sorted(out)


def plan(sub):
    types = {}
    with open(os.path.join(HERE, "address_map.csv"), newline="") as f:
        for r in csv.DictReader(f):
            if r["kind"] in ("type",) and not r["owner"]:
                stem = os.path.splitext(os.path.basename(r["file"]))[0]
                types.setdefault(stem, []).append(r)
    overrides = {}
    ov = os.path.join(ROOT, "port", "refactor", "names", f"{sub}-files.overrides.csv")
    if os.path.exists(ov):
        with open(ov, newline="") as f:
            for r in csv.reader(f):
                if r and r[0] != "old":
                    overrides[r[0]] = r[1]
    moves = []
    used = set()
    for stem in sorted(SUBSYSTEMS[sub]):
        files = current_files(stem)
        if not files:
            continue
        if stem in overrides:
            new_stem = overrides[stem]
        else:
            main = [t for t in types.get(stem, []) if t["old_name"] == stem or t["old_name"].startswith(stem)]
            if main:
                name = main[0]["new_name"] or main[0]["old_name"]
                new_stem = snake(name) if not re.fullmatch(r"[a-z0-9_]+", name) else name
            else:
                new_stem = snake(SIZE.sub("", stem)) if not re.fullmatch(r"[a-z0-9_]+", stem) else SIZE.sub("", stem)
        if new_stem in used:
            sys.exit(f"two files of {sub} would be called {new_stem}: add an override for {stem}")
        used.add(new_stem)
        for f in files:
            ext = os.path.splitext(f)[1]
            moves.append((f, f"{sub}/{new_stem}{ext}"))
    return moves


def rewrite_includes(mapping):
    """mapping: old relative path (under port/game) -> new one. Rewrites `#include "x"` everywhere."""
    by_base = {}
    for old, new in mapping.items():
        by_base.setdefault(os.path.basename(old), []).append((old, new))
    pat = re.compile(r'^([ \t]*#[ \t]*include[ \t]*")([^"\n]+)(")', re.M)
    changed = 0
    for dirpath, dirs, names in os.walk(os.path.join(ROOT, "port")):
        dirs[:] = [d for d in dirs if d not in ("upstream",)]
        for n in names:
            if not n.endswith(EXTS + (".c",)):
                continue
            path = os.path.join(dirpath, n)
            text = open(path, encoding="latin-1", newline="").read()

            def sub(m):
                inc = m.group(2)
                cands = by_base.get(os.path.basename(inc), [])
                for old, new in cands:
                    if inc == old or inc == os.path.basename(old):
                        return m.group(1) + new + m.group(3)
                return m.group(0)

            new_text = pat.sub(sub, text)
            if new_text != text:
                open(path, "w", encoding="latin-1", newline="").write(new_text)
                changed += 1
    return changed


def rewrite_paths_in_csv(path, column, mapping):
    with open(path, newline="") as f:
        rows = list(csv.reader(f))
    for r in rows[1:]:
        base = r[column]
        for prefix in ("port/game/", ""):
            key = base[len(prefix):] if prefix and base.startswith(prefix) else (base if not prefix else None)
            if key in mapping:
                r[column] = prefix + mapping[key]
                break
    with open(path, "w", newline="") as f:
        csv.writer(f, lineterminator="\n").writerows(rows)


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in SUBSYSTEMS:
        sys.exit(__doc__)
    sub = sys.argv[1]
    moves = plan(sub)
    for old, new in moves:
        print(f"{old} -> {new}")
    if "--apply" not in sys.argv:
        return 0
    mapping = dict(moves)
    for old, new in moves:
        os.makedirs(os.path.dirname(os.path.join(GAME, new)), exist_ok=True)
        subprocess.check_call(["git", "mv", os.path.join("port", "game", old), os.path.join("port", "game", new)], cwd=ROOT)
    n = rewrite_includes(mapping)
    # subsystems.py: stems
    p = os.path.join(HERE, "subsystems.py")
    text = open(p).read()
    for old, new in moves:
        so, sn = os.path.splitext(os.path.basename(old))[0], os.path.splitext(os.path.basename(new))[0]
        if so != sn:
            text = re.sub(r'"' + re.escape(so) + r'"', '"' + sn + '"', text)
    open(p, "w").write(text)
    rewrite_paths_in_csv(os.path.join(HERE, "address_map.csv"), 8, mapping)
    rewrite_paths_in_csv(os.path.join(ROOT, "port", "refactor", "tidy-baseline.csv"), 0, mapping)
    print(f"{len(moves)} files moved, includes rewritten in {n} files", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
