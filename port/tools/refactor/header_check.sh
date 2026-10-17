#!/bin/bash
# Every port/game header must compile on its own: for each one, a .cpp that includes only it is
# syntax-checked with the flags of a game TU of the build directory (default build_port/clang64).
#
#   port/tools/refactor/header_check.sh [BUILD_DIR] [headers...]
# Prints `FAIL <header>` with the first error for each failure; exit status 1 if any failed.
set -u
root=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$root"
build=${1:-build_port/clang64}
shift || true
tmp=${CLAUDE_JOB_DIR:-/tmp}/tmp/hdrcheck.$$
mkdir -p "$tmp"
cmd=$(python3 - "$build" <<'P'
import json, shlex, sys
for e in json.load(open(sys.argv[1] + "/compile_commands.json")):
    if e["file"].endswith("/port/game/rng.cpp"):
        a = e.get("arguments") or shlex.split(e["command"])
        out, skip = [], False
        for x in a:
            if skip:
                skip = False
            elif x in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
            elif x in ("-c", "-MD", "-MMD") or x == e["file"]:
                pass
            else:
                out.append(x)
        print(" ".join(shlex.quote(x) for x in out))
        break
P
)
headers=${*:-$(cd port/game && find . -name "*.hpp" -o -name "*.h" | grep -v 3rdParty | sed "s|^\./||")}
one() {
    h=$1
    f="$tmp/${h//\//_}.cpp"
    printf '#include "%s"\n' "$h" >"$f"
    if ! out=$(eval "$cmd" -fsyntax-only -x c++ "$f" 2>&1); then
        echo "FAIL $h: $(echo "$out" | grep -m1 'error' | sed 's|.*/port/game/||')"
    fi
}
export -f one
export cmd tmp
printf '%s\n' $headers | xargs -P 8 -I{} bash -c 'one {}' | sort >"$tmp/result"
cat "$tmp/result"
n=$(wc -l <"$tmp/result")
echo "$n header(s) do not compile on their own" >&2
rm -rf "$tmp"
[ "$n" = 0 ]
