#!/usr/bin/env python3
"""Mechanical new names for rename.py (port/refactor/2-names.md 2.3, task R2.4).

    python3 port/tools/refactor/propose.py SUBSYSTEM KIND [-o renames.csv] [--symbols build_port/symbols.json]
    python3 port/tools/refactor/propose.py --selftest

SUBSYSTEM is one of subsystems.py (or `all`); KIND is one of
    types        classes, structs, unions, enums, aliases and the namespaces that stand in for enums
    functions    functions and methods (not constructors, destructors or operators)
    members      data members (and static members)
    enumerators  enumerators (`kPascalCase`)
    globals      global variables: `kPascalCase` constants, `g` + `PascalCase` the others
    params       function parameters
    locals       local variables

It reads symbols.json (symbols.py), proposes a name for every symbol of the subsystem that does not follow
port/STYLE.md yet, and writes `old,new,todo,name`: `old` is the qualified name as the code spells it now (or
`usr:...` for params and locals), `new` is empty with a reason in `todo` when the rules can't decide
(placeholders such as `sub_4C9ED0`, raw data globals, an unnamed `field_2C`, keywords, collisions). rename.py
refuses a CSV with an empty `new`; fill those rows in by hand (name_step.py's overrides file).

Members of a class or struct that declares member functions get a trailing underscore (`left_`): its
constructors' and setters' parameters are called `left` too, and -Wshadow is an error. Members of a plain
data struct do not. See STYLE.md.
"""
import argparse
import collections
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.realpath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, HERE)
from subsystems import SUBSYSTEMS  # noqa: E402

SYMBOLS = os.path.join(ROOT, "build_port", "symbols.json")
KEYWORDS = set(
    """alignas alignof and and_eq asm auto bitand bitor bool break case catch char char8_t char16_t char32_t class
    compl concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default
    delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long
    mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public register
    reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this
    thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor
    xor_eq NULL EOF""".split()
)
ADDR_SUFFIX = re.compile(r"_(?:[0-9A-Fa-f]{6})(?=_|$)")
SIZE_SUFFIX = re.compile(r"_(?:0x[0-9A-Fa-f]+|[0-9A-F]{1,5})$")
INLINE_TAG = re.compile(r"_(?:inline|ool|OOL|no_budget\d*)(?=_|$)")
HUNGARIAN = re.compile(r"^(?:mp|pp|p|b|n|u|dw|w|i|f|sz|e|m)(?=[A-Z0-9])")

RE_TYPE = re.compile(r"^[A-Z][A-Za-z0-9]*$")
RE_SNAKE = re.compile(r"^[a-z][a-z0-9]*(_[a-z0-9]+)*$")
RE_SNAKE_US = re.compile(r"^[a-z][a-z0-9]*(_[a-z0-9]+)*_$")
RE_K = re.compile(r"^k[A-Z][A-Za-z0-9]*$")
RE_G = re.compile(r"^g[A-Z][A-Za-z0-9]*$")
# the project's fixed-width and char aliases (types.hpp) keep their names until phase 3
KEEP_ALIASES = {"s8", "s16", "s32", "s64", "u8", "u16", "u32", "u64", "f32", "f64", "char_type", "wchar_type", "uptr", "sptr",
                "size_t", "ptrdiff_t", "uintptr_t", "intptr_t", "wchar_t"}
# names a local or parameter must not take: a library function or macro it would hide or break
LIBRARY_NAMES = set("""sin cos tan asin acos atan atan2 sinh cosh tanh sqrt pow exp log log10 floor ceil fabs abs labs fmod ldexp
    frexp modf time clock rand srand free malloc calloc realloc exit abort atoi atof strlen strcpy strcmp strcat
    memcpy memset memcmp memmove printf sprintf snprintf sscanf fopen fclose fread fwrite fseek ftell remove rename
    signal select index rindex stat access open close read write link unlink send recv bind listen accept connect
    socket div ldiv bsearch qsort errno stdin stdout stderr""".split())
SKIP_FUNCTIONS = {"main", "WinMain", "DllMain"}
# the components of a point or a vector are its interface: they keep their names (no trailing underscore)
COORDINATES = {"x", "y", "z", "w"}


def pascal(name):
    """get_input_bits -> GetInputBits; GetInputBits stays; keeps digits and inner capitals."""
    parts = [p for p in name.split("_") if p]
    if name.upper() == name and any(c.isalpha() for c in name):  # MAX_PLAYERS -> MaxPlayers
        parts = [p.lower() for p in parts]
    return "".join(p[0].upper() + p[1:] for p in parts)


def snake(name):
    """pCurrentPlayer -> current_player; rec_buf_idx stays; FixedHUD -> fixed_hud."""
    name = HUNGARIAN.sub("", name)
    s = re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", name)
    s = re.sub(r"(?<=[A-Z])(?=[A-Z][a-z])", "_", s)
    return re.sub(r"_+", "_", s).strip("_").lower()


def strip_address(name):
    n = ADDR_SUFFIX.sub("", name)
    return INLINE_TAG.sub("", n)


def propose_function(name):
    if name.startswith("sub_"):
        return None, "placeholder"
    if name in SKIP_FUNCTIONS:
        return name, ""
    n = strip_address(name)
    if not n or n.isdigit():
        return None, "empty after stripping the address"
    return pascal(n), ""


def propose_member(name, with_underscore):
    m = re.match(r"^field_[0-9A-Fa-f]+(?:_(.*))?$", name)
    rest = (m.group(1) or "") if m else name
    if m and not rest:
        return None, "unnamed field"
    new = snake(rest.rstrip("_") if not m else rest)
    if not new or new[0].isdigit():
        return None, "no usable name left"
    return (new + "_" if with_underscore else new), ""


def propose_type(name):
    n = SIZE_SUFFIX.sub("", name)
    if name.startswith("T_"):
        n = n[2:]
    if not n:
        return None, "empty"
    return (pascal(n) if "_" in n or n[0].islower() else n), ""


def propose_enumerator(name):
    n = re.sub(r"_\d+$", "", name)
    if re.match(r"^k[A-Z]", n):
        return n, ""
    if not n:
        return None, "empty"
    return "k" + pascal(n), ""


def propose_global(name, const):
    n = ADDR_SUFFIX.sub("", name)
    n = re.sub(r"_0x[0-9A-Fa-f]+", "", n)
    if re.match(r"^(byte|word|dword|unk|flt|off|stru|dbl|str|asc|wasc|sub|loc)_", name):
        return None, "raw data placeholder"
    if re.match(r"^k[A-Z]", n):  # a constant by its name, whether or not the type says const
        return n, ""
    if const:
        return "k" + pascal(re.sub(r"^k_?", "", n)), ""
    if re.match(r"^g[A-Z]", n):
        return "g" + pascal(n[1:]), ""
    if re.match(r"^gp[A-Z]", n):
        return "g" + pascal(n[2:]), ""
    if re.match(r"^b[A-Z]", n):
        return "gDebug" + pascal(n[1:]), ""
    return "g" + pascal(HUNGARIAN.sub("", n)), ""


def propose_local(name):
    n = snake(name)
    if not n or n[0].isdigit():
        return None, "no usable name"
    return n, ""


def overload_sig(sig):
    """The parameter types as overload resolution tells them apart: `s8` and `const s8&` are not different."""
    inner, _, tail = sig.rpartition(")")
    inner = re.sub(r"\bconst\b|\bvolatile\b|&", "", inner)
    return re.sub(r"\s+", "", inner) + ")" + tail


def subsystem_of(file):
    stem = os.path.basename(file).rsplit(".", 1)[0]
    for sub, stems in SUBSYSTEMS.items():
        if stem in stems:
            return sub
    return "?"


def qual(s):
    return (s["owner"] + "::" if s["owner"] else "") + s["name"]


def scope_of(s):
    """Where a name must be unique. The parameters of a function's declaration and of its definition are different
    declarations (the names may differ), so a parameter's scope includes the file."""
    return s["owner"] + "@" + s["file"] if s["kind"] in ("param", "local") else s["owner"]


def key_of(s):
    return ("usr:" + s["usr"]) if s["kind"] in ("param", "local") else qual(s)


def wanted_symbols(sub, kind, syms, names=None):
    """(symbol, proposed name, reason) for every symbol of the kind that does not follow the style yet."""
    out = []
    for s in syms:
        if sub != "all" and subsystem_of(s["file"]) != sub:
            continue
        k, name = s["kind"], s["name"]
        if names and name not in names:
            continue
        if kind == "types" and k in ("type", "alias"):
            if k == "alias" and (name in KEEP_ALIASES or s["file"].endswith("types.hpp")):
                continue
            if RE_TYPE.match(name):
                continue
            out.append((s, *propose_type(name)))
        elif kind == "types" and k == "namespace":
            if RE_TYPE.match(name) or RE_SNAKE.match(name):
                continue
            n = re.sub(r"_enum$", "", name)
            out.append((s, pascal(n) if "_" in n or n[0].islower() else n, ""))
        elif kind == "functions" and k in ("function", "method"):
            if s["operator"] or RE_TYPE.match(name) or name.startswith("~") or name in SKIP_FUNCTIONS:
                continue
            out.append((s, *propose_function(name)))
        elif kind == "members" and k in ("field", "static_member"):
            if k == "static_member" and s.get("const"):
                if RE_K.match(name):
                    continue
                out.append((s, *propose_global(name, True)))
                continue
            us = s.get("has_methods", True) if k == "field" else True
            if name in COORDINATES or (RE_SNAKE_US if us else RE_SNAKE).match(name):
                continue
            out.append((s, *propose_member(name, us)))
        elif kind == "enumerators" and k == "enumerator":
            if RE_K.match(name):
                continue
            out.append((s, *propose_enumerator(name)))
        elif kind == "globals" and k == "global":
            ok = RE_K if (s.get("const") or name.startswith("k")) else RE_G
            if ok.match(name) and not ADDR_SUFFIX.search(name):
                continue
            out.append((s, *propose_global(name, bool(s.get("const")))))
        elif kind == "params" and k == "param":
            if RE_SNAKE.match(name):
                continue
            out.append((s, *propose_local(name)))
        elif kind == "locals" and k == "local":
            if RE_SNAKE.match(name):
                continue
            out.append((s, *propose_local(name)))
    return out


def propose(sub, kind, syms, names=None):
    chosen = wanted_symbols(sub, kind, syms, names)
    taken = collections.defaultdict(set)  # scope -> names in use now
    for s in syms:
        taken[s["owner"]].add(s["name"])
    rows = []
    for s, new, why in chosen:
        if new and new in KEYWORDS:
            new, why = None, f"{new} is a C++ keyword"
        elif new and s["kind"] in ("param", "local") and (new in LIBRARY_NAMES or new in taken[""]):
            new, why = None, f"{new} would hide a library or global name"
        rows.append([s, new, why])
    fn_sigs = collections.defaultdict(set)  # (scope, name) -> parameter types of the functions called so
    for s in syms:
        if s["kind"] in ("function", "method"):
            fn_sigs[(s["owner"], s["name"])].add(overload_sig(s.get("sig", "")))
    batch_names = collections.defaultdict(set)  # scope -> old names renamed in this batch
    for s, _, _ in rows:
        batch_names[s["owner"]].add(s["name"])
    for _ in range(2):  # the digit fallback can collide again
        counts = collections.defaultdict(set)
        for s, new, why in rows:
            if new:
                counts[(scope_of(s), new)].add(s["name"])
        for r in rows:
            s, new, why = r
            if not new:
                continue
            clash = counts[(scope_of(s), new)]
            if len(clash) > 1 and s["kind"] in ("function", "method"):
                # same name, different parameter types: an overload set, if no two are alike
                group = [x for x in rows if x[1] == new and x[0]["owner"] == s["owner"] and x[0]["kind"] in ("function", "method")]
                by_name = {x[0]["name"]: overload_sig(x[0].get("sig", "")) for x in group}  # a declaration and its definition share a name
                if len(set(by_name.values())) == len(by_name):
                    continue
                r[1], r[2] = None, "collides with " + ", ".join(sorted(clash - {s["name"]})) + " (same parameter types)"
            elif len(clash) > 1:
                m = re.search(r"_(\d+)$", s["name"])
                if s["kind"] == "enumerator" and m and not new.endswith(m.group(1)):
                    r[1], r[2] = new + m.group(1), ""
                else:
                    r[1], r[2] = None, "collides with " + ", ".join(sorted(clash - {s["name"]}))
            elif new in taken[s["owner"]] and new != s["name"] and new not in batch_names[s["owner"]]:
                existing = fn_sigs.get((s["owner"], new), set())
                other_kinds = any(x["name"] == new and x["owner"] == s["owner"] and x["kind"] not in ("function", "method") for x in syms)
                if s["kind"] in ("function", "method") and existing and not other_kinds and overload_sig(s.get("sig", "")) not in existing:
                    continue  # an overload of a function that is already there
                r[1], r[2] = None, f"{new} already exists in {s['owner'] or 'the global scope'}"
    result = {}
    for s, new, why in rows:
        key = key_of(s)
        if key in result and result[key][0] != (new or ""):
            result[key] = ["", "overloads would get different names", qual(s)]
        else:
            result[key] = [new or "", why, qual(s)]
    return sorted((k, v[0], v[1], v[2]) for k, v in result.items())


def selftest():
    f = propose_function
    assert f("get_input_bits_4CEAC0") == ("GetInputBits", "")
    assert f("ExitGame_4B8BD0") == ("ExitGame", "")
    assert f("sub_4C9ED0")[0] is None
    assert f("Draw_4B8BD0_inline") == ("Draw", "")
    assert propose_member("field_2C_game_exit_type", True) == ("game_exit_type_", "")
    assert propose_member("field_0_pHead", False) == ("head", "")
    assert propose_member("field_4_pPrev", True) == ("prev_", "")
    assert propose_member("mValue", True) == ("value_", "")
    assert propose_member("mpNext", True) == ("next_", "")
    assert propose_member("field_2C", True)[0] is None
    assert pascal("MAX_PLAYERS") == "MaxPlayers" and pascal("DMA_Video") == "DMAVideo"
    assert propose_type("Garage_48") == ("Garage", "")
    assert propose_type("sound_obj")[0] == "SoundObj"
    assert propose_type("T_gbh_Init")[0] == "GbhInit"
    assert propose_enumerator("Running_1") == ("kRunning", "")
    assert propose_enumerator("ped_walking") == ("kPedWalking", "")
    assert propose_global("kAng45_6F677C", True) == ("kAng45", "")
    assert propose_global("gGame_0x40_67E008", False) == ("gGame", "")
    assert propose_global("bDo_mike_67D5CC", False) == ("gDebugDoMike", "")
    assert propose_global("gpRng_67AB34", False) == ("gRng", "")
    assert propose_global("dword_6F1234", False)[0] is None
    assert propose_global("kFpZero_6F610C", False) == ("kFpZero", "")
    assert propose_global("tmpBuff_67BD9C", False) == ("gTmpBuff", "")
    assert propose_local("pCurrentPlayer") == ("current_player", "")
    assert propose_local("pInX") == ("in_x", "")
    assert snake("pCurrentPlayer") == "current_player" and snake("FixedHUD") == "fixed_hud"
    assert RE_SNAKE_US.match("left_") and not RE_SNAKE_US.match("left") and RE_SNAKE.match("rng_index")
    print("selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("subsystem", nargs="?")
    ap.add_argument("kind", nargs="?")
    ap.add_argument("-o", "--output")
    ap.add_argument("--symbols", default=SYMBOLS)
    ap.add_argument("--names", help="only these names, comma separated (a batch that must change them together)")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    if not (a.subsystem and a.kind):
        ap.error("SUBSYSTEM and KIND are required")
    if a.subsystem != "all" and a.subsystem not in SUBSYSTEMS:
        ap.error("subsystem must be one of " + ", ".join(SUBSYSTEMS))
    syms = json.load(open(a.symbols))
    rows = propose(a.subsystem, a.kind, syms, set(a.names.split(",")) if a.names else None)
    out = open(a.output, "w", newline="") if a.output else sys.stdout
    w = csv.writer(out, lineterminator="\n")
    w.writerow(["old", "new", "todo", "name"])
    w.writerows(rows)
    todo = sum(1 for r in rows if not r[1])
    print(f"{len(rows)} proposals, {todo} TODO", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
