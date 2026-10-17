#!/usr/bin/env python3
"""Checks that no global of port/game/ is initialised from a global another file defines.

    python3 port/tools/refactor/init_order.py     # lists them; exit status 1 if there are any

C++ leaves the order in which files run their static initialisers open: GCC runs them in
link order, and a compiler may also fold an initialiser into constant data. A global
initialised from another file's global can therefore see it still zero, depending on the
compiler and the order of the files on the link line. That happened to kFpEight_6FED48 (0 in
GCC builds, 8 in Clang ones and in the original) and gDummyH_678584. Keep every initialiser
to its own file's globals, defined above it.

It reads the DEFINE_GLOBAL_INIT and DEFINE_GLOBAL_ARRAY_INIT definitions (the game's way of
defining globals). Globals defined otherwise, and constructors that read globals, are not
checked.
"""
import pathlib
import re
import sys

GAME = pathlib.Path(__file__).resolve().parents[2] / "game"
DEFINE = re.compile(r"^\s*DEFINE_GLOBAL(_ARRAY)?(_INIT)?\s*\(", re.M)
IDENT = re.compile(r"[A-Za-z_]\w*")


def split_args(text):
    """Splits a macro's arguments at the top-level commas."""
    args, depth, current = [], 0, ""
    for c in text:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == "," and depth == 0:
            args.append(current.strip())
            current = ""
        else:
            current += c
    args.append(current.strip())
    return args


def definitions():
    """Yields (file, line, name, initialiser or None) for every DEFINE_GLOBAL* in the game."""
    for path in sorted(GAME.rglob("*.cpp")):
        text = path.read_text(errors="replace")
        for m in DEFINE.finditer(text):
            depth, end = 1, m.end()
            while depth:
                depth += {"(": 1, ")": -1}.get(text[end], 0)
                end += 1
            args = split_args(text[m.end() : end - 1])
            if len(args) < 3:
                continue
            is_array, is_init = bool(m.group(1)), bool(m.group(2))
            init = None
            if is_init:
                # (type, name, value, addr) or (type, name, size, addr, values)
                init = " , ".join(args[4:]) if is_array else " , ".join(args[2:-1])
            yield path.name, text.count("\n", 0, m.start()) + 1, args[1], init


def main():
    defs = list(definitions())
    owner = {name: file for file, _, name, _ in defs}
    found = 0
    for file, line, name, init in defs:
        if init is None:
            continue
        others = sorted({i for i in IDENT.findall(init) if owner.get(i, file) != file})
        for other in others:
            found += 1
            print(f"port/game/{file}:{line}: {name} is initialised from {other} ({owner[other]})")
    print(f"{found} initialiser(s) read another file's global", file=sys.stderr)
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
