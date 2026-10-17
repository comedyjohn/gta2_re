#!/usr/bin/env python3
"""Moves file-local `static` functions and variables into an anonymous namespace.

    python3 port/tools/refactor/anon_namespace.py build_port/clang64 port/game/Bink.cpp [...]

misc-use-anonymous-namespace has no fix-its in LLVM 18. For each namespace-scope declaration of the
given .cpp files that has static storage class, this removes the `static` keyword and wraps the
declaration in `namespace { ... }`; neighbours with only white space between them share one block.
Declarations inside a class or a function, and member-function definitions, are left alone;
`extern "C" { }` and named namespaces are looked into. Needs the libclang of the refactor venv (EXECUTOR.md 1.2).
"""
import json
import os
import re
import shlex
import subprocess
import sys

from clang import cindex

K = cindex.CursorKind


def args_for(build_dir, path):
    resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
    for e in json.load(open(os.path.join(build_dir, "compile_commands.json"))):
        if os.path.realpath(e["file"]) == path:
            argv = e.get("arguments") or shlex.split(e["command"])
            out, skip = [], False
            for a in argv[1:]:
                if skip:
                    skip = False
                elif a in ("-o", "-MF", "-MT", "-MQ"):
                    skip = True
                elif a in ("-c", "-MD", "-MMD") or a.endswith(".cpp"):
                    pass
                else:
                    out.append(a)
            return out + ["-resource-dir", resource, "-working-directory", e["directory"]]
    sys.exit(f"{path} is not in the compile database")


def edits_for(build_dir, path):
    tu = cindex.Index.create().parse(path, args=args_for(build_dir, path))
    if any(d.severity >= cindex.Diagnostic.Error for d in tu.diagnostics):
        sys.exit(f"{path}: does not parse")
    text = open(path, newline="").read()
    decls = []
    def top_level(cur):
        for c in cur.get_children():
            if c.kind in (K.LINKAGE_SPEC, K.NAMESPACE) and c.location.file is not None:
                yield from top_level(c)  # extern "C" { ... }
            else:
                yield c

    for c in top_level(tu.cursor):
        if c.location.file is None or os.path.realpath(c.location.file.name) != path:
            continue
        if c.kind not in (K.FUNCTION_DECL, K.VAR_DECL):
            continue
        if c.storage_class != cindex.StorageClass.STATIC:
            continue
        if c.semantic_parent.kind not in (K.TRANSLATION_UNIT, K.LINKAGE_SPEC, K.NAMESPACE):
            continue
        start, end = c.extent.start.offset, c.extent.end.offset
        if c.kind == K.VAR_DECL or not text[start:end].rstrip().endswith("}"):
            m = re.compile(r"\s*;").match(text, end)
            if m:
                end = m.end()
        # the `static` keyword is among the first tokens of the declaration
        m = re.compile(r"\bstatic\b[ \t]*").search(text, start, end)
        if not m:
            continue
        decls.append((start, end, m.start(), m.end()))
    # drop `static`, then wrap runs of neighbours
    blocks = []
    for start, end, ks, ke in sorted(decls):
        if blocks and not text[blocks[-1][1] : start].strip():
            blocks[-1][1] = end
            blocks[-1][2].append((ks, ke))
        else:
            blocks.append([start, end, [(ks, ke)]])
    edits = []
    for start, end, keywords in blocks:
        edits.append((end, end, "\n} // namespace\n"))
        for ks, ke in keywords:
            edits.append((ks, ke, ""))
        edits.append((start, start, "namespace {\n"))
    return text, edits


def main():
    build_dir = os.path.realpath(sys.argv[1])
    for p in sys.argv[2:]:
        path = os.path.realpath(p)
        text, edits = edits_for(build_dir, path)
        for s, e, new in sorted(edits, key=lambda x: (x[0], x[1]), reverse=True):
            text = text[:s] + new + text[e:]
        open(path, "w", newline="").write(text)
        print(f"{p}: {sum(1 for e in edits if e[2].startswith('namespace'))} namespace blocks")


if __name__ == "__main__":
    main()
