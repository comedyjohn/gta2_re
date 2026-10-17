#!/usr/bin/env python3
"""clang-tidy ratchet: the number of findings per (file, check) may only go down.

    python3 port/tools/refactor/tidy_ratchet.py --baseline build_port/clang64
    python3 port/tools/refactor/tidy_ratchet.py --check build_port/clang64 [files...]
    python3 port/tools/refactor/tidy_ratchet.py --selftest

--baseline runs clang-tidy (port/.clang-tidy) on every game/platform/tests TU of the build
directory's compile_commands.json (8 jobs, about 15 minutes) and writes port/refactor/tidy-baseline.csv
(file,check,count). A finding is counted once at its file:line:column, however many TUs show it.
--check runs on the given files, or on the ones that differ from master (plus the TUs that include a
changed header), and fails if a (file, check) count is above the baseline. It prints the lowered
counts; commit them with `--check ... --update`, which rewrites those rows of the baseline.
Use the pip clang-tidy of the refactor venv (EXECUTOR.md 1.2); $TIDY overrides the path.
"""
import argparse
import collections
import concurrent.futures
import csv
import json
import os
import re
import subprocess
import sys

ROOT = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASELINE = os.path.join(ROOT, "port", "refactor", "tidy-baseline.csv")
TIDY = os.environ.get("TIDY", os.path.expanduser("~/.venvs/gta2-refactor/bin/clang-tidy"))
DIAG = re.compile(r"^(/[^:]+):(\d+):(\d+): (?:warning|error): .*\[([\w.,-]+)\]$")
OURS = re.compile(r"/port/(game|platform|tests)/")


def translation_units(build_dir):
    db = json.load(open(os.path.join(build_dir, "compile_commands.json")))
    return sorted({e["file"] for e in db if OURS.search(e["file"]) and e["file"].endswith(".cpp")})


def run_tidy(args):
    build_dir, path = args
    out = subprocess.run([TIDY, "-p", build_dir, "--quiet", path], capture_output=True, text=True).stdout
    found = set()
    for line in out.splitlines():
        m = DIAG.match(line)
        if m and OURS.search(m.group(1)):
            for check in m.group(4).split(","):
                found.add((m.group(1), int(m.group(2)), int(m.group(3)), check))
    return found


def count(build_dir, paths, jobs=8):
    seen = set()
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        for found in pool.map(run_tidy, [(build_dir, p) for p in paths]):
            seen |= found
    counts = collections.Counter()
    for path, _, _, check in seen:
        counts[(os.path.relpath(path, ROOT), check)] += 1
    return counts


def read_baseline():
    if not os.path.exists(BASELINE):
        sys.exit(f"{BASELINE} does not exist: run --baseline first")
    with open(BASELINE) as f:
        return {(r["file"], r["check"]): int(r["count"]) for r in csv.DictReader(f)}


def write_baseline(counts):
    with open(BASELINE, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["file", "check", "count"])
        for (path, check), n in sorted(counts.items()):
            w.writerow([path, check, n])


def changed_files():
    out = subprocess.run(
        ["git", "diff", "--name-only", "master...HEAD"], capture_output=True, text=True, cwd=ROOT
    ).stdout.split()
    out += subprocess.run(["git", "diff", "--name-only"], capture_output=True, text=True, cwd=ROOT).stdout.split()
    return sorted({os.path.join(ROOT, f) for f in out if re.match(r"port/(game|platform|tests)/.*\.(cpp|hpp|h)$", f)})


def tus_for(build_dir, files):
    """The TUs to run for the given files: the .cpp files themselves, and the TUs including a header."""
    tus = set(translation_units(build_dir))
    chosen = {f for f in files if f.endswith(".cpp") and f in tus}
    headers = [os.path.basename(f) for f in files if not f.endswith(".cpp")]
    if headers:
        for tu in tus:
            text = open(tu, errors="replace").read()
            if any(re.search(r'#\s*include\s*[<"][^>"]*' + re.escape(h) + r'[>"]', text) for h in headers):
                chosen.add(tu)
    return sorted(chosen)


def check(build_dir, files, update):
    files = [os.path.realpath(f) for f in files] or changed_files()
    tus = tus_for(build_dir, files)
    if not tus:
        print("nothing to check")
        return 0
    base = read_baseline()
    counts = count(build_dir, tus)
    touched = {os.path.relpath(f, ROOT) for f in files} | {os.path.relpath(t, ROOT) for t in tus}
    worse, better = [], []
    for key in sorted(set(counts) | {k for k in base if k[0] in touched}):
        have, want = counts.get(key, 0), base.get(key, 0)
        # A header's findings are complete only when every TU that includes it ran: judge headers
        # only for rises, never for falls.
        complete = key[0].endswith(".cpp")
        if have > want:
            worse.append((key, want, have))
        elif have < want and complete and key[0] in touched:
            better.append((key, want, have))
    for (path, check_name), want, have in worse:
        print(f"RISE   {path}: {check_name}: {want} -> {have}")
    for (path, check_name), want, have in better:
        print(f"LOWER  {path}: {check_name}: {want} -> {have}")
    if update and better and not worse:
        for key, _, have in better:
            if have:
                base[key] = have
            else:
                base.pop(key, None)
        write_baseline(base)
        print(f"baseline lowered in {len(better)} rows")
    return 1 if worse else 0


def selftest():
    import tempfile

    with tempfile.TemporaryDirectory() as d:
        game = os.path.join(d, "port", "game")
        os.makedirs(game)
        src = os.path.join(game, "t.cpp")
        open(src, "w").write("int f(int* p) { return (long)p > 0 ? 1 : 0; }\nint g(int x) { return x; }\n")
        cfg = os.path.join(d, ".clang-tidy")
        open(cfg, "w").write("Checks: '-*,google-readability-casting'\nHeaderFilterRegex: '.*'\n")
        db = os.path.join(d, "build")
        os.makedirs(db)
        json.dump(
            [{"directory": d, "file": src, "command": f"clang++-18 -std=c++17 -c {src}"}],
            open(os.path.join(db, "compile_commands.json"), "w"),
        )
        first = count(db, [src])
        assert sum(first.values()) == 1, first
        open(src, "w").write("int f(int* p) { return (long)p > 0 ? 1 : 0; }\nint g(int x) { return (int)x; }\n")
        second = count(db, [src])
        assert sum(second.values()) == 2, second  # one C cast added: the count rises
        open(src, "w").write("int f(int* p) { return static_cast<long>(*p) > 0 ? 1 : 0; }\nint g(int x) { return x; }\n")
        third = count(db, [src])
        assert sum(third.values()) == 0, third
    print("selftest OK (counts", sum(first.values()), sum(second.values()), sum(third.values()), ")")
    return 0


def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--baseline", metavar="BUILD_DIR")
    g.add_argument("--check", metavar="BUILD_DIR")
    g.add_argument("--selftest", action="store_true")
    ap.add_argument("--update", action="store_true", help="with --check: write the lowered counts")
    ap.add_argument("files", nargs="*")
    ap.add_argument("-j", type=int, default=8)
    a = ap.parse_args()
    if a.selftest:
        return selftest()
    if a.baseline:
        counts = count(os.path.realpath(a.baseline), translation_units(os.path.realpath(a.baseline)), a.j)
        write_baseline(counts)
        print(f"{sum(counts.values())} findings in {len(counts)} (file, check) rows", file=sys.stderr)
        return 0
    return check(os.path.realpath(a.check), a.files, a.update)


if __name__ == "__main__":
    sys.exit(main())
