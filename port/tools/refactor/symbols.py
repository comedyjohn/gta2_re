#!/usr/bin/env python3
"""Every declared name of port/game with what the naming rules need to know (task R2.4).

    python3 port/tools/refactor/symbols.py build_port/clang64 [-o build_port/symbols.json] [-j N]

Parses every translation unit with libclang and writes one JSON list: a record per declaration in
port/game (not 3rdParty) with `usr`, `kind` (function, method, ctor, dtor, field, global, type, alias,
enumerator, namespace, param, local), `name`, `owner` (the qualified scope), `file`, `line`, and the facts
the rules use: `access`, `static`, `const` (a constant: `const` or `constexpr` variable), `has_methods`
(for a field: its class declares member functions), `scoped` (for an enumerator: an `enum class`),
`operator`. propose.py reads this file; name_step.py refreshes it before every step.
"""
import argparse
import concurrent.futures
import json
import os
import shlex
import subprocess
import sys

from clang import cindex

K = cindex.CursorKind
ROOT = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GAME = os.path.join(ROOT, "port", "game") + os.sep
SCOPES = (K.TRANSLATION_UNIT, K.NAMESPACE, K.LINKAGE_SPEC)
TYPES = {K.CLASS_DECL: "type", K.STRUCT_DECL: "type", K.UNION_DECL: "type", K.ENUM_DECL: "type",
         K.CLASS_TEMPLATE: "type", K.TYPEDEF_DECL: "alias", K.TYPE_ALIAS_DECL: "alias"}
FUNCS = {K.FUNCTION_DECL: "function", K.CXX_METHOD: "method", K.CONSTRUCTOR: "ctor", K.DESTRUCTOR: "dtor",
         K.FUNCTION_TEMPLATE: "function", K.CONVERSION_FUNCTION: "method"}


def qualified(cur):
    parts = []
    while cur is not None and cur.kind != K.TRANSLATION_UNIT:
        if cur.kind != K.LINKAGE_SPEC and cur.spelling and not cur.spelling.startswith("("):
            parts.append(cur.spelling)
        cur = cur.semantic_parent
    return "::".join(reversed(parts))


def owner_of(cur):
    p = cur.semantic_parent
    while p is not None and p.kind == K.LINKAGE_SPEC:
        p = p.semantic_parent
    if p is None or p.kind == K.TRANSLATION_UNIT:
        return ""
    return qualified(p)


def has_methods(cls):
    for c in cls.get_children():
        if c.kind in (K.CXX_METHOD, K.CONSTRUCTOR, K.DESTRUCTOR, K.FUNCTION_TEMPLATE, K.CONVERSION_FUNCTION):
            return True
        if c.kind == K.CXX_BASE_SPECIFIER:
            d = c.type.get_canonical().get_declaration()
            if d.kind != K.NO_DECL_FOUND and d != cls and has_methods(d):
                return True
    return False


def record(cur, in_function):
    k = cur.kind
    name = cur.spelling
    if not name or name.startswith("("):
        return None
    f = cur.location.file
    if f is None or not f.name.startswith(GAME) or f.name.startswith(GAME + "3rdParty"):
        return None
    r = {"usr": cur.get_usr(), "name": name, "owner": owner_of(cur), "file": os.path.relpath(f.name, GAME),
         "line": cur.location.line}
    if k in FUNCS:
        r["kind"] = FUNCS[k]
        r["operator"] = name.startswith("operator")
        r["static"] = cur.is_static_method() if k == K.CXX_METHOD else False
        # the parameter types and the const-ness: what makes two functions of one name different overloads
        r["sig"] = "(" + ",".join(a.type.get_canonical().spelling for a in cur.get_arguments()) + ")" + (
            "const" if k == K.CXX_METHOD and cur.is_const_method() else "")
    elif k == K.FIELD_DECL:
        parent = cur.semantic_parent
        r.update(kind="field", has_methods=has_methods(parent) if parent is not None else False,
                 access=cur.access_specifier.name, const=cur.type.is_const_qualified())
    elif k == K.VAR_DECL:
        if in_function:
            r["kind"] = "local"
        elif cur.semantic_parent.kind in (K.CLASS_DECL, K.STRUCT_DECL, K.CLASS_TEMPLATE):
            r.update(kind="static_member", const=cur.type.is_const_qualified())
        else:
            r.update(kind="global", const=cur.type.is_const_qualified(),
                     static=cur.storage_class == cindex.StorageClass.STATIC)
    elif k == K.PARM_DECL:
        r["kind"] = "param"
    elif k in TYPES:
        if k in (K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.ENUM_DECL) and not cur.is_definition():
            return None
        r["kind"] = TYPES[k]
        r["tag"] = k.name
    elif k == K.ENUM_CONSTANT_DECL:
        parent = cur.semantic_parent
        r.update(kind="enumerator", scoped=parent.is_scoped_enum(),
                 enum="" if not parent.spelling or parent.spelling.startswith("(") else parent.spelling)
        r["owner"] = qualified(parent) if parent.spelling and not parent.spelling.startswith("(") else owner_of(parent)
    elif k == K.NAMESPACE:
        r["kind"] = "namespace"
    elif k in (K.TEMPLATE_TYPE_PARAMETER, K.TEMPLATE_NON_TYPE_PARAMETER):
        r["kind"] = "tparam"
    else:
        return None
    return r


def scan_tu(args):
    path, argv = args
    tu = cindex.Index.create().parse(path, args=argv)
    if any(d.severity >= cindex.Diagnostic.Error for d in tu.diagnostics):
        return path, None
    out = {}

    def visit(cur, in_function):
        f = cur.location.file
        if f is not None and not f.name.startswith(GAME):
            return
        if f is not None:
            r = record(cur, in_function)
            if r:
                out[(r["usr"], r["file"], r["line"], r["name"])] = r
        fn = in_function or cur.kind in FUNCS
        if cur.kind in (K.TRANSLATION_UNIT, K.NAMESPACE, K.LINKAGE_SPEC, K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL,
                        K.ENUM_DECL, K.CLASS_TEMPLATE, K.FUNCTION_TEMPLATE) or fn:
            for ch in cur.get_children():
                visit(ch, fn)

    visit(tu.cursor, False)
    return path, list(out.values())


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


def scan(build_dir, jobs):
    seen = {}
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, recs in pool.map(scan_tu, list(commands(build_dir))):
            if recs is None:
                sys.exit(f"{path} does not parse")
            for r in recs:
                seen[(r["usr"], r["file"], r["line"], r["name"])] = r
    return sorted(seen.values(), key=lambda r: (r["file"], r["line"], r["name"]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("build_dir", nargs="?", default=os.path.join(ROOT, "build_port", "clang64"))
    ap.add_argument("-o", "--output", default=os.path.join(ROOT, "build_port", "symbols.json"))
    ap.add_argument("-j", type=int, default=8)
    a = ap.parse_args()
    recs = scan(os.path.realpath(a.build_dir), a.j)
    with open(a.output, "w") as f:
        json.dump(recs, f)
    kinds = {}
    for r in recs:
        kinds[r["kind"]] = kinds.get(r["kind"], 0) + 1
    print(f"{len(recs)} declarations {kinds} -> {os.path.relpath(a.output, ROOT)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
