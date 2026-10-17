#!/bin/bash
# todo_all.sh SUBSYSTEM: the names propose.py can't decide, for every kind (after a fresh scan unless --no-scan)
cd "$(dirname "$0")/../../.."
T=~/.venvs/gta2-refactor/bin
[ "$2" = "--no-scan" ] || $T/python port/tools/refactor/symbols.py build_port/clang64 2>&1 | tail -1 | cut -c1-40
for k in types functions members enumerators globals params locals; do
    echo "== $1 $k: $($T/python port/tools/refactor/propose.py $1 $k 2>&1 >/dev/null | tail -1)"
    $T/python port/tools/refactor/propose.py $1 $k 2>/dev/null | awk -F, 'NR>1 && $2==""' | cut -d, -f1,3 | sed 's/^usr:[^,]*,/(usr),/' | sort -u
done
