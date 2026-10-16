#!/usr/bin/env python3
"""Counts the warnings a stricter flag set would add to port/game/, without building.

    python3 port/tools/refactor/strict_warnings.py build_port/clang64 "-Wconversion -Wshadow"
    python3 port/tools/refactor/strict_warnings.py build_port/clang64 "-Wconversion" --files
    python3 port/tools/refactor/strict_warnings.py build_port/clang64 "-Wshadow" --list -Wshadow
(Give the flags with a leading space, " -Wshadow", or argparse takes them for options.)

Replays every game/ compile command of <build-dir>/compile_commands.json with -fsyntax-only, the extra
flags, and the -Wno-* options that hide narrowing, writable strings, multi-character constants and
invalid offsetof removed. Each warning location is counted once (headers repeat in every TU).
The build directory must be configured with CMAKE_EXPORT_COMPILE_COMMANDS=ON (Clang recommended:
its flag names are what the table in port/refactor/1-tooling.md uses).
"""
import argparse
import collections
import concurrent.futures
import json
import os
import re
import shlex
import subprocess

DROP = {"-Wno-narrowing", "-Wno-write-strings", "-Wno-multichar", "-Wno-invalid-offsetof"}
DIAG = re.compile(r"(.*?):(\d+):(\d+): (?:warning|error): .*\[(-W[\w+=-]+)")


def commands(build_dir, extra):
    db = json.load(open(os.path.join(build_dir, "compile_commands.json")))
    for entry in db:
        if "/port/game/" not in entry["file"] or not entry["file"].endswith(".cpp"):
            continue
        args = entry.get("arguments") or shlex.split(entry["command"])
        out, skip = [], False
        for a in args:
            if skip:
                skip = False
                continue
            if a == "-o":
                skip = True
                continue
            if a not in DROP:
                out.append(a)
        yield entry["directory"], out + ["-fsyntax-only"] + extra


def run(job):
    directory, cmd = job
    return subprocess.run(cmd, cwd=directory, capture_output=True, text=True).stderr


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("build_dir")
    ap.add_argument("flags")
    ap.add_argument("--files", action="store_true", help="counts per file instead of per warning")
    ap.add_argument("--list", metavar="WARNING", help="prints the locations of this warning (e.g. -Wshadow) instead of counts")
    ap.add_argument("-j", type=int, default=os.cpu_count())
    args = ap.parse_args()

    seen = set()
    per_warning = collections.Counter()
    per_file = collections.Counter()
    with concurrent.futures.ThreadPoolExecutor(args.j) as pool:
        for stderr in pool.map(run, commands(args.build_dir, args.flags.split())):
            for line in stderr.splitlines():
                m = DIAG.match(line)
                if not m or "/port/game/" not in m.group(1):
                    continue
                key = m.groups()
                if key in seen:
                    continue
                seen.add(key)
                if args.list and m.group(4) == args.list:
                    print(f"{m.group(1)}:{m.group(2)}:{m.group(3)}")
                per_warning[m.group(4)] += 1
                per_file[os.path.basename(m.group(1))] += 1
    if args.list:
        return
    counts = per_file if args.files else per_warning
    for name, n in counts.most_common():
        print(f"{n:6} {name}")
    print(f"{sum(counts.values()):6} total")


if __name__ == "__main__":
    main()
