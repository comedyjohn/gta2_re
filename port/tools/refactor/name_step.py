#!/usr/bin/env python3
"""One step of the phase-2 loop (EXECUTOR.md R2.x): propose, apply the hand-made overrides, rename.

    python3 port/tools/refactor/name_step.py SUBSYSTEM KIND            # propose + overrides, stops at TODOs
    python3 port/tools/refactor/name_step.py SUBSYSTEM KIND --apply    # then rename.py and clang-format

port/refactor/names/SUBSYSTEM-KIND.overrides.csv (`old,new`) holds the names decided by hand: they replace
the proposal for that `old` (and add rows the rules never produce). The merged result is written to
port/refactor/names/SUBSYSTEM-KIND.csv, which is the record of what was renamed. The step stops while a
row has no new name. Run port/tools/refactor/check.sh and commit afterwards.
"""
import csv
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.realpath(os.path.join(HERE, "..", "..", ".."))
NAMES = os.path.join(ROOT, "port", "refactor", "names")
sys.path.insert(0, HERE)
import propose  # noqa: E402


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    apply_ = "--apply" in sys.argv
    if len(args) != 2:
        sys.exit(__doc__)
    sub, kind = args
    os.makedirs(NAMES, exist_ok=True)
    if "--no-symbols" not in sys.argv:
        subprocess.check_call([sys.executable, os.path.join(HERE, "address_map.py"), "--symbols", os.path.join(ROOT, "build_port", "clang64")])
    rows = {old: [new, why] for old, new, why in propose.propose(sub, kind, propose.read_rows())}
    ov_path = os.path.join(NAMES, f"{sub}-{kind}.overrides.csv")
    if os.path.exists(ov_path):
        for r in csv.reader(open(ov_path, newline="")):
            if r and r[0] != "old" and not r[0].startswith("#"):
                rows[r[0]] = [r[1], ""]
    final = os.path.join(NAMES, f"{sub}-{kind}.csv")
    with open(final, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["old", "new"])
        for old in sorted(rows):
            w.writerow([old, rows[old][0]])
    todo = [(o, v[1]) for o, v in sorted(rows.items()) if not v[0]]
    print(f"{len(rows)} names, {len(todo)} without a new name", file=sys.stderr)
    for o, why in todo:
        print(f"TODO {o}: {why}")
    if todo or not apply_:
        return 1 if todo else 0
    rc = subprocess.call([sys.executable, os.path.join(HERE, "rename.py"), final, "-j", "8"] + sys.argv[3:] if False else
                         [sys.executable, os.path.join(HERE, "rename.py"), final, "-j", "8"] + [a for a in sys.argv[1:] if a.startswith("--") and a not in ("--apply", "--no-symbols")])
    if rc:
        return rc
    changed = subprocess.run(["git", "diff", "--name-only", "--", "port/game", "port/platform", "port/tests"],
                             capture_output=True, text=True, cwd=ROOT).stdout.split()
    changed = [os.path.join(ROOT, c) for c in changed if c.endswith((".cpp", ".hpp", ".h"))]
    fmt = os.path.expanduser("~/.venvs/gta2-refactor/bin/clang-format")
    if changed:
        subprocess.check_call([fmt, "-i", *changed])
    return 0


if __name__ == "__main__":
    sys.exit(main())
