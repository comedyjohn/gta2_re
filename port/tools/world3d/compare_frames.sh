#!/bin/sh
# Compares the 3D renderer with the classic one, picture by picture (port/world3d/PLAN.md 3.6): for each
# replay of the corpus, updates 200, 400 and 600 are captured with --draw once per renderer, and
# compare_frames.py prints how many pixels differ. Needs the game data (see tools/replaytest.sh).
#
#   port/tools/world3d/compare_frames.sh BUILD_DIR [OUT_DIR]     (OUT_DIR keeps the pictures)
set -eu
[ $# -ge 1 ] || { sed -n '2,8p' "$0" | sed 's/^# \{0,1\}//' >&2; exit 2; }
build=$(cd "$1" && pwd)
here=$(cd "$(dirname "$0")" && pwd)
corpus="${GTA2_CORPUS:-$HOME/Games/gta2/corpus}"
game="${GTA2_GAME_DIR:-$HOME/Games/gta2/game}"
out="${2:-$(mktemp -d)}"
mkdir -p "$out"
updates="200 400 600"

for rep in "$corpus"/replays/*.rep; do
    name=$(basename "$rep" .rep)
    for renderer in classic world3d; do
        shots=""
        for n in $updates; do shots="$shots --screenshot $n:$out/$name-$renderer-$n.ppm"; done
        # shellcheck disable=SC2086
        env -i HOME="$HOME" PATH=/usr/bin:/bin GTA2_RENDERER=$renderer \
            xvfb-run -a -s "-screen 0 1600x1200x24" "$build/gta2" --replay "$rep" --game-dir "$game" --draw \
            $shots >/dev/null 2>&1 &
    done
    wait
    for n in $updates; do
        if [ -f "$out/$name-classic-$n.ppm" ] && [ -f "$out/$name-world3d-$n.ppm" ]; then
            printf '%s update %s: ' "$name" "$n"
            python3 -I "$here/compare_frames.py" "$out/$name-classic-$n.ppm" "$out/$name-world3d-$n.ppm" "$out/$name-diff-$n.png"
        else
            echo "$name update $n: no picture (the replay is shorter?)"
        fi
    done
done
echo "pictures in $out"
