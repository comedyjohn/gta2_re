#!/usr/bin/env python3
"""One step of the phase-2 loop (EXECUTOR.md R2.x): scan, propose, apply the hand-made names, rename.

    python3 port/tools/refactor/name_step.py SUBSYSTEM KIND             # scan + propose + overrides; stops at TODOs
    python3 port/tools/refactor/name_step.py SUBSYSTEM KIND --apply     # then rename.py, address_map.csv, clang-format
    flags: --no-scan (reuse build_port/symbols.json)  --templates / --residual (passed to rename.py)

KIND is a propose.py kind: types functions members enumerators globals params locals.

port/refactor/names/SUBSYSTEM-KIND.overrides.csv (`old,new`) holds the names decided by hand: they replace the
proposal for that `old` and add rows the rules never produce. The merged result is written to
port/refactor/names/SUBSYSTEM-KIND[-N].csv (N counts repeated batches), which is the record of what was
renamed. The step stops while a row has no new name. Run port/tools/refactor/check.sh and commit afterwards.
"""
import csv
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.realpath(os.path.join(HERE, "..", "..", ".."))
NAMES = os.path.join(ROOT, "port", "refactor", "names")
SYMBOLS = os.path.join(ROOT, "build_port", "symbols.json")
MAP_PATH = os.path.join(HERE, "address_map.csv")
PY = sys.executable
sys.path.insert(0, HERE)
import propose  # noqa: E402

MAP_KIND = {"type": "type", "alias": "type", "function": "function", "method": "method", "field": "member",
            "static_member": "global", "global": "global", "enumerator": "enumerator", "namespace": "namespace"}


def committed(path):
    return subprocess.run(["git", "ls-files", "--error-unmatch", path], capture_output=True, cwd=ROOT).returncode == 0


def update_map(pairs, syms_by_qual):
    """Record old -> new in address_map.csv (adding a row for a name the map did not have)."""
    with open(MAP_PATH, newline="") as f:
        rows = list(csv.DictReader(f))
    cols = list(rows[0].keys())
    index = {}
    for r in rows:
        index[((r["new_owner"] or r["owner"]), (r["new_name"] or r["old_name"]))] = r
        index.setdefault((r["owner"], r["old_name"]), r)
    for old, new in pairs:
        owner, _, name = old.rpartition("::")
        r = index.get((owner, name))
        if r is None:
            s = syms_by_qual.get(old)
            if s is None:
                continue
            r = dict.fromkeys(cols, "")
            r.update(kind=MAP_KIND.get(s["kind"], s["kind"]), owner=owner, old_name=name, file=s["file"])
            rows.append(r)
        r["new_name"] = new
        index[(owner, new)] = r
    # a renamed type or namespace renames the owner part of everything inside it
    renamed = {}
    for r in rows:
        if r["kind"] in ("type", "namespace") and r["new_name"]:
            renamed[(r["owner"] + "::" if r["owner"] else "") + r["old_name"]] = r["new_name"]
    for r in rows:
        comps = r["owner"].split("::") if r["owner"] else []
        out = [renamed.get("::".join(comps[: i + 1]), c) for i, c in enumerate(comps)]
        if out != comps:
            r["new_owner"] = "::".join(out)
    with open(MAP_PATH, "w", newline="") as f:
        w = csv.DictWriter(f, cols, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flags = [a for a in sys.argv[1:] if a.startswith("--")]
    if len(args) != 2:
        sys.exit(__doc__)
    sub, kind = args
    os.makedirs(NAMES, exist_ok=True)
    if "--no-scan" not in flags:
        subprocess.check_call([PY, os.path.join(HERE, "symbols.py"), os.path.join(ROOT, "build_port", "clang64")])
    syms = json.load(open(SYMBOLS))
    rows = {k: [new, why, name] for k, new, why, name in propose.propose(sub, kind, syms)}
    ov_path = os.path.join(NAMES, f"{sub}-{kind}.overrides.csv")
    if os.path.exists(ov_path):
        with open(ov_path, newline="") as f:
            for r in csv.reader(f):
                if r and r[0] != "old" and not r[0].startswith("#"):
                    rows[r[0]] = [r[1], "", r[0]]
    final = os.path.join(NAMES, f"{sub}-{kind}.csv")
    n = 1
    while committed(final):  # a record that was already applied stays; a new batch gets the next file
        n += 1
        final = os.path.join(NAMES, f"{sub}-{kind}-{n}.csv")
    with open(final, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["old", "new", "name"])
        for old in sorted(rows):
            w.writerow([old, rows[old][0], rows[old][2]])
    todo = [(k, v[1], v[2]) for k, v in sorted(rows.items()) if not v[0]]
    print(f"{len(rows)} names, {len(todo)} without a new name -> {os.path.relpath(final, ROOT)}", file=sys.stderr)
    for k, why, name in todo:
        print(f"TODO {k if not k.startswith('usr:') else name}: {why}")
    if todo or "--apply" not in flags:
        return 1 if todo else 0
    extra = [f for f in flags if f in ("--templates", "--residual")]
    rc = subprocess.call([PY, os.path.join(HERE, "rename.py"), final, "-j", "8", *extra])
    if rc:
        return rc
    by_qual = {propose.qual(s): s for s in syms}
    update_map([(k, v[0]) for k, v in rows.items() if not k.startswith("usr:")], by_qual)
    changed = subprocess.run(["git", "diff", "--name-only", "--diff-filter=ACMR", "--", "port/game", "port/platform", "port/tests"],
                             capture_output=True, text=True, cwd=ROOT).stdout.split()
    changed = [os.path.join(ROOT, c) for c in changed if c.endswith((".cpp", ".hpp", ".h")) and "/3rdParty/" not in c]
    fmt = os.path.expanduser("~/.venvs/gta2-refactor/bin/clang-format")
    if changed:
        subprocess.check_call([fmt, "-i", *changed])
    return 0


if __name__ == "__main__":
    sys.exit(main())
