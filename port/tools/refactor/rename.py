#!/usr/bin/env python3
"""USR-based C++ renames for port/ (port/refactor/2-names.md 2.3).

    rename.py [--dry-run] [--build-dir DIR] [--templates] [--residual] renames.csv
    rename.py --propose SUBSYSTEM KIND ...        (R2.4, see propose.py)

renames.csv has the columns `old,new`. `old` is a qualified name as written in the code
(`Game_0x40::field_2C_game_exit_type`, `Car_BC`, `gGame_0x40_67E008`) or `usr:<clang USR>` for a
symbol that has no unique qualified name (locals, parameters). The new name is applied to every
symbol with that qualified name (an overload set is renamed whole).

How it works. Every translation unit of DIR/compile_commands.json (default build_port/clang64) is
parsed with libclang (the pip libclang needs `-resource-dir`, so the tool adds it and stops if a
translation unit has an error). Each declaration and each reference whose symbol is one of the targets is
recorded at its *spelling* location (a name passed as a macro argument is edited where it is written).
Renaming a class renames its constructors and destructors too. The batch is REFUSED (nothing is
written, exit status 2) when:
  * a new name already exists in the same scope, in a base class, in a derived class, or two different old
    names would get the same new name in one scope;
  * a reference is not spelled as the old name in the source (a name made by a macro, `##`);
  * a reference is in a file outside port/;
  * a target is a virtual function or overrides one (rename the whole family by hand).
Two blind spots of the libclang view are checked with text afterwards: the old name still spelled in
code (outside comments and strings) where no cursor resolved it is a RESIDUAL. Residuals inside a
class or function template (a dependent name such as `pNew->mpNext`) are edited with `--templates`,
which is right only if the batch renames that member in every class the template is used with. Other
residuals (a branch of `#if` the build does not compile, a macro body) are listed and edited with
`--residual` after you have looked at them. Without the flag the exit status is 3 and nothing is written.

After the edits, address_map.csv gets the new names. Run the build and the check afterwards
(`port/tools/refactor/check.sh`): `linux32` builds the branches this view does not see.
"""
import argparse
import collections
import concurrent.futures
import csv
import ctypes
import json
import os
import re
import shlex
import subprocess
import sys

from clang import cindex

K = cindex.CursorKind
ROOT = os.path.realpath(os.environ.get("RENAME_ROOT") or os.path.join(os.path.dirname(__file__), "..", "..", ".."))
PORT = os.path.join(ROOT, "port") + os.sep
MAP_PATH = os.path.join(ROOT, "port", "tools", "refactor", "address_map.csv")

DECLS = {
    K.FUNCTION_DECL, K.CXX_METHOD, K.CONSTRUCTOR, K.DESTRUCTOR, K.VAR_DECL, K.FIELD_DECL, K.PARM_DECL,
    K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.ENUM_DECL, K.TYPEDEF_DECL, K.TYPE_ALIAS_DECL,
    K.ENUM_CONSTANT_DECL, K.FUNCTION_TEMPLATE, K.CLASS_TEMPLATE, K.NAMESPACE, K.TEMPLATE_TYPE_PARAMETER,
    K.TEMPLATE_NON_TYPE_PARAMETER,
}
TEMPLATES = {K.CLASS_TEMPLATE, K.FUNCTION_TEMPLATE, K.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION}
CLASSES = {K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.CLASS_TEMPLATE}
SCOPES = {K.TRANSLATION_UNIT, K.NAMESPACE, K.LINKAGE_SPEC}
IDENT = re.compile(r"[A-Za-z_]\w*")

_lib = cindex.conf.lib
_lib.clang_getSpellingLocation.argtypes = [
    cindex.SourceLocation, ctypes.POINTER(cindex.c_object_p), ctypes.POINTER(ctypes.c_uint),
    ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint),
]
_lib.clang_getSpellingLocation.restype = None


def spelling_location(loc):
    """(file name, offset) of where a location is spelled; for a macro argument that is the argument."""
    f, line, col, off = cindex.c_object_p(), ctypes.c_uint(), ctypes.c_uint(), ctypes.c_uint()
    _lib.clang_getSpellingLocation(loc, ctypes.byref(f), ctypes.byref(line), ctypes.byref(col), ctypes.byref(off))
    if not f:
        return None, 0, 0
    return cindex.File(f).name, off.value, line.value


def qualified_name(cur):
    parts = []
    c = cur
    while c is not None and c.kind != K.TRANSLATION_UNIT:
        if c.kind != K.LINKAGE_SPEC and c.spelling and not c.spelling.startswith("("):
            parts.append(c.spelling)
        c = c.semantic_parent
    return "::".join(reversed(parts))


def scope_key(cur):
    """The scope a declaration lives in: the USR of its class, function or namespace; "" for global."""
    p = cur.semantic_parent
    while p is not None and p.kind in (K.LINKAGE_SPEC,):
        p = p.semantic_parent
    if p is None or p.kind == K.TRANSLATION_UNIT:
        return ""
    return p.get_usr() or qualified_name(p)


class Worker:
    """Scans one translation unit."""

    def __init__(self, wanted, usr_targets, olds):
        self.wanted = wanted  # qualified name -> new name
        self.usr_targets = usr_targets  # usr -> new name
        self.olds = olds  # plain old spellings (for the residual check)
        self.cache = {}

    def match(self, decl):
        """The key of `wanted` that names this declaration: the whole qualified name, or its tail (the
        namespace in front of a named enum is not in the CSV)."""
        parts = qualified_name(decl).split("::")
        for i in range(len(parts)):
            k = "::".join(parts[i:])
            if k in self.wanted:
                return k
        return None

    def target_of(self, decl):
        """(new name, old spelling) if decl is renamed by this batch, else None."""
        usr = decl.get_usr()
        key = usr or ("q:" + qualified_name(decl))
        if key in self.cache:
            return self.cache[key]
        result = None
        if usr in self.usr_targets:
            result = (self.usr_targets[usr], decl.spelling)
        else:
            q = self.match(decl)
            if q is not None and decl.kind in DECLS:
                result = (self.wanted[q], decl.spelling, q)
            elif decl.kind in (K.CONSTRUCTOR, K.DESTRUCTOR):
                cq = self.match(decl.semantic_parent)
                if cq is not None:
                    result = (self.wanted[cq], decl.spelling, cq)
        self.cache[key] = result
        return result

    def run(self, path, argv):
        tu = cindex.Index.create().parse(path, args=argv, options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
        bad = [d for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
        if bad:
            return {"error": f"{path}: {bad[0].spelling} ({bad[0].location.file}:{bad[0].location.line})"}
        out = {
            "edits": {},  # (file, offset) -> (old spelling, new name, usr, line)
            "outside": [],  # references in files outside port/
            "macro": [],  # references whose name is not where the cursor says (file, off, line, old, new)
            "scopes": collections.defaultdict(set),  # scope key -> {(name, usr)}
            "bases": collections.defaultdict(set),  # class usr -> base usrs
            "targets": {},  # usr -> (qualified name, new name, scope key, kind name, virtual)
            "seen": set(),  # (file, offset) of resolved non-target cursors spelled as an old name
            "templates": set(),  # (file, start, end)
        }
        sources = {}

        def text(f):
            if f not in sources:
                sources[f] = open(f, encoding="latin-1", newline="").read()
            return sources[f]

        def visit(cur):
            f = cur.location.file
            if f is not None and not f.name.startswith(PORT):
                return
            if cur.kind in TEMPLATES and cur.extent.start.file is not None:
                e = cur.extent
                out["templates"].add((e.start.file.name, e.start.offset, e.end.offset))
            if cur.kind in DECLS and f is not None:
                usr = cur.get_usr()
                if cur.spelling:
                    out["scopes"][scope_key(cur)].add((cur.spelling, usr))
                    if cur.kind == K.ENUM_CONSTANT_DECL and not cur.semantic_parent.is_scoped_enum():
                        out["scopes"][scope_key(cur.semantic_parent)].add((cur.spelling, usr))
                if cur.kind in CLASSES and cur.is_definition():
                    for b in cur.get_children():
                        if b.kind == K.CXX_BASE_SPECIFIER:
                            d = b.type.get_canonical().get_declaration()
                            out["bases"][usr].add(d.get_usr())
                t = self.target_of(cur)
                if t:
                    virtual = cur.kind == K.CXX_METHOD and cur.is_virtual_method()
                    out["targets"][usr] = (t[2] if len(t) > 2 else qualified_name(cur), t[0], scope_key(cur), cur.kind.name, virtual)
            if f is not None and (cur.kind in DECLS or cur.kind.is_reference() or cur.kind in (
                K.DECL_REF_EXPR, K.MEMBER_REF_EXPR, K.OVERLOADED_DECL_REF
            )):
                ref = cur.referenced
                if ref is not None and ref.kind in DECLS:
                    t = self.target_of(ref)
                    file, off, line = spelling_location(cur.location)
                    if t and file:
                        old_sp = ref.spelling
                        cls_name = None
                        if ref.kind == K.DESTRUCTOR:
                            cls_name = ref.semantic_parent.spelling
                        elif ref.kind == K.CONSTRUCTOR:
                            cls_name = ref.semantic_parent.spelling
                        old = cls_name or old_sp
                        src = text(file) if os.path.exists(file) else ""
                        if cls_name and ref.kind == K.DESTRUCTOR and src[off : off + 1] == "~":
                            off += 1
                        if not file.startswith(PORT):
                            out["outside"].append((file, line, ref.spelling))
                        elif src[off : off + len(old)] != old or (
                            off + len(old) < len(src) and (src[off + len(old)].isalnum() or src[off + len(old)] == "_")
                        ):
                            out["macro"].append((file, off, line, old, t[0]))
                        else:
                            out["edits"][(file, off)] = (old, t[0], ref.get_usr(), line)
                    elif file and file.startswith(PORT) and ref.spelling in self.olds:
                        out["seen"].add((file, off))
            for child in cur.get_children():
                visit(child)

        for top in tu.cursor.get_children():
            if top.location.file is not None and top.location.file.name.startswith(PORT):
                visit(top)
        out["scopes"] = {k: sorted(v) for k, v in out["scopes"].items()}
        out["bases"] = {k: sorted(v) for k, v in out["bases"].items()}
        out["seen"] = sorted(out["seen"])
        out["templates"] = sorted(out["templates"])
        out["edits"] = [(k[0], k[1], *v) for k, v in out["edits"].items()]
        return out


def _scan(args):
    wanted, usr_targets, olds, path, argv = args
    return path, Worker(wanted, usr_targets, olds).run(path, argv)


def commands(build_dir, only=None):
    resource = subprocess.check_output(["clang++-18", "-print-resource-dir"], text=True).strip()
    seen = set()
    for e in json.load(open(os.path.join(build_dir, "compile_commands.json"))):
        f = e["file"]
        if not f.endswith((".cpp", ".cc", ".c")) or f in seen:
            continue
        if only and not any(re.search(o, f) for o in only):
            continue
        seen.add(f)
        argv = e.get("arguments") or shlex.split(e["command"])
        out, skip = [], False
        for a in argv[1:]:
            if skip:
                skip = False
            elif a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif a in ("-c", "-MD", "-MMD") or a == f:
                pass
            else:
                out.append(a)
        yield f, out + ["-resource-dir", resource, "-working-directory", e["directory"]]


def strip_code(text):
    """text with comments, string and character literals blanked (same length)."""
    out = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        two = text[i : i + 2]
        if two == "//":
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = " "
            i = j
        elif two == "/*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if text[k] != "\n":
                    out[k] = " "
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            for k in range(i + 1, min(j, n)):
                out[k] = " "
            i = j + 1
        else:
            i += 1
    return "".join(out)


def match_paren(text, start):
    """Index of the ')' that closes the '(' at start (strings and comments skipped), or the end."""
    code = strip_code(text)
    depth = 0
    for i in range(start, len(code)):
        if code[i] == "(":
            depth += 1
        elif code[i] == ")":
            depth -= 1
            if depth == 0:
                return i
    return len(code) - 1


IDENTITY = {}


def load_renames(path):
    wanted, usr_targets = {}, {}
    with open(path, newline="") as f:
        for row in csv.reader(f):
            if not row or row[0].startswith("#") or row[0] == "old":
                continue
            old, new = row[0].strip(), row[1].strip()
            if not new:
                sys.exit(f"{path}: no new name for {old}")
            if not IDENT.fullmatch(new):
                sys.exit(f"{path}: {new!r} is not an identifier")
            if old.startswith("usr:"):
                usr_targets[old[4:]] = new
            elif old.split("::")[-1] == new:
                IDENTITY[old] = new  # nothing to edit; address_map.csv still gets the new_name
            else:
                wanted[old] = new
    return wanted, usr_targets


def rename(csv_path, build_dir, dry, templates, residual, jobs, only=None):
    wanted, usr_targets = load_renames(csv_path)
    if not wanted and not usr_targets:
        if not dry:
            update_map(dict(IDENTITY), {})
        print("nothing to rename; address_map.csv updated" if not dry else "nothing to rename")
        return 0
    olds = {q.split("::")[-1] for q in wanted}
    argsets = [(wanted, usr_targets, olds, p, a) for p, a in commands(build_dir, only)]
    results = []
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, out in pool.map(_scan, argsets):
            if "error" in out:
                sys.exit("does not parse: " + out["error"])
            results.append(out)

    targets, edits, scopes, bases, seen, templ, outside = {}, {}, collections.defaultdict(set), {}, set(), set(), []
    macro_refs = set()
    for r in results:
        targets.update(r["targets"])
        for (f, off, old, new, usr, line) in r["edits"]:
            key = (f, off)
            if key in edits and edits[key][1] != new:
                edits[key] = (old, None, usr, line)
            else:
                edits.setdefault(key, (old, new, usr, line))
        for k, v in r["scopes"].items():
            scopes[k].update(map(tuple, v))
        for k, v in r["bases"].items():
            bases.setdefault(k, set()).update(v)
        seen.update(map(tuple, r["seen"]))
        templ.update(map(tuple, r["templates"]))
        outside += r["outside"]
        macro_refs.update(map(tuple, r["macro"]))

    refused = []
    port_files = {}
    for dirpath, _, names in os.walk(PORT):
        for n in names:
            if n.endswith((".cpp", ".hpp", ".h", ".inl", ".c")):
                port_files[os.path.join(dirpath, n)] = None

    def source(p):
        if port_files.get(p) is None:
            port_files[p] = open(p, encoding="latin-1", newline="").read()
        return port_files[p]

    defines = None
    for (f, off, line, old, new) in sorted(macro_refs):
        # The cursor points at a macro invocation. The name is either a literal argument of it (a
        # declaration made by DEFINE_GLOBAL(type, name, addr)): edit it there; or it is spelled in the
        # macro's body (the residual pass finds it, to be edited with --residual); or it is made by the
        # macro (token pasting): refuse.
        src = source(f)
        m = IDENT.match(src, off)
        if not m:
            refused.append(f"{os.path.relpath(f, ROOT)}:{line}: {old} is not spelled in the source")
            continue
        macro_name = m.group(0)
        args = ""
        if src[m.end() : m.end() + 1] == "(":
            close = match_paren(src, m.end())
            args = src[m.end() : close + 1]
        hits = [h for h in re.finditer(r"\b" + re.escape(old) + r"\b", strip_code(args))] if args else []
        if len(hits) == 1:
            edits[(f, m.end() + hits[0].start())] = (old, new, "", line)
            continue
        if len(hits) > 1:
            refused.append(f"{os.path.relpath(f, ROOT)}:{line}: {old} appears {len(hits)} times in the arguments of {macro_name}")
            continue
        if defines is None:
            defines = {}
            for p in port_files:
                for dm in re.finditer(r"^[ \t]*#[ \t]*define[ \t]+(\w+)(.*(?:\\\n.*)*)", source(p), re.M):
                    defines.setdefault(dm.group(1), []).append(dm.group(2))
        if any(re.search(r"\b" + re.escape(old) + r"\b", b) for b in defines.get(macro_name, [])):
            continue  # spelled in the macro body: a residual
        refused.append(f"{os.path.relpath(f, ROOT)}:{line}: {old} is made by the macro {macro_name}, not spelled in the source")
    for q in wanted:
        if not any(t[0] == q for t in targets.values()):
            refused.append(f"{q}: no such symbol")
    for f, line, name in outside:
        refused.append(f"{f}:{line}: reference to {name} outside port/")
    for (f, off), (old, new, usr, line) in sorted(edits.items()):
        if new is None:
            refused.append(f"{os.path.relpath(f, ROOT)}:{line}: {old} is not spelled in the source (macro?)")
    new_of = {u: t[1] for u, t in targets.items()}
    for usr, (q, new, scope, kind, virtual) in targets.items():
        if virtual:
            refused.append(f"{q}: virtual function; rename the family by hand")
    # collisions
    derived = collections.defaultdict(set)
    for c, bs in bases.items():
        for b in bs:
            derived[b].add(c)

    def names_in(scope):
        return scopes.get(scope, set())

    def all_bases(c, acc=None):
        acc = set() if acc is None else acc
        for b in bases.get(c, ()):
            if b not in acc:
                acc.add(b)
                all_bases(b, acc)
        return acc

    def all_derived(c, acc=None):
        acc = set() if acc is None else acc
        for d in derived.get(c, ()):
            if d not in acc:
                acc.add(d)
                all_derived(d, acc)
        return acc

    by_scope = collections.defaultdict(dict)
    for usr, (q, new, scope, kind, virtual) in targets.items():
        old = q.split("::")[-1]
        if kind in ("CONSTRUCTOR", "DESTRUCTOR"):
            continue
        prev = by_scope[scope].setdefault(new, q)
        if prev != q:
            refused.append(f"{q} and {prev} would both be called {new} in one scope")
        scopes_to_check = {scope} | all_bases(scope) | all_derived(scope)
        for sc in scopes_to_check:
            for name, other in names_in(sc):
                if name == new and other != usr and new_of.get(other, name) == new:
                    refused.append(f"{q}: the new name {new} already exists in the scope {sc or '(global)'}")
    # residuals
    edited = {(f, o) for (f, o) in edits}
    res_template, res_other = [], []
    files = set()
    for dirpath, _, names in os.walk(PORT):
        if "/tests/rename/" in dirpath + "/":
            pass
        for n in names:
            if n.endswith((".cpp", ".hpp", ".h", ".inl", ".c")):
                files.add(os.path.join(dirpath, n))
    olds_re = re.compile(r"\b(" + "|".join(re.escape(o) for o in sorted(olds)) + r")\b") if olds else None
    for p in sorted(files):
        text = open(p, encoding="latin-1", newline="").read()
        if olds_re is None or not olds_re.search(text):
            continue
        code = strip_code(text)
        for m in olds_re.finditer(code):
            key = (p, m.start())
            if key in edited or key in seen:
                continue
            line = text.count("\n", 0, m.start()) + 1
            inside = any(f == p and s <= m.start() < e for (f, s, e) in templ)
            (res_template if inside else res_other).append((p, m.start(), m.group(1), line))
    if refused:
        for line in refused:
            print("REFUSED", line)
        return 2
    by_new = {q.split("::")[-1]: n for q, n in wanted.items()}
    extra = []
    if templates:
        extra += res_template
    if residual:
        extra += res_other
    for p, off, name, line in res_template:
        print(f"RESIDUAL (template) {os.path.relpath(p, ROOT)}:{line}: {name}" + ("" if templates else "  [--templates]"))
    for p, off, name, line in res_other:
        print(f"RESIDUAL {os.path.relpath(p, ROOT)}:{line}: {name}" + ("" if residual else "  [--residual]"))
    if (res_template and not templates) or (res_other and not residual):
        return 3
    # which new name does a residual get? the one of its old spelling (the batch must be unambiguous)
    final = {(f, o): (old, new) for (f, o), (old, new, usr, line) in edits.items()}
    for p, off, name, line in extra:
        newname = {q.split("::")[-1]: n for q, n in wanted.items()}.get(name)
        if newname is None or len({n for q, n in wanted.items() if q.split("::")[-1] == name}) != 1:
            print(f"AMBIGUOUS residual {os.path.relpath(p, ROOT)}:{line}: {name} has several new names")
            return 2
        final[(p, off)] = (name, newname)
    per_file = collections.defaultdict(list)
    for (f, off), (old, new) in final.items():
        per_file[f].append((off, old, new))
    for f, es in sorted(per_file.items()):
        print(f"{'would edit' if dry else 'edit'} {os.path.relpath(f, ROOT)}: {len(es)}")
    print(f"{len(final)} edits in {len(per_file)} files, {len(targets)} symbols", file=sys.stderr)
    if dry:
        return 0
    for f, es in per_file.items():
        text = open(f, encoding="latin-1", newline="").read()
        for off, old, new in sorted(es, reverse=True):
            assert text[off : off + len(old)] == old, (f, off, old)
            text = text[:off] + new + text[off + len(old) :]
        open(f, "w", encoding="latin-1", newline="").write(text)
    update_map({**wanted, **IDENTITY}, targets)
    return 0


def update_map(wanted, targets):
    if not os.path.exists(MAP_PATH):
        return
    rows = list(csv.DictReader(open(MAP_PATH, newline="")))
    cols = list(rows[0].keys())
    by_qual = {}
    for q, new in wanted.items():
        by_qual[q] = new
    for r in rows:
        qual = f"{r['owner']}::{r['old_name']}" if r["owner"] else r["old_name"]
        if qual in by_qual:
            r["new_name"] = by_qual[qual]
        if r["owner"] in by_qual:
            r["new_owner"] = by_qual[r["owner"]]
    with open(MAP_PATH, "w", newline="") as f:
        w = csv.DictWriter(f, cols, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--propose":
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import propose

        sys.argv = [sys.argv[0]] + sys.argv[2:]
        return propose.main()
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("renames", nargs="?")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--build-dir", default=os.path.join(ROOT, "build_port", "clang64"))
    ap.add_argument("--templates", action="store_true", help="also edit residuals inside templates")
    ap.add_argument("--residual", action="store_true", help="also edit the other residuals")
    ap.add_argument("--only", action="append", help="regex: only parse the TUs whose path matches (tests)")
    ap.add_argument("-j", type=int, default=6)
    a = ap.parse_args()
    if not a.renames:
        ap.error("renames.csv missing")
    return rename(a.renames, os.path.realpath(a.build_dir), a.dry_run, a.templates, a.residual, a.j, a.only)


if __name__ == "__main__":
    sys.exit(main())
