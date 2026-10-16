#!/bin/bash
# Runs every replay of the corpus under valgrind memcheck (uninitialised reads, invalid accesses).
# Slow (hours): run it in the background. port/refactor/0-safety-net.md task 0.9.
#
#   port/tools/refactor/valgrind.sh BUILD_DIR      e.g. build_port/linux64
#
# Logs go to BUILD_DIR/valgrind/NAME.{vg,out}. Exit status 1 when a replay has findings.
set -u
build=${1:?usage: valgrind.sh BUILD_DIR}
corpus="${GTA2_CORPUS:-$HOME/Games/gta2/corpus}"
game="${GTA2_GAME_DIR:-$HOME/Games/gta2/game}"
out="$build/valgrind"
mkdir -p "$out"
failed=0
for rep in "$corpus"/replays/*.rep; do
    name=$(basename "$rep" .rep)
    if valgrind --error-exitcode=1 --track-origins=yes --log-file="$out/$name.vg" \
        "$build/gta2" --replay "$rep" --game-dir "$game" --hash-log "$out/$name.log" >"$out/$name.out" 2>&1; then
        echo "CLEAN $name"
    else
        echo "FINDINGS $name (see $out/$name.vg)"
        failed=1
    fi
done
exit $failed
