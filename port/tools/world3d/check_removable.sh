#!/bin/sh
# Checks that the 3D renderer (port/world3d/PLAN.md) changes nothing it shouldn't:
#   1. the port builds and passes its tests with it (preset linux64) and without it (noworld3d64);
#   2. the replay corpus gives the expected state hashes on both, with the classic renderer;
#   3. the corpus still gives them with the 3D renderer chosen and every frame drawn (--draw):
#      drawing in 3D may not change what the game computes.
# Also prints how many lines are tagged "port: world3d" (the lines to delete to remove it).
#
#   port/tools/world3d/check_removable.sh            all of it (about 4 minutes)
#   port/tools/world3d/check_removable.sh --quick    1 and 2 only (about 1 minute)
#
# The corpus needs the game data: see port/tools/replaytest.sh. Runs in a clean environment, since
# compiler variables of other toolchains in the shell (CC, CXX, LDFLAGS) break the presets.
set -eu
here=$(cd "$(dirname "$0")/../.." && pwd) # port/
quick=0
[ "${1:-}" = --quick ] && quick=1

clean() {
    env -i HOME="$HOME" PATH=/usr/bin:/bin ${GTA2_CORPUS:+GTA2_CORPUS="$GTA2_CORPUS"} \
        ${GTA2_GAME_DIR:+GTA2_GAME_DIR="$GTA2_GAME_DIR"} "$@"
}

cd "$here"
for preset in linux64 noworld3d64; do
    echo "== build $preset"
    clean cmake --preset "$preset" >/dev/null
    clean cmake --build --preset "$preset" >"../build_port/$preset.build.log" 2>&1 ||
        { grep -E "error:|FAILED" "../build_port/$preset.build.log"; exit 1; }
    clean ctest --test-dir "../build_port/$preset" --output-on-failure >"../build_port/$preset.ctest.log" 2>&1 ||
        { cat "../build_port/$preset.ctest.log"; exit 1; }
    grep "tests passed" "../build_port/$preset.ctest.log"
    clean xvfb-run -a -s "-screen 0 1600x1200x24" "../build_port/$preset/gta2" --selftest 2>&1 | grep -q "SELFTEST PASSED" ||
        { echo "FAIL: $preset --selftest"; exit 1; }
    echo "selftest passed"
done

if nm -C ../build_port/noworld3d64/gta2 | grep -q world3d; then
    echo "FAIL: the build without world3d still has world3d symbols"
    exit 1
fi

echo "== corpus, classic renderer, with and without world3d"
clean GTA2_RENDERER=classic tools/replaytest.sh ../build_port/linux64 ../build_port/noworld3d64

if [ $quick = 0 ]; then
    echo "== corpus, 3D renderer, every frame drawn"
    clean GTA2_RENDERER=world3d GTA2_REPLAY_ARGS=--draw tools/replaytest.sh ../build_port/linux64
fi

tagged() {
    grep -rn "port: world3d" --include='*.cpp' --include='*.h' --include='*.hpp' --include='*.txt' . | grep -v '^./world3d/'
}
echo "== $(tagged | wc -l) lines tagged \"port: world3d\" outside port/world3d, in:"
tagged | cut -d: -f1 | sort | uniq -c
echo "OK"
