#!/bin/sh
# Rebuilds the VC6 (matching) build under Wine and compares every MATCH_FUNC with a baseline saved
# by `compare_builds.py --save`. Port work must keep this at "no changes". Usage: check_matching.sh <baseline dir>
set -e
base=${1:?usage: check_matching.sh <baseline dir>}
cd "$(dirname "$0")/../.."
. venv/bin/activate
export WINEDEBUG=-all
rm -f build_vc6/decomp_main.exe
python3 build.py --ignore_no_match >build_vc6.log 2>&1 || true
[ -f build_vc6/decomp_main.exe ] || { echo "VC6 build failed, see build_vc6.log"; exit 1; }
cd Scripts/bin_comp
python3 msvc_dump_new_data.py >/dev/null
python3 compare_builds.py "$base"
