#!/bin/bash
# Applies one clang-tidy check's fix-its to a subsystem (EXECUTOR.md 4.1): the fixes are exported per
# TU (8 jobs), merged by clang-apply-replacements (identical header edits are deduplicated, conflicting
# ones are an error), then the changed files are clang-formatted.
#
#   port/tools/refactor/tidy_fix.sh readability-uppercase-literal-suffix vehicles
#   port/tools/refactor/tidy_fix.sh readability-uppercase-literal-suffix all   (port/game only)
#   port/tools/refactor/tidy_fix.sh readability-uppercase-literal-suffix platform   (or tests)
#   REMOVE_ONLY=1 keeps only the fix-its that delete text (for misc-include-cleaner: unused includes)
#   BUILD=build_port/clang64 T=~/.venvs/gta2-refactor/bin  (defaults)
set -eu
check=$1
sub=${2:-all}
root=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$root"
T=${T:-$HOME/.venvs/gta2-refactor/bin}
BUILD=${BUILD:-build_port/clang64}
out=${CLAUDE_JOB_DIR:-/tmp}/tmp/tidyfix.$$
mkdir -p "$out"
files=$($T/python - "$sub" <<'P'
import sys, json, os
sys.path.insert(0, "port/tools/refactor")
from subsystems import SUBSYSTEMS
sub = sys.argv[1]
db = json.load(open(os.environ.get("BUILD", "build_port/clang64") + "/compile_commands.json"))
for e in db:
    f = e["file"]
    if not f.endswith(".cpp"):
        continue
    if sub in ("platform", "tests"):
        if "/port/%s/" % sub in f:
            print(f)
        continue
    if "/port/game/" not in f:
        continue
    stem = os.path.basename(f)[:-4]
    if sub == "all" or stem in SUBSYSTEMS.get(sub, set()):
        print(f)
P
)
[ -n "$files" ] || { echo "no files for $sub"; exit 1; }
printf '%s\n' $files | awk '{print NR, $0}' | xargs -P 8 -L 1 bash -c '"$0" -p "$1" --quiet --checks="-*,'"$check"'" --export-fixes="$2/$3.yaml" "$4" >/dev/null 2>&1 || true' "$T/clang-tidy" "$BUILD" "$out"
if [ -n "${REMOVE_ONLY:-}" ]; then
    # keep only the deletions (misc-include-cleaner also wants to add includes, at one spot
    # several times over, and clang-apply-replacements refuses those)
    $T/python - "$out" <<'P'
import glob, os, sys, yaml
for f in glob.glob(sys.argv[1] + "/*.yaml"):
    d = yaml.safe_load(open(f))
    # never a file's own header, nor the ones that carry macros or platform names the build needs
    keep_always = {"Light_1D4CC.hpp", "Function.hpp", "windows.h", "stdio.h", "memory.h", "types.h", "stat.h", "shlwapi.h", "ddraw.h"}
    def removable(x):
        m = x["DiagnosticMessage"]
        parts = m["Message"].split()
        header = parts[2] if len(parts) > 2 else ""
        stem = os.path.basename(m["FilePath"]).rsplit(".", 1)[0]
        return header not in keep_always and not header.endswith(".inl") and os.path.splitext(header)[0] != stem
    d["Diagnostics"] = [x for x in d.get("Diagnostics", []) if x["DiagnosticMessage"].get("Replacements") and removable(x)]
    for x in d["Diagnostics"]:
        x["DiagnosticMessage"]["Replacements"] = [r for r in x["DiagnosticMessage"]["Replacements"] if r["ReplacementText"] == ""]
    d["Diagnostics"] = [x for x in d["Diagnostics"] if x["DiagnosticMessage"]["Replacements"]]
    yaml.safe_dump(d, open(f, "w"))
P
fi
ls "$out"/*.yaml >/dev/null 2>&1 || { echo "no fixes"; exit 0; }
clang-apply-replacements-18 "$out"
git diff --name-only -- port/game port/platform port/tests | grep -E '\.(cpp|hpp|h)$' | xargs -r "$T/clang-format" -i
echo "fixed: $(git diff --shortstat)"
rm -rf "$out"
