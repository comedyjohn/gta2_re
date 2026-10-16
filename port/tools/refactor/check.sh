#!/bin/bash
# "The check" of port/refactor/EXECUTOR.md §2: builds, warnings, tests, replays. Stops at the first
# failure and ends with `CHECK OK` or `CHECK FAILED: <step>`.
#
#   port/tools/refactor/check.sh            the full check (run it before every commit)
#   port/tools/refactor/check.sh --quick    skips clang64, noworld3d64 and nettest (for iterating)
set -u
quick=0
[ "${1:-}" = "--quick" ] && quick=1

root=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$root" || exit 2
E="env -i HOME=$HOME PATH=/usr/bin:/bin:$HOME/.local/bin"
logdir="build_port/check"
mkdir -p "$logdir"

# Old warnings that are expected (port/world3d/PLAN.md §1.1).
known_warnings='platform/dinput.cpp:322|tests/test_map_reader.cpp:(30|32)'

fail() {
    echo "CHECK FAILED: $1"
    exit 1
}

# step NAME COMMAND...: runs COMMAND with its output in the log; shows the tail on failure.
step() {
    name=$1
    shift
    if ! "$@" >"$logdir/$name.log" 2>&1; then
        tail -40 "$logdir/$name.log"
        fail "$name"
    fi
}

builds="linux32 linux64"
[ $quick = 0 ] && builds="$builds clang64 noworld3d64"

for p in $builds; do
    step "build-$p" bash -c "cd port && $E cmake --build --preset $p"
done

# A warning shows only when its TU is compiled, so this judges the files this change rebuilt.
cat $(for p in $builds; do echo "$logdir/build-$p.log"; done) | grep 'warning:' |
    grep -Ev "$known_warnings" >"$logdir/warnings.log"
if [ -s "$logdir/warnings.log" ]; then
    head -20 "$logdir/warnings.log"
    fail "warnings"
fi

step ctest-linux64 $E ctest --test-dir build_port/linux64 --output-on-failure
[ $quick = 0 ] && step ctest-clang64 $E ctest --test-dir build_port/clang64 --output-on-failure
step selftest $E xvfb-run -a build_port/linux64/gta2 --selftest
step audiotest $E build_port/linux64/gta2 --audiotest
step simcheck $E build_port/linux32/gta2 --simcheck
[ $quick = 0 ] && step nettest $E port/tools/nettest.sh build_port/linux32 build_port/linux64

rbuilds="build_port/linux32 build_port/linux64"
[ $quick = 0 ] && rbuilds="$rbuilds build_port/clang64"
step replays $E port/tools/replaytest.sh $rbuilds
grep -q FAIL "$logdir/replays.log" && { cat "$logdir/replays.log"; fail "replays"; }

echo "CHECK OK"
