#!/bin/bash
# Source-based coverage of the replay corpus (port/refactor/4-control-flow.md 4.1).
#
#   port/tools/refactor/coverage.sh [BUILD_DIR]     default build_port/coverage64 (preset coverage64)
#
# Plays every replay of the corpus with the instrumented build, merges the profiles and writes into
# BUILD_DIR: coverage.csv (function, lines, line %, branches, branch %), opcodes.csv (script opcode, times
# its case was entered) and html/index.html. Hours are not needed: about two minutes, headless.
set -eu
root=$(cd "$(dirname "$0")/../../.." && pwd)
build=${1:-$root/build_port/coverage64}
corpus="${GTA2_CORPUS:-$HOME/Games/gta2/corpus}"
game="${GTA2_GAME_DIR:-$HOME/Games/gta2/game}"
exe="$build/gta2"
[ -x "$exe" ] || { echo "no $exe: cmake --preset coverage64 && cmake --build --preset coverage64 (in port/)" >&2; exit 2; }

prof="$build/profiles"
rm -rf "$prof"
mkdir -p "$prof"
for rep in "$corpus"/replays/*.rep; do
    name=$(basename "$rep" .rep)
    LLVM_PROFILE_FILE="$prof/$name.profraw" "$exe" --replay "$rep" --game-dir "$game" --hash-log "$prof/$name.log" \
        >"$prof/$name.out" 2>&1 || { echo "replay $name failed" >&2; exit 1; } &
done
wait
llvm-profdata-18 merge -sparse "$prof"/*.profraw -o "$build/coverage.profdata"

# Only the game's and the platform's own code, not the system and 3rdParty headers
ignore='(/usr/|/3rdParty/|/tests/)'
llvm-cov-18 export "$exe" -instr-profile="$build/coverage.profdata" -ignore-filename-regex="$ignore" \
    -summary-only >"$build/coverage.json"
python3 "$root/port/tools/refactor/coverage_report.py" "$build" "$exe"
llvm-cov-18 show "$exe" -instr-profile="$build/coverage.profdata" -ignore-filename-regex="$ignore" \
    -format=html -output-dir="$build/html" >/dev/null 2>&1 || true
echo "wrote $build/coverage.csv, opcodes.csv, html/"
