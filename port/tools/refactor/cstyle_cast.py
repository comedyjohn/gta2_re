#!/usr/bin/env python3
"""Turns the C-style casts of port/game/ into static_cast / const_cast / reinterpret_cast.

    python3 port/tools/refactor/cstyle_cast.py scan build_port/clang64 edits.json
    python3 port/tools/refactor/cstyle_cast.py apply edits.json [--select REGEX] [--dry-run]
    python3 port/tools/refactor/cstyle_cast.py selftest

`scan` parses every port/game TU of the compile database with libclang (Clang64 configuration),
picks the C++ cast for each CStyleCastExpr from its types and writes the edits (file, offsets,
old text, new text) to a JSON file; it lists the casts it refuses, with the reason. `apply` makes
the edits whose file matches --select, checking that the old text is where it was seen. Casts that
contain another cast are left for the next round: scan again after applying.

The cast is chosen like this (the C cast tries these in order, so does the tool):
    arithmetic / enum / bool / class value / void* <-> T* / pointers of related classes -> static_cast
    the same type with fewer cv-qualifiers                                   -> const_cast
    pointer <-> integer, pointers of unrelated types, references             -> reinterpret_cast
Refused: a cast that also drops const while converting, a cast inside a macro definition or from a
macro argument, a dependent type, anything the rules above do not name. Never defaults to
reinterpret_cast: between related classes it would skip the pointer adjustment the C cast does.
`(void)x` is left alone (the warning ignores it).
"""
import argparse
import collections
import concurrent.futures
import json
import os
import re
import shlex
import subprocess
import sys

from clang import cindex

ROOT = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GAME = os.path.join(ROOT, "port", "game") + os.sep
CV = re.compile(r"\b(const|volatile)\b")
ARITH = {
    cindex.TypeKind.BOOL, cindex.TypeKind.CHAR_U, cindex.TypeKind.UCHAR, cindex.TypeKind.CHAR16,
    cindex.TypeKind.CHAR32, cindex.TypeKind.USHORT, cindex.TypeKind.UINT, cindex.TypeKind.ULONG,
    cindex.TypeKind.ULONGLONG, cindex.TypeKind.UINT128, cindex.TypeKind.CHAR_S, cindex.TypeKind.SCHAR,
    cindex.TypeKind.WCHAR, cindex.TypeKind.SHORT, cindex.TypeKind.INT, cindex.TypeKind.LONG,
    cindex.TypeKind.LONGLONG, cindex.TypeKind.INT128, cindex.TypeKind.FLOAT, cindex.TypeKind.DOUBLE,
    cindex.TypeKind.LONGDOUBLE, cindex.TypeKind.ENUM,
}
INTEGRAL = ARITH - {cindex.TypeKind.FLOAT, cindex.TypeKind.DOUBLE, cindex.TypeKind.LONGDOUBLE}
POINTERS = {cindex.TypeKind.POINTER}
REFS = {cindex.TypeKind.LVALUEREFERENCE, cindex.TypeKind.RVALUEREFERENCE}
ARRAYS = {cindex.TypeKind.CONSTANTARRAY, cindex.TypeKind.INCOMPLETEARRAY}


class Refuse(Exception):
    pass


def cvless(t):
    return " ".join(CV.sub("", t.get_canonical().spelling).split()).replace(" *", "*").replace("* ", "*")


def record_decl(t):
    t = t.get_canonical()
    if t.kind != cindex.TypeKind.RECORD:
        return None
    return t.get_declaration().canonical


def derived_from(derived, base):
    if derived == base:
        return True
    for child in derived.get_children():
        if child.kind == cindex.CursorKind.CXX_BASE_SPECIFIER:
            decl = record_decl(child.type)
            if decl is not None and derived_from(decl, base):
                return True
    return False


def related(a, b):
    da, db = record_decl(a), record_decl(b)
    return da is not None and db is not None and (derived_from(da, db) or derived_from(db, da))


def pointee(t):
    """(pointee type, is_reference) of a pointer, reference or array type; None otherwise."""
    t = t.get_canonical()
    if t.kind in POINTERS:
        return t.get_pointee(), False
    if t.kind in REFS:
        return t.get_pointee(), True
    if t.kind in ARRAYS:
        return t.element_type, False
    return None


def classify(target, operand, is_ref=False):
    """Returns the C++ cast keyword for (target type, operand type) or raises Refuse."""
    tc, sc = target.get_canonical(), operand.get_canonical()
    if tc.kind == cindex.TypeKind.VOID:
        return None
    if "type-parameter" in tc.spelling or "type-parameter" in sc.spelling or tc.kind == cindex.TypeKind.DEPENDENT:
        raise Refuse("dependent type")
    tp, sp = pointee(tc), pointee(sc)
    if is_ref:
        # libclang gives a reference cast the type of its referent: both sides are lvalues of that type
        tp = (tc, True)
        sp = (sc.get_pointee() if sc.kind in REFS else sc, True)
    if tp and sp:
        t_elem, s_elem = tp[0].get_canonical(), sp[0].get_canonical()
        drops_const = (s_elem.is_const_qualified() and not t_elem.is_const_qualified()) or (
            s_elem.is_volatile_qualified() and not t_elem.is_volatile_qualified()
        )
        if cvless(tc) == cvless(sc) or (tp[1] and cvless(t_elem) == cvless(s_elem)):
            return "const_cast" if tc.spelling != sc.spelling and drops_const_anywhere(tc, sc) else "static_cast"
        if drops_const:
            raise Refuse("drops const and converts")
        if t_elem.kind == cindex.TypeKind.VOID or s_elem.kind == cindex.TypeKind.VOID:
            if t_elem.kind in (cindex.TypeKind.FUNCTIONPROTO, cindex.TypeKind.FUNCTIONNOPROTO) or s_elem.kind in (
                cindex.TypeKind.FUNCTIONPROTO, cindex.TypeKind.FUNCTIONNOPROTO,
            ):
                return "reinterpret_cast"
            return "static_cast"
        if related(t_elem, s_elem):
            return "static_cast"
        return "reinterpret_cast"
    if tc.kind in POINTERS and sc.kind in INTEGRAL:
        return "reinterpret_cast"
    if tc.kind in INTEGRAL and sc.kind in POINTERS:
        return "reinterpret_cast"
    if tc.kind in ARITH and sc.kind in ARITH:
        return "static_cast"
    if tc.kind == cindex.TypeKind.RECORD and sc.kind == cindex.TypeKind.RECORD:
        return "static_cast"
    raise Refuse(f"no rule for {sc.spelling} -> {tc.spelling}")


def drops_const_anywhere(tc, sc):
    return len(CV.findall(sc.spelling)) > len(CV.findall(tc.spelling))


def matching_paren(text, start):
    depth = 0
    for i in range(start, len(text)):
        if text[i] in "([{":
            depth += 1
        elif text[i] in ")]}":
            depth -= 1
            if depth == 0:
                return i
    return -1


def in_define(text, offset):
    """True if the offset is on a #define line or its continuation lines."""
    line_start = text.rfind("\n", 0, offset) + 1
    while True:
        line = text[line_start : text.find("\n", line_start) if "\n" in text[line_start:] else len(text)]
        if line.lstrip().startswith("#define"):
            return True
        if line_start == 0:
            return False
        prev_start = text.rfind("\n", 0, line_start - 1) + 1
        if not text[prev_start : line_start - 1].rstrip("\r").endswith("\\"):
            return False
        line_start = prev_start


def scan_tu(args):
    path, argv = args
    index = cindex.Index.create()
    tu = index.parse(path, args=argv, options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    bad = [d for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if bad:
        return path, None, [f"{path}: {bad[0].spelling}"]
    found, refused = [], []
    sources = {}

    def text_of(f):
        if f not in sources:
            sources[f] = open(f, newline="", errors="surrogateescape").read()
        return sources[f]

    def visit(cur):
        if cur.location.file is not None and not cur.location.file.name.startswith(GAME):
            return
        if cur.kind == cindex.CursorKind.CSTYLE_CAST_EXPR and cur.location.file is not None:
            try:
                found.append(edit_for(cur, text_of))
            except Refuse as e:
                refused.append(f"{cur.location.file.name}:{cur.location.line}:{cur.location.column}: {e}")
        for child in cur.get_children():
            visit(child)

    for top in tu.cursor.get_children():
        if top.location.file is not None and top.location.file.name.startswith(GAME):
            visit(top)
    return path, [e for e in found if e], refused


def edit_for(cur, text_of):
    f = cur.extent.start.file.name
    text = text_of(f)
    start, end = cur.extent.start.offset, cur.extent.end.offset
    if cur.extent.end.file is None or cur.extent.end.file.name != f or text[start] != "(":
        raise Refuse("cast inside a macro")
    if in_define(text, start):
        raise Refuse("cast inside a #define")
    close = matching_paren(text, start)
    exprs = [c for c in cur.get_children() if c.kind.is_expression()]
    if close < 0 or not exprs:
        raise Refuse("cannot find the operand")
    operand = exprs[-1]
    op_start = operand.extent.start.offset
    if operand.extent.start.file is None or operand.extent.start.file.name != f or operand.extent.end.offset != end:
        raise Refuse("operand extent differs (macro argument?)")
    if text[close + 1 : op_start].strip():
        raise Refuse("unexpected text between the type and the operand")
    type_text = " ".join(text[start + 1 : close].split())
    kind = classify(cur.type, operand.type, type_text.endswith("&"))
    if kind is None:
        return None
    op_text = text[close + 1 : end].strip()
    if op_text.startswith("(") and matching_paren(op_text, 0) == len(op_text) - 1:
        op_text = op_text[1:-1]
    old = text[start:end]
    new = f"{kind}<{type_text}>({op_text})"
    return {"file": f, "start": start, "end": end, "old": old, "new": new, "kind": kind}


def commands(build_dir):
    resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
    db = json.load(open(os.path.join(build_dir, "compile_commands.json")))
    for entry in db:
        if not entry["file"].startswith(GAME) or not entry["file"].endswith(".cpp"):
            continue
        argv = entry.get("arguments") or shlex.split(entry["command"])
        out, skip = [], False
        for a in argv[1:]:
            if skip:
                skip = False
                continue
            if a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif a in ("-c", "-MD", "-MMD") or a == entry["file"] or a.endswith(".cpp"):
                continue
            else:
                out.append(a)
        yield entry["file"], out + ["-resource-dir", resource, "-working-directory", entry["directory"]]


def scan(build_dir, out_path, jobs):
    edits, refused, failed = {}, set(), []
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, found, ref in pool.map(scan_tu, list(commands(build_dir))):
            if found is None:
                failed += ref
                continue
            refused.update(ref)
            for e in found:
                key = (e["file"], e["start"])
                if key in edits and edits[key]["new"] != e["new"]:
                    refused.add(f"{e['file']}@{e['start']}: different results in different TUs")
                    edits[key]["conflict"] = True
                edits.setdefault(key, e)
    good = [e for e in edits.values() if not e.get("conflict")]
    json.dump(good, open(out_path, "w"), indent=1)
    kinds = collections.Counter(e["kind"] for e in good)
    for line in sorted(refused):
        print("REFUSED", line)
    for line in failed:
        print("TU FAILED", line)
    print(f"{len(good)} edits {dict(kinds)}; {len(refused)} refused; {len(failed)} TUs failed", file=sys.stderr)
    return 1 if failed else 0


def apply(edits_path, select, dry):
    edits = [e for e in json.load(open(edits_path)) if re.search(select, e["file"])]
    by_file = collections.defaultdict(list)
    for e in edits:
        by_file[e["file"]].append(e)
    done = deferred = 0
    for f, es in by_file.items():
        text = open(f, newline="", errors="surrogateescape").read()
        es.sort(key=lambda e: e["start"])
        keep = []
        for e in es:
            inner = any(o is not e and e["start"] <= o["start"] and o["end"] <= e["end"] for o in es)
            if inner:
                deferred += 1
            else:
                keep.append(e)
        for e in reversed(keep):
            if text[e["start"] : e["end"]] != e["old"]:
                sys.exit(f"{f}@{e['start']}: text changed since the scan; scan again")
            text = text[: e["start"]] + e["new"] + text[e["end"] :]
            done += 1
        if not dry:
            open(f, "w", newline="", errors="surrogateescape").write(text)
    print(f"{done} casts rewritten in {len(by_file)} files; {deferred} contain another cast (scan again)", file=sys.stderr)
    return 0


def selftest():
    import tempfile

    src = """
struct A { int a; };
struct B { int b; };
struct C : B, A { int c; };
enum E { e0 };
void f(C* c, A* a, const A* ca, void* v, unsigned long n, int i, float x, B& rb) {
    (void)i;
    A* p1 = (A*)c;          // static (derived to base)
    C* p2 = (C*)a;          // static (base to derived)
    A* p3 = (A*)ca;         // const
    A* p4 = (A*)v;          // static
    A* p5 = (A*)n;          // reinterpret
    unsigned long q = (unsigned long)a;  // reinterpret
    float y = (float)i;     // static
    int j = (int)(x * 2);   // static, reuses the parens
    E ee = (E)i;            // static
    C* p6 = (C*)(A*)ca;     // nested; refuses the inner (drops const and converts)
    B* p7 = (B*)ca;         // refused
    B& r = (B&)*ca;         // refused (const and converts)
    int* z = (int*)&rb;     // reinterpret
}
"""
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "t.cpp")
        open(path, "w").write(src)
        global GAME
        GAME = d + os.sep
        resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
        _, found, refused = scan_tu((path, ["-std=c++17", "-resource-dir", resource]))
        got = [(e["old"], e["kind"], e["new"]) for e in found]
        for g in got:
            print(g)
        for r in sorted(refused):
            print("REFUSED", r.split("/")[-1])
        want = {
            "(A*)c": "static_cast",
            "(C*)a": "static_cast",
            "(A*)ca": "const_cast",
            "(A*)v": "static_cast",
            "(A*)n": "reinterpret_cast",
            "(unsigned long)a": "reinterpret_cast",
            "(float)i": "static_cast",
            "(int)(x * 2)": "static_cast",
            "(E)i": "static_cast",
            "(int*)&rb": "reinterpret_cast",
        }
        have = {old: kind for old, kind, _ in got}
        for old, kind in want.items():
            assert have.get(old) == kind, (old, have.get(old), kind)
        assert "static_cast<int>(x * 2)" in [n for _, _, n in got]
        assert len(refused) == 2, refused
    print("selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("scan")
    s.add_argument("build_dir")
    s.add_argument("out")
    s.add_argument("-j", type=int, default=4)
    a = sub.add_parser("apply")
    a.add_argument("edits")
    a.add_argument("--select", default=".")
    a.add_argument("--dry-run", action="store_true")
    sub.add_parser("selftest")
    args = ap.parse_args()
    if args.cmd == "scan":
        return scan(args.build_dir, args.out, args.j)
    if args.cmd == "apply":
        return apply(args.edits, args.select, args.dry_run)
    return selftest()


if __name__ == "__main__":
    sys.exit(main())
