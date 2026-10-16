#!/usr/bin/env python3
"""Measures gta2-rtx-remix (https://github.com/gebdag/gta2-rtx-remix) for port/world3d/PLAN.md.

    git clone --depth 1 https://github.com/gebdag/gta2-rtx-remix /tmp/rtx
    python3 -I port/tools/world3d/survey.py /tmp/rtx            # line counts per plan category
    python3 -I port/tools/world3d/survey.py /tmp/rtx --addrs    # plus its 9.6f addresses as port functions

The upstream code reads the game's memory at fixed v9.6f addresses and patches its code. The port has
the game's source, so every address becomes a function or a struct field: --addrs looks each FUN_/0x00
address up in Scripts/ida/functions_data.json (9.6f -> 10.5 pairs) and greps port/game for the name.
"""
import json
import os
import re
import subprocess
import sys

# What happens to each upstream file in the port (PLAN.md, "What is taken over")
CATEGORIES = {
    "keep": [  # plain C++, taken over with small changes
        "src/math3d.h", "src/camera.h", "src/camera.cpp", "src/world_mesh.h", "src/world_mesh.cpp",
        "src/alpha_bleed.h", "dll/src/ground.h", "dll/src/ground.cpp", "dll/src/time_of_day.h",
        "dll/src/time_of_day.cpp", "dll/src/settings.h", "dll/src/settings.cpp",
    ],
    "adapt": [  # logic kept, memory reads and code patches replaced by the game's structs and hooks
        "src/gta2_map.h", "src/gta2_map.cpp", "src/gta2_style.h", "src/gta2_style.cpp",
        "dll/src/live_geometry.h", "dll/src/live_geometry.cpp", "dll/src/texture_store.h",
        "dll/src/texture_store.cpp", "dll/src/synthetic_lights.h", "dll/src/synthetic_lights.cpp",
        "dll/src/gibs.h", "dll/src/gibs.cpp", "dll/src/hud_pass.h", "dll/src/hud_pass.cpp",
        "dll/src/overlay.h", "dll/src/overlay.cpp", "dll/src/world_view.h", "dll/src/world_view.cpp",
    ],
    "remix": [  # the Direct3D 9 / RTX Remix backend only (phase 7)
        "src/renderer.h", "src/renderer.cpp", "dll/src/remix_api.h", "dll/src/remix_api.cpp",
        "dll/src/remix_lights.h", "dll/src/remix_lights.cpp",
    ],
    "devtools": [  # the F4 ImGui tuning panel and the light benchmark (phase 8)
        "dll/src/debug_overlay.h", "dll/src/debug_overlay.cpp", "dll/src/light_bench.h",
        "dll/src/light_bench.cpp",
    ],
    "drop": [  # DLL proxying, address tables, binary patches and Windows workarounds the port has no use for
        "dll/src/game_access.h", "dll/src/dllmain.cpp", "dll/src/gbh_exports.h", "dll/src/log.h",
        "dll/src/log.cpp", "dll/src/frame_limiter.h", "dll/src/frame_limiter.cpp",
        "dll/src/intro_movie.h", "dll/src/intro_movie.cpp", "vid/src/vid_proxy.cpp",
    ],
}


def lines(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return sum(1 for _ in f)


def survey(root):
    seen = set()
    total = 0
    print(f"{'category':10} {'files':>5} {'lines':>7}")
    for cat, files in CATEGORIES.items():
        n = 0
        for rel in files:
            p = os.path.join(root, rel)
            if os.path.exists(p):
                n += lines(p)
                seen.add(rel)
            else:
                print(f"  missing upstream: {rel}")
        total += n
        print(f"{cat:10} {len(files):5} {n:7}")
    print(f"{'total':10} {len(seen):5} {total:7}")
    for d in ("src", "dll/src", "vid/src"):
        for name in sorted(os.listdir(os.path.join(root, d))):
            rel = f"{d}/{name}"
            if rel not in seen:
                print(f"  not categorised (new upstream?): {rel} ({lines(os.path.join(root, rel))} lines)")


def addrs(root, repo):
    pairs = json.load(open(os.path.join(repo, "Scripts/ida/functions_data.json")))
    by96 = {int(e["v96f_address"], 16): e for e in pairs if e.get("v96f_address")}
    found = {}
    for d in ("src", "dll/src", "vid/src", "dll"):
        for name in os.listdir(os.path.join(root, d)):
            p = os.path.join(root, d, name)
            if not os.path.isfile(p):
                continue
            text = open(p, encoding="utf-8", errors="replace").read()
            for m in re.finditer(r"(?:FUN_|0x)(00[45][0-9A-Fa-f]{5})\b", text):
                found.setdefault(int(m.group(1), 16), set()).add(f"{d}/{name}")
    print(f"\n{len(found)} code addresses (0x400000..0x5FFFFF) named upstream")
    hit = 0
    for a in sorted(found):
        e = by96.get(a)
        if not e:
            continue
        hit += 1
        short = e["name"].split("::")[-1]
        r = subprocess.run(["grep", "-rl", short, os.path.join(repo, "port/game")], capture_output=True, text=True)
        where = " ".join(sorted(os.path.basename(x) for x in r.stdout.split())[:2])
        print(f"  0x{a:06X} -> {e.get('v105_address', '?'):>9} {e['name']:55} {where}")
    print(f"{hit} of {len(found)} are function starts with a known 10.5 pair; the rest are call sites,"
          " globals or functions inlined in 10.5 (docs/inlines_96f.md)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    up = sys.argv[1]
    repo = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../.."))
    survey(up)
    if "--addrs" in sys.argv:
        addrs(up, repo)
