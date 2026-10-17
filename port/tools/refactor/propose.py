#!/usr/bin/env python3
"""Mechanical new names for rename.py (port/refactor/2-names.md 2.3, task R2.4).

    python3 port/tools/refactor/propose.py SUBSYSTEM KIND [-o renames.csv]
    python3 port/tools/refactor/propose.py --selftest

SUBSYSTEM is one of subsystems.py (or `all`); KIND is one of
    types       strip the size suffix (`Garage_48` -> `Garage`), PascalCase
    functions   functions and methods: strip the address, PascalCase (`get_input_bits_4CEAC0` -> `GetInputBits`)
    members     strip `field_<hex>_`, drop the Hungarian prefix, snake_case (`field_4_pPrev` -> `prev`)
    enumerators strip `_<digits>`, add `k`, PascalCase (`Running_1` -> `kRunning`)
    constants   `k` globals: strip the address (`kAng45_6F677C` -> `kAng45`)
    globals     `g` globals: strip address and size (`gGame_0x40_67E008` -> `gGame`); `b` debug flags -> `gDebug...`

It reads address_map.csv, so a symbol already renamed (a `new_name`) is skipped, and the qualified old name
of a member is written with its owner's current name (types are renamed first). Output columns:
`old,new,todo`. A row the rules can't decide has an empty `new` and a reason in `todo`: a collision in its
scope, an empty result, a name made only of a placeholder (`sub_4C9ED0`, an unnamed `field_2C`), a C++
keyword. rename.py refuses a CSV with an empty `new`; fill the rows in by hand.

Members get no trailing underscore yet: they are public until phase 6 gives each class its interface,
and `PublicMemberCase` is `lower_case` (STYLE.md); phase 6 adds the `_` when it makes them private.
"""
import argparse
import collections
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from subsystems import SUBSYSTEMS  # noqa: E402

MAP_PATH = os.path.join(HERE, "address_map.csv")
SYMBOLS_PATH = os.path.join(HERE, "symbol_names.csv")
KEYWORDS = set(
    """alignas alignof and and_eq asm auto bitand bitor bool break case catch char char8_t char16_t char32_t class
    compl concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default
    delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long
    mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public register
    reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this
    thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor
    xor_eq""".split()
)
ADDR_SUFFIX = re.compile(r"_(?:[0-9A-Fa-f]{6})(?=_|$)")
SIZE_SUFFIX = re.compile(r"_(?:0x[0-9A-Fa-f]+|[0-9A-F]{1,5})$")
INLINE_TAG = re.compile(r"_(?:inline|ool|OOL|no_budget\d*)(?=_|$)")
HUNGARIAN = re.compile(r"^(?:pp|p|b|n|u|dw|w|i|f|sz|e)(?=[A-Z0-9])")


def pascal(name):
    """get_input_bits -> GetInputBits; GetInputBits stays; keeps digits and inner capitals."""
    parts = [p for p in name.split("_") if p]
    return "".join(p[0].upper() + p[1:] for p in parts)


def snake(name):
    """pCurrentPlayer -> current_player; rec_buf_idx stays; FixedHUD -> fixed_hud."""
    name = HUNGARIAN.sub("", name)
    s = re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", name)
    s = re.sub(r"(?<=[A-Z])(?=[A-Z][a-z])", "_", s)
    return re.sub(r"_+", "_", s).strip("_").lower()


def strip_function(name):
    n = ADDR_SUFFIX.sub("", name)
    n = INLINE_TAG.sub("", n)
    return n


def propose_function(name):
    if name.startswith(("sub_", "operator")) or "~" in name:
        return None, "placeholder or operator"
    n = strip_function(name)
    if not n or n.isdigit():
        return None, "empty after stripping the address"
    return pascal(n), ""


def propose_member(name):
    m = re.match(r"^field_[0-9A-Fa-f]+(?:_(.*))?$", name)
    if not m:
        return None, "not a field_ name"
    rest = m.group(1) or ""
    if not rest:
        return None, "unnamed field"
    new = snake(rest)
    if not new or new[0].isdigit():
        return None, "no usable name left"
    return new, ""


def propose_type(name):
    n = SIZE_SUFFIX.sub("", name)
    if n == name:
        n = name
    if not n:
        return None, "empty"
    return pascal(n) if "_" in n or n[0].islower() else n, ""


def propose_enumerator(name):
    n = re.sub(r"_\d+$", "", name)
    if re.match(r"^k[A-Z]", n):
        return n, ""
    if not n:
        return None, "empty"
    return "k" + pascal(n), ""


def propose_constant(name):
    n = ADDR_SUFFIX.sub("", name)
    n = SIZE_SUFFIX.sub("", n) if re.search(r"_0x[0-9A-Fa-f]+$", n) else n
    if not re.match(r"^k[A-Z]", n):
        return None, "not a k constant"
    return n, ""


def propose_global(name):
    n = ADDR_SUFFIX.sub("", name)
    n = re.sub(r"_0x[0-9A-Fa-f]+", "", n)
    if re.match(r"^(byte|word|dword|unk|flt|off|stru|stru_|dbl|str|asc|wasc)_", name):
        return None, "raw data placeholder"
    if re.match(r"^g[A-Z]", n):
        return "g" + pascal(n[1:]), ""
    if re.match(r"^b[A-Z]", n):
        return "gDebug" + pascal(n[1:]), ""
    return None, "unclear prefix"


def subsystem_of(file):
    stem = os.path.basename(file).rsplit(".", 1)[0]
    for sub, stems in SUBSYSTEMS.items():
        if stem in stems:
            return sub
    return "?"


def read_rows():
    with open(MAP_PATH, newline="") as f:
        return list(csv.DictReader(f))


def propose(sub, kind, rows):
    types = {r["old_name"]: r for r in rows if r["kind"] == "type"}
    new_owner = {r["old_name"]: r["new_name"] for r in types.values() if r["new_name"]}

    def current_owner_chain(owner):
        chain = []
        while owner:
            chain.append(new_owner.get(owner, owner))
            owner = types[owner]["owner"] if owner in types else ""
        return list(reversed(chain))

    def current_qual(r):
        return "::".join(current_owner_chain(r["owner"]) + [r["new_name"] or r["old_name"]])

    selected = []
    for r in rows:
        if sub != "all" and subsystem_of(r["file"]) != sub:
            continue
        k = r["kind"]
        if r["new_name"]:
            continue
        if kind == "types" and k == "type":
            selected.append((r, *propose_type(r["old_name"])))
        elif kind == "functions" and k in ("function", "method"):
            if r["old_name"] in (r["owner"], "~" + r["owner"]) or r["old_name"].startswith(("~", "operator")):
                continue
            selected.append((r, *propose_function(r["old_name"])))
        elif kind == "members" and k == "member":
            selected.append((r, *propose_member(r["old_name"])))
        elif kind == "enumerators" and k == "enumerator":
            selected.append((r, *propose_enumerator(r["old_name"])))
        elif kind == "constants" and k == "global" and re.match(r"^k[A-Z]", r["old_name"]):
            selected.append((r, *propose_constant(r["old_name"])))
        elif kind == "globals" and k == "global" and not re.match(r"^k[A-Z]", r["old_name"]):
            selected.append((r, *propose_global(r["old_name"])))
    # names already taken in each scope (current names), then collisions among the proposals
    def cur_owner(r):
        return r["new_owner"] or r["owner"]

    taken = collections.defaultdict(set)
    if os.path.exists(SYMBOLS_PATH):  # every name the code declares now, with its owner as spelled now
        with open(SYMBOLS_PATH, newline="") as f:
            for sr in csv.DictReader(f):
                taken[sr["owner"]].add(sr["name"])
    for r in rows:
        scope = cur_owner(r) if r["kind"] in ("member", "method", "enumerator") else ""
        if r["kind"] == "type" and r["owner"]:
            scope = cur_owner(r)
        taken[scope].add(r["new_name"] or r["old_name"])
    out, counts = [], collections.defaultdict(list)
    for r, new, why in selected:
        scope = cur_owner(r) if r["kind"] in ("member", "method", "enumerator") else ""
        if r["kind"] == "type":
            scope = cur_owner(r)
        if new:
            if new in KEYWORDS:
                new, why = None, f"{new} is a C++ keyword"
            elif r["kind"] != "type" and not re.match(r"^[A-Za-z_]\w*$", new):
                new, why = None, "not an identifier"
        out.append([r, new, why, scope])
        if new:
            counts[(scope, new)].append(r["old_name"])
    pending = {(s, o["old_name"]) for o, n, w, s in map(tuple, out)}
    for o in out:
        r, new, why, scope = o
        if not new:
            continue
        clash = counts[(scope, new)]
        if len(set(clash)) > 1 and r["kind"] == "enumerator":
            # Unknown_0, Unknown_1, ...: the value stays in the name
            m = re.search(r"_(\d+)$", r["old_name"])
            o[1], o[2] = (new + m.group(1), "") if m else (None, "collides and has no value suffix")
        elif len(set(clash)) > 1:
            o[1], o[2] = None, "collides with " + ", ".join(sorted(set(clash) - {r["old_name"]}))
        elif new in taken[scope] and new != r["old_name"] and (scope, new) not in pending:
            o[1], o[2] = None, f"{new} already exists in {scope or 'the global scope'}"
    result = []
    for r, new, why, scope in out:
        result.append((current_qual(r), new or "", why))
    return sorted(set(result))


def selftest():
    f = propose_function
    assert f("get_input_bits_4CEAC0") == ("GetInputBits", "")
    assert f("ExitGame_4B8BD0") == ("ExitGame", "")
    assert f("PoolAllocate") == ("PoolAllocate", "")
    assert f("sub_4C9ED0")[0] is None
    assert f("Draw_4B8BD0_inline") == ("Draw", "")
    m = propose_member
    assert m("field_2C_game_exit_type") == ("game_exit_type", "")
    assert m("field_0_pHead") == ("head", "")
    assert m("field_75340_rec_buf_idx") == ("rec_buf_idx", "")
    assert m("field_4_pPrev") == ("prev", "")
    assert m("field_18_bShutDown") == ("shut_down", "")
    assert m("field_2C")[0] is None
    assert propose_type("Garage_48") == ("Garage", "")
    assert propose_type("Game_0x40") == ("Game", "")
    assert propose_type("sound_obj")[0] == "SoundObj"
    assert propose_enumerator("Running_1") == ("kRunning", "")
    assert propose_enumerator("none_0") == ("kNone", "")
    assert propose_constant("kAng45_6F677C") == ("kAng45", "")
    assert propose_global("gGame_0x40_67E008") == ("gGame", "")
    assert propose_global("bDo_mike_67D5CC") == ("gDebugDoMike", "")
    assert propose_global("dword_6F1234")[0] is None
    assert snake("pCurrentPlayer") == "current_player" and snake("FixedHUD") == "fixed_hud"
    print("selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("subsystem", nargs="?")
    ap.add_argument("kind", nargs="?")
    ap.add_argument("-o", "--output")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    if not (a.subsystem and a.kind):
        ap.error("SUBSYSTEM and KIND are required")
    if a.subsystem != "all" and a.subsystem not in SUBSYSTEMS:
        ap.error("subsystem must be one of " + ", ".join(SUBSYSTEMS))
    rows = propose(a.subsystem, a.kind, read_rows())
    out = open(a.output, "w", newline="") if a.output else sys.stdout
    w = csv.writer(out, lineterminator="\n")
    w.writerow(["old", "new", "todo"])
    w.writerows(rows)
    todo = sum(1 for r in rows if not r[1])
    print(f"{len(rows)} proposals, {todo} TODO", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
