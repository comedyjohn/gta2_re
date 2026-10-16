#!/bin/sh
# The replay tests: plays every replay of the corpus headless (gta2 --replay) and compares the
# state hash logged after every update with the expected log. A refactor must not change any of
# them; a Fix: commit that changes them updates them (--update). port/refactor/0-safety-net.md.
#
#   port/tools/replaytest.sh BUILD_DIR...            check each build against the expected logs
#   port/tools/replaytest.sh --update BUILD_DIR      write the expected logs from this build
#   port/tools/replaytest.sh --add NAME FILE.rep [NOTE]   add a replay (gta2 --record) to the corpus
#
# The corpus holds game data, so it lives outside the repository: $GTA2_CORPUS (default
# ~/Games/gta2/corpus), with replays/NAME.rep, expected/NAME.log and corpus.txt (name, updates,
# SHA-256, note). The game folder is $GTA2_GAME_DIR (default ~/Games/gta2/game).
# Every build must log the same: the game's maths is the same on every system (gta2 --simcheck).
set -u

corpus="${GTA2_CORPUS:-$HOME/Games/gta2/corpus}"
game="${GTA2_GAME_DIR:-$HOME/Games/gta2/game}"
jobs="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

usage() {
    sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//' >&2
    exit 2
}

fail() {
    echo "replaytest: $*" >&2
    exit 2
}

[ -d "$game" ] || fail "no game folder at $game (set GTA2_GAME_DIR)"

# first_difference EXPECTED ACTUAL: "update U: component, component" or "the length"
first_difference() {
    paste -d '\n' "$1" "$2" | awk '
        NR % 2 == 1 { a = $0; next }
        a != $0 {
            n = split("frame rng_count total rng sprites peds cars objects players script police", names, " ")
            split(a, x, " "); split($0, y, " ")
            if (x[1] == "" || y[1] == "") { print "one log is longer (" (x[1] == "" ? "actual" : "expected") ")"; exit }
            s = ""
            for (i = 4; i <= n; i++) if (x[i] != y[i]) s = s (s == "" ? "" : ", ") names[i]
            print "update " x[1] ": " s
            exit
        }'
}

case "${1:-}" in
    "" | -h | --help) usage ;;
    --add)
        [ $# -ge 3 ] || usage
        name=$2
        rep=$3
        note="${4:-}"
        [ -f "$rep" ] || fail "no replay at $rep"
        mkdir -p "$corpus/replays" "$corpus/expected"
        [ -e "$corpus/replays/$name.rep" ] && fail "$name is in the corpus already"
        cp "$rep" "$corpus/replays/$name.rep"
        sha=$(sha256sum "$rep" | cut -d' ' -f1)
        printf '%s\t-\t%s\t%s\n' "$name" "$sha" "$note" >>"$corpus/corpus.txt"
        echo "Added $name; now write its expected log: $0 --update BUILD_DIR"
        exit 0
        ;;
esac

update=0
if [ "$1" = --update ]; then
    update=1
    shift
    [ $# -eq 1 ] || fail "--update takes one build"
fi
[ $# -ge 1 ] || usage
ls "$corpus"/replays/*.rep >/dev/null 2>&1 || fail "no replays in $corpus/replays (set GTA2_CORPUS)"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Every (build, replay) pair, played $jobs at a time. The fourth field spreads the X displays
# xvfb-run picks, which can clash when several start at once.
i=0
for build in "$@"; do
    [ -x "$build/gta2" ] || fail "no gta2 in $build"
    for rep in "$corpus"/replays/*.rep; do
        name=$(basename "$rep" .rep)
        tag=$(echo "$build" | tr '/' '_')
        i=$((i + 1))
        echo "$build $rep $work/$tag.$name.log $((100 + i * 3))"
    done
done >"$work/pairs"
# One game a pair; xvfb-run keeps its window off the screen (it doesn't draw anyway: skip_draw)
export game
xargs -P "$jobs" -L 1 sh -c '
    if command -v xvfb-run >/dev/null 2>&1; then
        xvfb-run -a -n "$4" "$1/gta2" --replay "$2" --game-dir "$game" --hash-log "$3" >"$3.out" 2>&1
    else
        "$1/gta2" --replay "$2" --game-dir "$game" --hash-log "$3" >"$3.out" 2>&1
    fi
    echo $? >"$3.status"' sh <"$work/pairs"

failed=0
while read -r build rep log _; do
    name=$(basename "$rep" .rep)
    status=$(cat "$log.status" 2>/dev/null || echo "?")
    if [ "$status" != 0 ]; then
        echo "FAIL $name ($build): gta2 exited with $status; $(tail -n 3 "$log.out" | tr '\n' ' ')"
        failed=1
        continue
    fi
    if [ $update = 1 ]; then
        cp "$log" "$corpus/expected/$name.log"
        updates=$(grep -vc '^#' "$log")
        # The number of updates goes into the manifest
        awk -v n="$name" -v u="$updates" 'BEGIN { FS = OFS = "\t" } $1 == n { $2 = u } { print }' \
            "$corpus/corpus.txt" >"$work/corpus.txt" && cp "$work/corpus.txt" "$corpus/corpus.txt"
        echo "UPDATED $name: $updates updates"
        # A replay recorded by this port should play back as it was recorded
        sync_note=$(grep -h 'out of sync' "$log.out" | head -n 1)
        [ -n "$sync_note" ] && echo "     note: $sync_note"
        continue
    fi
    expected="$corpus/expected/$name.log"
    if [ ! -f "$expected" ]; then
        echo "FAIL $name ($build): no expected log (run --update)"
        failed=1
    elif cmp -s "$expected" "$log"; then
        echo "PASS $name ($build)"
    else
        echo "FAIL $name ($build): first difference at $(first_difference "$expected" "$log")"
        echo "     see where with: $build/gta2 --replay $rep --game-dir $game --break-at UPDATE --hash-log dump.txt"
        failed=1
    fi
done <"$work/pairs"
exit $failed
