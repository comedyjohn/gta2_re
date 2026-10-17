#!/usr/bin/env python3
"""Which functions read and write each global of port/game (port/refactor/2-names.md 2.5).

    python3 port/tools/refactor/global_users.py [build_dir] [-o global_users.csv]
    python3 port/tools/refactor/global_users.py --show gGame_0x40_67E008
    python3 port/tools/refactor/global_users.py --selftest

Parses every translation unit with libclang and, for each reference to a namespace-scope variable,
records the function it sits in and how it is used:

    write   the target of `=`, `+=`, `++`, ... (also through `g[i]`, `g.field`, `g->field`)
    addr    `&g`, or `g` bound to a non-const reference or pointer parameter (may be written elsewhere)
    method  a member function called on the global (`g.Update()`: may write it)
    read    anything else

CSV columns: `global,file,reads,writes,addr,methods,functions` (the function lists are `;`-separated
and kept per class of use in `--show`). A global used in one function only, and only written before it
is read, can become a local (phase 2.5's procedure); the tool lists the facts, the decision is made by
reading the code.
"""
import argparse
import collections
import concurrent.futures
import csv
import json
import os
import shlex
import subprocess
import sys

from clang import cindex

K = cindex.CursorKind
ROOT = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GAME = os.path.join(ROOT, "port", "game") + os.sep
ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
PASS_THROUGH = {K.ARRAY_SUBSCRIPT_EXPR, K.MEMBER_REF_EXPR, K.PAREN_EXPR, K.UNEXPOSED_EXPR, K.CSTYLE_CAST_EXPR,
                K.CXX_STATIC_CAST_EXPR, K.CXX_REINTERPRET_CAST_EXPR}


def function_name(cur):
    parts = []
    c = cur
    while c is not None and c.kind != K.TRANSLATION_UNIT:
        if c.spelling and not c.spelling.startswith("("):
            parts.append(c.spelling)
        c = c.semantic_parent
    return "::".join(reversed(parts))


def operator_of(parent, first_child):
    """The operator token of a binary or compound-assignment operator (the one after the left operand)."""
    end = first_child.extent.end.offset
    for t in parent.get_tokens():
        if t.extent.start.offset >= end and t.kind == cindex.TokenKind.PUNCTUATION:
            return t.spelling
    return ""


def classify(stack, ref):
    """How the reference at the top of the chain `stack` (ancestors, nearest last) uses the global."""
    child = ref
    for parent in reversed(stack):
        k = parent.kind
        if k in (K.BINARY_OPERATOR, K.COMPOUND_ASSIGNMENT_OPERATOR):
            kids = list(parent.get_children())
            if kids and kids[0] == child or (kids and kids[0].extent == child.extent):
                if operator_of(parent, kids[0]) in ASSIGN_OPS:
                    return "write"
            return "read"
        if k == K.UNARY_OPERATOR:
            toks = [t.spelling for t in parent.get_tokens()]
            if toks and (toks[0] in ("++", "--") or toks[-1] in ("++", "--")):
                return "write"
            if toks and toks[0] == "&":
                return "addr"
            return "read"
        if k == K.CALL_EXPR:
            # g.Method(): the global is the object of a member call; g bound to a parameter: addr if non-const
            callee = parent.referenced
            if callee is not None and callee.kind in (K.CXX_METHOD, K.CONSTRUCTOR, K.DESTRUCTOR):
                kids = list(parent.get_children())
                if kids and (kids[0] == child or kids[0].extent == child.extent):
                    return "method" if not callee.is_const_method() else "read"
            if callee is not None:
                args = list(callee.get_arguments())
                kids = [c for c in parent.get_children() if c.kind != K.UNEXPOSED_EXPR or True]
                for i, a in enumerate(args):
                    t = a.type
                    if t.kind in (cindex.TypeKind.LVALUEREFERENCE, cindex.TypeKind.POINTER) and not t.get_pointee().is_const_qualified():
                        if i + 1 < len(kids) and kids[i + 1].extent == child.extent:
                            return "addr"
            return "read"
        if k in PASS_THROUGH:
            child = parent
            continue
        return "read"
    return "read"


def scan_tu(args):
    path, argv = args
    tu = cindex.Index.create().parse(path, args=argv)
    if any(d.severity >= cindex.Diagnostic.Error for d in tu.diagnostics):
        return path, None
    uses = set()  # (global, function, how)
    defs = {}

    def visit(cur, stack, func):
        f = cur.location.file
        if f is not None and not f.name.startswith(GAME):
            return
        if cur.kind in (K.FUNCTION_DECL, K.CXX_METHOD, K.CONSTRUCTOR, K.DESTRUCTOR) and cur.is_definition():
            func = function_name(cur)
        if cur.kind == K.VAR_DECL and cur.semantic_parent.kind in (K.TRANSLATION_UNIT, K.NAMESPACE, K.LINKAGE_SPEC):
            if f is not None:
                defs[cur.spelling] = os.path.relpath(f.name, GAME)
        if cur.kind == K.DECL_REF_EXPR and f is not None:
            ref = cur.referenced
            if ref is not None and ref.kind == K.VAR_DECL and ref.semantic_parent.kind in (K.TRANSLATION_UNIT, K.NAMESPACE, K.LINKAGE_SPEC):
                if not ref.storage_class == cindex.StorageClass.STATIC or True:
                    uses.add((ref.spelling, func or "(initialiser)", classify(stack, cur)))
        for child in cur.get_children():
            visit(child, stack + [cur], func)

    for top in tu.cursor.get_children():
        if top.location.file is not None and top.location.file.name.startswith(GAME):
            visit(top, [], "")
    return path, (sorted(uses), defs)


def commands(build_dir):
    resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
    for e in json.load(open(os.path.join(build_dir, "compile_commands.json"))):
        if not e["file"].startswith(GAME) or not e["file"].endswith(".cpp"):
            continue
        argv = e.get("arguments") or shlex.split(e["command"])
        out, skip = [], False
        for a in argv[1:]:
            if skip:
                skip = False
            elif a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif a in ("-c", "-MD", "-MMD") or a == e["file"]:
                pass
            else:
                out.append(a)
        yield e["file"], out + ["-resource-dir", resource, "-working-directory", e["directory"]]


def gather(build_dir, jobs):
    uses = collections.defaultdict(lambda: collections.defaultdict(set))  # global -> how -> functions
    defs = {}
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, res in pool.map(scan_tu, list(commands(build_dir))):
            if res is None:
                sys.exit(f"{path} does not parse")
            for g, fn, how in res[0]:
                uses[g][how].add(fn)
            defs.update(res[1])
    return uses, defs


def write_csv(uses, defs, path):
    with open(path, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["global", "file", "reads", "writes", "addr", "methods", "functions"])
        for g in sorted(uses):
            u = uses[g]
            funcs = set().union(*u.values())
            w.writerow([g, defs.get(g, ""), len(u["read"]), len(u["write"]), len(u["addr"]), len(u["method"]),
                        ";".join(sorted(funcs))])


def show(uses, name):
    hits = [g for g in uses if g == name]
    if not hits:
        print(f"{name}: not used")
        return 1
    for how in ("write", "addr", "method", "read"):
        fns = sorted(uses[name][how])
        if fns:
            print(f"{how:7} ({len(fns)}): " + ", ".join(fns))
    return 0


def selftest():
    import tempfile

    src = """
struct S { int a; void Set() { a = 1; } int Get() const { return a; } };
int gCounter; S gS; int gArr[4];
void Take(int& r); void Peek(const int& r);
void Writer() { gCounter = 1; gCounter++; gArr[2] = 3; gS.a = 4; }
void Reader() { int x = gCounter + gArr[1] + gS.a; Peek(gCounter); (void)x; }
void Mixed() { gS.Set(); int y = gS.Get(); Take(gCounter); int* p = &gArr[0]; (void)y; (void)p; }
"""
    with tempfile.TemporaryDirectory() as d:
        global GAME
        GAME = d + os.sep
        p = os.path.join(d, "t.cpp")
        open(p, "w").write(src)
        resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
        _, (found, _) = scan_tu((p, ["-std=c++17", "-resource-dir", resource]))
    got = {(g, fn): set() for g, fn, h in found}
    for g, fn, h in found:
        got[(g, fn)].add(h)
    assert got[("gCounter", "Writer")] == {"write"}, got
    assert got[("gArr", "Writer")] == {"write"}
    assert got[("gS", "Writer")] == {"write"}
    assert got[("gCounter", "Reader")] == {"read"}, got
    assert got[("gS", "Reader")] == {"read"}
    assert got[("gS", "Mixed")] == {"method", "read"}, got
    assert got[("gCounter", "Mixed")] == {"addr"}, got
    assert got[("gArr", "Mixed")] == {"addr"}, got
    print("selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("build_dir", nargs="?", default=os.path.join(ROOT, "build_port", "clang64"))
    ap.add_argument("-o", "--output")
    ap.add_argument("--show")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("-j", type=int, default=6)
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    uses, defs = gather(os.path.realpath(a.build_dir), a.j)
    if a.show:
        return show(uses, a.show)
    write_csv(uses, defs, a.output or os.path.join(ROOT, "build_port", "global_users.csv"))
    print(f"{len(uses)} globals used", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
