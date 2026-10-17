#!/usr/bin/env python3
"""The address map: original names, addresses and their new names (port/refactor/2-names.md 2.2).

    python3 port/tools/refactor/address_map.py --init build_port/clang64   # builds address_map.csv
    python3 port/tools/refactor/address_map.py --lookup 0x4B8BD0           # or an old or a new name
    python3 port/tools/refactor/address_map.py --strip-comments            # removes the moved comments
    python3 port/tools/refactor/address_map.py --selftest

`--init` reads every declaration of port/game with libclang (every translation unit of the build
directory) and writes port/tools/refactor/address_map.csv, one row per symbol:

    kind        function, method, global, member, type or enumerator
    address     0x4B8BD0 for functions and globals (from the `_4B8BD0` suffix, or from a `// 0xADDR`
                comment above the definition), the offset 0x2C for members (`field_2C_...`), the size
                for types (`Car_BC` is 0xBC bytes)
    owner       the class of a method, member or enumerator (old name)
    old_name    the name as it is in the code
    new_name    filled in by rename.py (empty until then)
    new_owner   same for the owner
    addr_96f    the 9.6f address from `// 9.6f 0x...` and `// FUNCTION: 96f 0x...` comments
    decompme    the scratch id from `// https://decomp.me/scratch/...` comments
    file        where the symbol is defined, relative to port/game

`--strip-comments` deletes the three comment forms above from the code once they are in the CSV; other
`9.6f` notes (`// 9.6f inline`, `// 9.6f calls ...`) say something the CSV can't and stay.
"""
import argparse
import collections
import concurrent.futures
import csv
import json
import os
import re
import shlex
import subprocess
import sys

from clang import cindex

K = cindex.CursorKind
ROOT = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GAME = os.path.join(ROOT, "port", "game") + os.sep
CSV_PATH = os.path.join(ROOT, "port", "tools", "refactor", "address_map.csv")
COLUMNS = ["kind", "address", "owner", "old_name", "new_name", "new_owner", "addr_96f", "decompme", "file"]

OWNERS = (K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.ENUM_DECL, K.CLASS_TEMPLATE, K.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION,
          K.NAMESPACE)
PLAIN_KINDS = {K.FUNCTION_DECL, K.CXX_METHOD, K.FIELD_DECL, K.VAR_DECL, K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL,
               K.ENUM_DECL, K.ENUM_CONSTANT_DECL, K.TYPEDEF_DECL, K.TYPE_ALIAS_DECL, K.FUNCTION_TEMPLATE, K.CLASS_TEMPLATE}
SYMBOLS_PATH = os.path.join(ROOT, "port", "tools", "refactor", "symbol_names.csv")
FUNC_ADDR = re.compile(r"_([0-9A-Fa-f]{6})(?=_|$)")
COMMENT_ADDR = re.compile(r"^\s*//\s*(?:FUNCTION:\s*)?0x([0-9A-Fa-f]{6})\s*$")
COMMENT_96F = re.compile(r"^\s*//\s*(?:9\.6f|FUNCTION: 96f)\s+0x([0-9A-Fa-f]{5,6})\s*$")
COMMENT_DECOMPME = re.compile(r"^\s*//\s*https://decomp\.me/scratch/([A-Za-z0-9]+)\s*$")
FIELD = re.compile(r"^field_([0-9A-Fa-f]+)(?:_|$)")
TYPE_SIZE = re.compile(r"_(0x[0-9A-Fa-f]+|[0-9A-F]{1,5})$")
ENUM_VALUE = re.compile(r"_(\d+)$")


def plausible(addr):
    return 0x400000 <= int(addr, 16) <= 0x7FFFFF


def address_from_name(name):
    found = [m.group(1) for m in FUNC_ADDR.finditer(name) if plausible(m.group(1))]
    return "0x" + found[-1].upper() if found else ""


def comment_block(lines, line_no):
    """The comment lines directly above a declaration (line_no is 1-based)."""
    block, i = [], line_no - 2
    while i >= 0 and lines[i].lstrip().startswith("//"):
        block.append(lines[i])
        i -= 1
    return block


def row_for(cur, lines_of):
    """The CSV row for a declaration cursor, or None if it carries nothing to map."""
    name = cur.spelling
    f = cur.location.file.name
    rel = os.path.relpath(f, GAME)
    if rel.startswith("3rdParty"):
        return None
    owner = ""
    if cur.semantic_parent is not None and cur.semantic_parent.kind in OWNERS:
        parent = cur.semantic_parent
        owner = parent.spelling
        if not owner or owner.startswith("(unnamed") or owner.startswith("(anonymous"):
            # an unnamed enum or union: its enclosing class is the owner (or none at namespace scope)
            outer = parent.semantic_parent
            owner = outer.spelling if outer is not None and outer.kind in OWNERS else ""
    row = dict.fromkeys(COLUMNS, "")
    row["old_name"], row["owner"], row["file"] = name, owner, rel
    k = cur.kind
    if k in (K.CXX_METHOD, K.FUNCTION_DECL, K.CONSTRUCTOR, K.DESTRUCTOR, K.FUNCTION_TEMPLATE):
        row["kind"] = "method" if owner else "function"
        row["address"] = address_from_name(name)
    elif k == K.VAR_DECL:
        if cur.semantic_parent.kind not in (K.TRANSLATION_UNIT, K.NAMESPACE, K.CLASS_DECL, K.STRUCT_DECL):
            return None
        row["kind"] = "global"
        row["address"] = address_from_name(name)
    elif k == K.FIELD_DECL:
        m = FIELD.match(name)
        if not m:
            return None
        row["kind"] = "member"
        row["address"] = "0x" + m.group(1).upper()
    elif k in (K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.ENUM_DECL):
        if not cur.is_definition() or not name or name.startswith("("):
            return None
        row["kind"] = "type"
        m = TYPE_SIZE.search(name)
        if m:
            size = m.group(1)
            row["address"] = "0x" + (size[2:] if size.startswith("0x") else size).upper()
    elif k == K.ENUM_CONSTANT_DECL:
        m = ENUM_VALUE.search(name)
        if not m:
            return None
        row["kind"] = "enumerator"
    else:
        return None
    if k in (K.CXX_METHOD, K.FUNCTION_DECL, K.CONSTRUCTOR, K.DESTRUCTOR, K.FUNCTION_TEMPLATE):
        lines = lines_of(f)
        for text in comment_block(lines, cur.location.line):
            m = COMMENT_ADDR.match(text)
            if m and not row["address"] and plausible(m.group(1)):
                row["address"] = "0x" + m.group(1).upper()
            m = COMMENT_96F.match(text)
            if m:
                row["addr_96f"] = "0x" + m.group(1).upper()
            m = COMMENT_DECOMPME.match(text)
            if m:
                row["decompme"] = m.group(1)
        if not (row["address"] or row["addr_96f"] or row["decompme"]):
            return None
    elif k == K.VAR_DECL and not row["address"]:
        return None
    return cur.get_usr(), row


def scan_tu(args):
    path, argv = args
    tu = cindex.Index.create().parse(path, args=argv)
    if any(d.severity >= cindex.Diagnostic.Error for d in tu.diagnostics):
        return path, None
    cache = {}

    def lines_of(f):
        if f not in cache:
            cache[f] = open(f, errors="replace").read().split("\n")
        return cache[f]

    rows = []
    plain = set()

    def visit(cur):
        if cur.location.file is not None and not cur.location.file.name.startswith(GAME):
            return
        if cur.location.file is not None:
            r = row_for(cur, lines_of) if cur.kind != K.TRANSLATION_UNIT else None
            if r:
                rows.append(r)
            if cur.spelling and not cur.spelling.startswith("(") and cur.kind in PLAIN_KINDS and not os.path.relpath(
                cur.location.file.name, GAME
            ).startswith("3rdParty"):
                p = cur.semantic_parent
                owner = p.spelling if p is not None and p.kind in OWNERS else ""
                plain.add((owner, cur.spelling))
        if cur.kind in (K.TRANSLATION_UNIT, K.NAMESPACE, K.CLASS_DECL, K.STRUCT_DECL, K.UNION_DECL, K.ENUM_DECL,
                        K.LINKAGE_SPEC, K.CLASS_TEMPLATE):
            for child in cur.get_children():
                visit(child)

    visit(tu.cursor)
    return path, (rows, sorted(plain))


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
            elif a in ("-c", "-MD", "-MMD") or a == e["file"] or a.endswith(".cpp"):
                pass
            else:
                out.append(a)
        yield e["file"], out + ["-resource-dir", resource, "-working-directory", e["directory"]]


def merge(a, b):
    """Two rows for one USR (a declaration and a definition): keep what either knows."""
    for col in COLUMNS:
        if not a[col] and b[col]:
            a[col] = b[col]
    if b["file"].endswith(".cpp") and not a["file"].endswith(".cpp"):
        a["file"] = b["file"]
    return a


def fix_owners(build_dir, jobs):
    """Fill the owner of rows that have none (members of class templates) from a fresh scan."""
    fresh = collections.defaultdict(set)
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, found in pool.map(scan_tu, list(commands(build_dir))):
            for usr, row in found[0]:
                if row["owner"]:
                    fresh[(row["old_name"], row["file"])].add((row["owner"], row["kind"]))
    with open(CSV_PATH, newline="") as f:
        rows = list(csv.DictReader(f))
    fixed = 0
    for r in rows:
        if not r["owner"]:
            owners = fresh.get((r["old_name"], r["file"]), set())
            if len(owners) == 1:
                r["owner"], r["kind"] = next(iter(owners))
                fixed += 1
    with open(CSV_PATH, "w", newline="") as f:
        w = csv.DictWriter(f, COLUMNS, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    print(f"{fixed} owners filled in", file=sys.stderr)
    return 0


def init(build_dir, jobs, symbols_only=False):
    rows = {}
    failed = []
    plain = set()
    with concurrent.futures.ProcessPoolExecutor(jobs) as pool:
        for path, found in pool.map(scan_tu, list(commands(build_dir))):
            if found is None:
                failed.append(path)
                continue
            for usr, row in found[0]:
                rows[usr] = merge(rows[usr], row) if usr in rows else row
            plain.update(map(tuple, found[1]))
    if failed:
        sys.exit("do not parse: " + ", ".join(failed))
    ordered = sorted(rows.values(), key=lambda r: (r["kind"], r["owner"], r["old_name"]))
    # keep the names that rename.py has already recorded
    if os.path.exists(CSV_PATH):
        old = {(r["kind"], r["owner"], r["old_name"]): r for r in csv.DictReader(open(CSV_PATH, newline=""))}
        for r in ordered:
            prev = old.get((r["kind"], r["owner"], r["old_name"]))
            if prev:
                r["new_name"], r["new_owner"] = prev["new_name"], prev["new_owner"]
    with open(SYMBOLS_PATH, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["owner", "name"])
        w.writerows(sorted(plain))
    if symbols_only:
        print(f"{len(plain)} symbol names -> {os.path.relpath(SYMBOLS_PATH, ROOT)}", file=sys.stderr)
        return 0
    with open(CSV_PATH, "w", newline="") as f:
        w = csv.DictWriter(f, COLUMNS, lineterminator="\n")
        w.writeheader()
        w.writerows(ordered)
    kinds = collections.Counter(r["kind"] for r in ordered)
    print(f"{len(ordered)} rows {dict(kinds)} -> {os.path.relpath(CSV_PATH, ROOT)}", file=sys.stderr)
    return 0


def lookup(key):
    key_hex = key.lower()
    if re.fullmatch(r"(0x)?[0-9a-f]{5,6}", key_hex):
        key_hex = "0x" + key_hex.removeprefix("0x").upper()
    hits = 0
    with open(CSV_PATH) as f:
        for r in csv.DictReader(f):
            names = {r["old_name"], r["new_name"], r["owner"] + "::" + r["old_name"]}
            if key in names or (key_hex.startswith("0x") and key_hex in (r["address"], r["addr_96f"])):
                hits += 1
                qual = f"{r['owner']}::{r['old_name']}" if r["owner"] else r["old_name"]
                new = r["new_name"] or "-"
                print(f"{r['kind']:10} {r['address'] or '-':10} {qual}  ->  {new}  [{r['file']}]"
                      + (f"  9.6f {r['addr_96f']}" if r["addr_96f"] else "")
                      + (f"  decomp.me/scratch/{r['decompme']}" if r["decompme"] else ""))
    if not hits:
        print(f"no row for {key}", file=sys.stderr)
    return 0 if hits else 1


def strip_comments():
    removed = kept = 0
    with open(CSV_PATH) as f:
        rows = list(csv.DictReader(f))
    known_96f = {r["addr_96f"] for r in rows if r["addr_96f"]}
    known_scratch = {r["decompme"] for r in rows if r["decompme"]}

    def in_csv(body):
        m = COMMENT_96F.match(body)
        if m:
            return "0x" + m.group(1).upper() in known_96f
        m = COMMENT_DECOMPME.match(body)
        return bool(m) and m.group(1) in known_scratch

    for dirpath, _, files in os.walk(GAME):
        for name in files:
            if not name.endswith((".cpp", ".hpp", ".h", ".inl")):
                continue
            p = os.path.join(dirpath, name)
            text = open(p, newline="", errors="surrogateescape").read()
            keep = []
            for line in text.split("\n"):
                body = line.rstrip("\r")
                if (COMMENT_96F.match(body) or COMMENT_DECOMPME.match(body)) and in_csv(body):
                    removed += 1
                else:
                    if COMMENT_96F.match(body) or COMMENT_DECOMPME.match(body):
                        kept += 1
                    keep.append(line)
            if len(keep) != len(text.split("\n")):
                open(p, "w", newline="", errors="surrogateescape").write("\n".join(keep))
    print(f"{removed} comment lines removed, {kept} kept (not in the CSV)", file=sys.stderr)
    return 0


def selftest():
    assert address_from_name("ExitGame_4B8BD0") == "0x4B8BD0"
    assert address_from_name("gGame_0x40_67E008") == "0x67E008"
    assert address_from_name("kAng45_6F677C") == "0x6F677C"
    assert address_from_name("get_input_bits_4CEAC0") == "0x4CEAC0"
    assert address_from_name("Update_ABCDEF") == ""
    assert FIELD.match("field_2C_game_exit_type").group(1) == "2C"
    assert FIELD.match("field_75340_rec_buf_idx").group(1) == "75340"
    assert TYPE_SIZE.search("Car_BC").group(1) == "BC" and TYPE_SIZE.search("kanji_0x2").group(1) == "0x2"
    assert not TYPE_SIZE.search("Sprite")
    assert COMMENT_96F.match("    // 9.6f 0x4A50B0") and COMMENT_96F.match("// FUNCTION: 96f 0x4A50B0")
    assert not COMMENT_96F.match("// 9.6f inline") and not COMMENT_96F.match("// 9.6f calls Ang16::PolarToCartesian_41FC20")
    assert COMMENT_DECOMPME.match("// https://decomp.me/scratch/AbC12")
    assert COMMENT_ADDR.match("// 0x462e70")
    print("selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--init", metavar="BUILD_DIR", help="build address_map.csv (only before the first rename: it reads the current names)")
    g.add_argument("--symbols", metavar="BUILD_DIR", help="refresh symbol_names.csv (every declared name, for collision checks)")
    g.add_argument("--fix-owners", metavar="BUILD_DIR", help="fill empty owners (class template members) from a scan")
    g.add_argument("--lookup", metavar="KEY")
    g.add_argument("--strip-comments", action="store_true")
    g.add_argument("--selftest", action="store_true")
    ap.add_argument("-j", type=int, default=4)
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    if a.init:
        return init(os.path.realpath(a.init), a.j)
    if a.fix_owners:
        return fix_owners(os.path.realpath(a.fix_owners), a.j)
    if a.symbols:
        return init(os.path.realpath(a.symbols), a.j, symbols_only=True)
    if a.lookup:
        return lookup(a.lookup)
    return strip_comments()


if __name__ == "__main__":
    sys.exit(main())
