#!/usr/bin/env python3
"""Measures how far port/game/ is from the targets in port/refactor/README.md.

    python3 port/tools/refactor/metrics.py            # totals
    python3 port/tools/refactor/metrics.py --files    # per file, worst first
    python3 port/tools/refactor/metrics.py --subsystems

Counts are textual (regular expressions over the sources, comments included), so they are
approximate, but they are stable, which is what tracking progress needs.
"""
import argparse
import pathlib
import re
import sys

GAME = pathlib.Path(__file__).resolve().parents[2] / "game"
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from subsystems import SUBSYSTEMS  # noqa: E402

METRICS = {
    "lines": None,
    "addr_names": re.compile(r"\b[A-Za-z]\w*_[0-9A-F]{6}\b"),
    "sub_funcs": re.compile(r"\bsub_[0-9A-Fa-f]{6}\b"),
    "field_names": re.compile(r"\bfield_[0-9A-Fa-f]+(?:_\w+)?\b"),
    "globals_def": re.compile(r"\bDEFINE_GLOBAL\w*\("),
    "raw_data": re.compile(r"\b(?:byte|word|dword|unk|flt|off|stru)_[0-9A-Fa-f]{6}\b"),
    "new": re.compile(r"\bnew\b"),
    "delete": re.compile(r"\bdelete\b"),
    "malloc_free": re.compile(r"\b(?:malloc|free|realloc|calloc|Aligned_malloc\w*)\("),
    "mem_fns": re.compile(r"\bmem(?:cpy|set|move)\("),
    "c_strings": re.compile(r"\b(?:sprintf|vsprintf|strcpy|strncpy|strcat|wcscpy|wcscat|swprintf|_snprintf)\("),
    "goto": re.compile(r"\bgoto\b"),
    "union": re.compile(r"\bunion\b"),
    "win32": re.compile(r"\b(?:HWND|HINSTANCE|HRESULT|DWORD|BOOL|LPARAM|WPARAM|SendMessage\w*|PostMessage\w*|"
                        r"Reg\w+Ex\w*|IDirect\w+|LPDIRECT\w+|timeGetTime|GetTickCount|MessageBox\w*)\b"),
    "windows_h": re.compile(r"#include\s*<windows\.h>"),
    "file_reads": re.compile(r"\bGlobal_Read_4A71C0\b"),
    "todo": re.compile(r"\b(?:TODO|FIXME)\b"),
    "reinterpret": re.compile(r"\breinterpret_cast\b"),
}


def file_metrics(path):
    text = path.read_text(errors="replace")
    out = {"lines": text.count("\n")}
    for name, rx in METRICS.items():
        if rx is not None:
            out[name] = len(rx.findall(text))
    return out


def subsystem_of(stem):
    for sub, stems in SUBSYSTEMS.items():
        if stem in stems:
            return sub
    return "unassigned"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--files", action="store_true")
    ap.add_argument("--subsystems", action="store_true")
    ap.add_argument("--sort", default="lines")
    args = ap.parse_args()

    per_file = {}
    for p in sorted(GAME.iterdir()):
        if p.suffix in (".cpp", ".hpp", ".h"):
            per_file[p.name] = file_metrics(p)

    cols = list(METRICS)
    if args.files:
        rows = sorted(per_file.items(), key=lambda kv: -kv[1][args.sort])
        print("file," + ",".join(cols))
        for name, m in rows:
            print(name + "," + ",".join(str(m[c]) for c in cols))
        return
    if args.subsystems:
        agg = {}
        for name, m in per_file.items():
            sub = subsystem_of(name.rsplit(".", 1)[0])
            a = agg.setdefault(sub, {c: 0 for c in cols})
            for c in cols:
                a[c] += m[c]
        print("subsystem," + ",".join(cols))
        for sub, m in sorted(agg.items(), key=lambda kv: -kv[1]["lines"]):
            print(sub + "," + ",".join(str(m[c]) for c in cols))
        return
    total = {c: sum(m[c] for m in per_file.values()) for c in cols}
    for c in cols:
        print(f"{c:14} {total[c]}")


if __name__ == "__main__":
    main()
