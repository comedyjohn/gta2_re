#!/usr/bin/env python3
"""Turns the merged coverage profile into CSV files (called by coverage.sh).

  coverage_report.py BUILD_DIR EXE

Writes BUILD_DIR/coverage.csv (function, file, lines, missed lines, line %, branches, branch %) and
BUILD_DIR/opcodes.csv (script opcode, times its `case` was entered), and prints a summary.
"""
import csv
import glob
import os
import re
import subprocess
import sys

build, exe = sys.argv[1], sys.argv[2]
root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.."))
profdata = os.path.join(build, "coverage.profdata")
ignore = "-ignore-filename-regex=(/usr/|/3rdParty/|/tests/)"


def llvm_cov(cmd, *args):
    return subprocess.run(["llvm-cov-18", cmd, exe, f"-instr-profile={profdata}", ignore, *args],
                          check=True, capture_output=True, text=True).stdout


def demangle(names):
    out = subprocess.run(["c++filt"], input="\n".join(names), check=True, capture_output=True, text=True).stdout
    return out.split("\n")


sources = sorted(glob.glob(f"{root}/port/game/*.cpp") + glob.glob(f"{root}/port/platform/*.cpp")
                 + glob.glob(f"{root}/port/platform/*/*.cpp"))
rows = []
# "File '...':" headers group the functions (the same inline function appears under every file using it)
text = llvm_cov("report", "-show-functions", *sources)
current = ""
for line in text.splitlines():
    m = re.match(r"File '(.*)':", line)
    if m:
        current = os.path.relpath(m.group(1), root)
        continue
    parts = line.split()
    # name, regions, missed, cover, lines, missed, cover, branches, missed, cover
    if len(parts) == 10 and parts[3].endswith("%") and parts[0] != "TOTAL" and current:
        rows.append((parts[0], current, parts[4], parts[5], parts[6].rstrip("%"), parts[7], parts[9].rstrip("%")))
names = demangle([r[0] for r in rows])
with open(os.path.join(build, "coverage.csv"), "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["function", "file", "lines", "missed_lines", "line_pct", "branches", "branch_pct"])
    for name, r in zip(names, rows):
        w.writerow([name, *r[1:]])

# Script opcodes: the count printed at the start of every `case SCRCMD_x:` line
opcodes = {}
shown = llvm_cov("show", f"{root}/port/game/miss2_0x11C.cpp")
for line in shown.splitlines():
    m = re.match(r"\s*\d+\|\s*([\d.]+[kKMG]?)\|\s*case (SCRCMD_\w+):", line)
    if m:
        c = m.group(1)
        mult = {"k": 1e3, "K": 1e3, "M": 1e6, "G": 1e9}.get(c[-1], 1)
        opcodes[m.group(2)] = opcodes.get(m.group(2), 0) + int(float(c.rstrip("kKMG")) * mult)
with open(os.path.join(build, "opcodes.csv"), "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["opcode", "entered"])
    for k, v in sorted(opcodes.items()):
        w.writerow([k, v])
hit = sum(1 for v in opcodes.values() if v)
print(f"{len(rows)} functions in coverage.csv; script opcodes entered by the corpus: {hit} of {len(opcodes)}")
