# Finishing the refactor: the executor's runbook

This file is for the model (or person) that carries out the refactoring plan in this directory,
task by task, without the context of the sessions that wrote it. The phase files (`0-safety-net.md` to
`8-documentation.md`) say **what** each phase changes and why. This file says **in which order, with
which commands, how to tell that a task is done, and when to stop and ask.** Where the two disagree,
this file wins for order and procedure, the phase file for content.

Read this whole file once before the first task. After that, each session starts at
[§3 Starting a session](#3-starting-a-session).

## 0. What you may and may not decide

The refactor changes how the code is written, never what the game computes. Almost every task can be
checked by a machine: the replay corpus logs a hash of the whole simulation after every update, and a
refactor must leave every log byte for byte the same. That is what makes it safe for you to work
alone. The only things you can't check by machine are listed below; for them you stop.

**Decided (2026-10-16, by the user; don't reopen):**

| # | Decision |
|---|---|
| 1 | Naming: Google C++ style. `PascalCase` types and functions, `snake_case` locals and parameters, `snake_case_` private/protected members, `snake_case` public members of plain structs, `kPascalCase` constants and enumerators, `g` + `PascalCase` mutable globals, `snake_case` files. Table in [2-names.md §2.1](2-names.md). |
| 2 | Address suffixes go from identifiers. Each function keeps a `// 0xADDR` comment; every rename lands in `port/tools/refactor/address_map.csv`. |
| 3 | Golden replays run locally (`replaytest.sh`) and, once one exists, on a self-hosted runner as the gate. You run them locally. |
| 4 | The original game's saves, replays, player data and mission scripts stay loadable forever. The port may write a new format for its own saves, with a version field. |
| 5 | Text: GXT is decoded to UTF-8 at load; `std::string` and `std::format` everywhere; code points are decoded only when drawing. |
| 6 | No exceptions in the simulation. `std::expected` for fallible operations, `FatalError` for broken invariants. Exceptions are allowed at load time and in tools. |

**You stop and write a question into `port/refactor/QUESTIONS.md` (then move on to another task that
doesn't depend on it) when:**

1. A change you believe is a pure refactor changes a replay hash, and after following [§6](#6-when-the-check-fails)
   you can't find a refactor-only version of it.
2. You would have to decide whether behaviour of the original is a bug to fix or a quirk to keep
   (a counter that wraps, a stale pointer read, an uninitialised read whose value matters).
   Exception: the classes in [§0.1](#01-fix-commits-you-may-make-alone).
3. A task needs a human: recording a replay by playing, play-testing on real hardware, choosing a name
   for a type whose purpose you can't work out from its code and its callers.
4. A tool from the plan (rename tool, tidy ratchet) can't be built to the spec, or its output looks
   wrong on more than a handful of symbols.
5. A task would touch `Source/` or `3rdParty/`, or files that `git status` shows as modified by
   someone else. `port/world3d/` is a special case: its code reads 38 game names
   (`world3d/game_view.cpp`, `scene.h`), so a **tool-made rename** must edit it like any other user of
   the symbol. Make no other change there, don't format it (it keeps upstream's style), and keep the
   `// port: world3d` tags on their lines.
6. You reach a **checkpoint** (marked ⛳ in the task list). Checkpoints are where the user reviews a
   batch before the next one builds on it. Stop the session there with a report.
7. Another session is working in the same checkout (for example on world3d). Never run two sessions
   in one working tree; ask the user to give the refactor its own branch or worktree.

`QUESTIONS.md` format (create it on first use):

```markdown
## Q<n>: <one line> (task <id>, <date>)
What I tried: ...
What I saw: <command and output, trimmed>
Options: A) ... B) ...
My recommendation: ...
Status: open
```

### 0.1 `Fix:` commits you may make alone

Only these, each as its own commit with `Fix:` in the subject, the reason in the body, and the expected
logs rewritten in the same commit (`replaytest.sh --update build_port/linux32`):

- Undefined behaviour that a sanitizer reports (ASan, UBSan, Valgrind) where the fix is to give the
  value the original program had on its platform (see `known-ub.md` for examples already handled).
- A difference between our builds (GCC 32 vs 64 vs Clang) where one of them is wrong; the fix makes
  all builds agree.
- A crash.

Everything else that changes a hash is a question (rule 2 above). Never run `--update` to make a red
replay green in a refactor commit.

## 1. One-time setup

Do these once per machine; check each with the command after it.

```bash
cd ~/Documents/Code/gta2_re
git status -sb                                   # on claude/<branch>, never master
git log --oneline -5
```

### 1.1 Clean build environment

The interactive shell may export another toolchain's `CC`, `CXX`, `LDFLAGS`. Every build command in
this file runs through:

```bash
E="env -i HOME=$HOME PATH=/usr/bin:/bin:$HOME/.local/bin"
```

### 1.2 Python tools (clang-tidy, clang-format, libclang)

They are not installed system-wide. Make a venv outside the repository and pin the LLVM 18 versions
(the same major version as the system's `clang++-18`):

```bash
python3 -m venv ~/.venvs/gta2-refactor
~/.venvs/gta2-refactor/bin/pip install clang-tidy==18.1.8 clang-format==18.1.8 libclang==18.1.1
~/.venvs/gta2-refactor/bin/clang-tidy --version      # LLVM version 18.1.8
~/.venvs/gta2-refactor/bin/clang-format --version
~/.venvs/gta2-refactor/bin/python -c "import clang.cindex; print('libclang ok')"
```

**The pip libclang has no built-in headers.** Without help it fails on `stddef.h` with a fatal error,
yet it still returns a cursor tree. That tree is corrupt: game types come out inside `namespace std`
(USRs like `c:@N@std@S@Ped`). Every libclang tool here must add
`-resource-dir $(clang++-18 -print-resource-dir)` (today `/usr/lib/llvm-18/lib/clang/18`) to each
TU's arguments, and must refuse to continue when a TU has any diagnostic of severity error or fatal.
(The pip `clang-tidy` doesn't have this problem.)

Two system packages are needed, and installing them needs the user (`sudo`). Ask once, at R0.1:
`! sudo apt install valgrind clang-tools-18`. `valgrind` is for R0.3. `clang-tools-18` provides
`clang-apply-replacements-18` for §4.1.

Task R1.0 below writes these pins into `port/tools/refactor/requirements.txt` so the next machine
gets the same versions. Refer to the tools as `T=~/.venvs/gta2-refactor/bin` in commands.

### 1.3 Game data and corpus

```bash
ls ~/Games/gta2/game/data/*.gmp | head -3       # game data
cat ~/Games/gta2/corpus/corpus.txt              # the replay corpus (4 replays on 2026-10-16)
```

If either is missing, stop: the replay tests can't run, and nothing in this file may be committed
without them.

### 1.4 Builds

```bash
cd port
for p in linux32 linux64 clang64; do $E cmake --preset $p && $E cmake --build --preset $p || break; done
```

`build_port/clang64/compile_commands.json` is what clang-tidy and the rename tool read.

### 1.5 Baseline

```bash
port/tools/replaytest.sh build_port/linux32 build_port/linux64 build_port/clang64
```

Every line must say `PASS`. If not, the tree is already broken: stop and report, don't start.

## 2. The check

Task R0.1 (first task) writes `port/tools/refactor/check.sh`, so that "the check" is one command.
Until it exists, run its steps by hand. It must do, in this order, stopping at the first failure:

| Step | Command | Time |
|---|---|---|
| Build GCC 32 | `$E cmake --build --preset linux32` | ~1 min incremental |
| Build GCC 64 | `$E cmake --build --preset linux64` | |
| Build Clang 64 | `$E cmake --build --preset clang64` | |
| Build without world3d | `$E cmake --build --preset noworld3d64` (the world3d hooks compile to empty inlines there; a change around a hook can break only this build) | |
| No new warnings | grep the three build logs for `warning:`; compare with the list of known ones (§2.1) | |
| Unit tests | `$E ctest --test-dir build_port/linux64 --output-on-failure` and the same for `clang64` | 5 s |
| Renderer | `$E xvfb-run -a build_port/linux64/gta2 --selftest` | 5 s |
| Mixer | `$E build_port/linux64/gta2 --audiotest` | 2 s |
| Maths tables | `$E build_port/linux32/gta2 --simcheck` | 1 s |
| Network | `port/tools/nettest.sh build_port/linux32 build_port/linux64` | 30 s |
| Replays | `port/tools/replaytest.sh build_port/linux32 build_port/linux64 build_port/clang64` | ~30 s |

It ends with one line `CHECK OK` or `CHECK FAILED: <step>`. `check.sh --quick` skips Clang and
`nettest`, for use while iterating; **commit only after a full `check.sh`.**

### 2.1 Known warnings

Two warnings are old and expected (see `port/world3d/PLAN.md` §1.1):
`platform/dinput.cpp:322` (`-Wclass-memaccess`) and `tests/test_map_reader.cpp:30/32`
(`-Wdeprecated-enum-enum-conversion`). `check.sh` keeps that list in a variable. Anything else is new
and must be fixed before committing (a rename can't add warnings; if one appears, the rename is wrong).

## 3. Starting a session

1. `git status -sb` and `git log --oneline -10`. If there are uncommitted changes you didn't make,
   stop and ask.
2. Read `port/refactor/PROGRESS.md` (task R0.1 creates it): the last finished task and any notes.
3. Read `QUESTIONS.md`: answered questions may unblock tasks.
4. Take the first task in [§7](#7-the-task-list) that is not done and not blocked. Read its section in
   the phase file it names before starting.
5. Run `check.sh --quick` once, to know the tree is green before you change anything.

## 4. Doing a task

1. Make the change. Mechanical changes are made by a tool (clang-tidy `-fix`, `rename.py`, a script you
   write and keep under `port/tools/refactor/`), never by hand across many files. If you write a
   one-off script, keep it anyway: the commit message points at it so the change can be replayed.
2. `check.sh`. Green: go on. Red: [§6](#6-when-the-check-fails).
3. Read your own diff (`git diff --stat`, then `git diff` of a sample of files). Look for: edits in
   comments or strings that shouldn't change, changes in `3rdParty/`, an unexpected number of files.
4. Tick the task's checkbox in the phase file and add a line to `PROGRESS.md`, in the same commit.
5. Commit (§5).

### 4.1 Running clang-tidy `-fix`

Never run `clang-tidy -fix` on several TUs in parallel. Each process fixes the shared headers itself,
so the edits collide and leave duplicated or garbled text. Do one of these:

- **Parallel and safe:** run `clang-tidy --export-fixes=<dir>/<n>.yaml` per TU (8 jobs), then
  `clang-apply-replacements-18 <dir>`. It deduplicates identical header edits and reports
  conflicting ones. Treat a conflict as a failure.
- **Sequential:** `-fix` one TU at a time. A header fixed by an earlier TU no longer warns in later
  ones. This is slow: about 265 TUs at a few seconds each.

Write the first as `port/tools/refactor/tidy_fix.sh <check> <subsystem>` and use it for every `-fix`
task. After applying, run `clang-format` on the changed files only, since fix-its don't always match
the format.

**Size of a commit:** one kind of change, one subsystem (or one file for manual work). A commit
that touches more than about 3,000 lines must be purely tool-made (formatting, a tidy `-fix`, a rename
batch). If a manual change grows past about 300 lines, split it.

## 5. Commits

```
Port: refactor <task id>, <what changed>

<What was done, by which tool and with which arguments, so it can be replayed:
  e.g. "clang-tidy -checks=-*,readability-braces-around-statements -fix on vehicles (subsystems.py)".>
<How it was checked: "check.sh: CHECK OK (replays: 4 PASS on linux32, linux64, clang64)".>

Co-Authored-By: ...                      (the attribution lines the repository's last commits use;
Claude-Session: ...                       copy their form from `git log -3`)
```

- `Fix:` commits: subject `Port: Fix: <what>`, body says what the original did, why it is wrong,
  which replays changed and at which update.
- Never push, never commit to `master`, never `git commit --amend` a commit you didn't just make,
  never `git rebase` published history. The user pushes.
- Never commit the game data, replays or anything under `~/Games`.

## 6. When the check fails

| Failure | What to do |
|---|---|
| Build error | Fix it. If it's in a tool-made change, fix the tool and rerun it on a clean tree (`git checkout -- port/`), rather than patching its output by hand. |
| New warning | Same. |
| Unit test | Your change broke behaviour the test pins. Undo, understand, redo. |
| `--selftest` pixel diff | Something in drawing changed. Phase 1 to 5 tasks must not change pixels. Undo. |
| Replay `FAIL ... first difference at update N: <components>` | Your change altered the simulation. Steps below. |
| Replay fails on one build only (say Clang, not GCC) | A compiler-dependent result (argument evaluation order, uninitialised read). §0.1 may apply. |
| Flaky (passes on rerun) | Never ignore. Run the replay 16 times in parallel and compare md5 of the logs (`port/refactor/0-safety-net.md`, flaky-log check). A flaky log is an uninitialised read or a pointer used as a value: find it with Valgrind or UBSan, fix as §0.1. |

Replay failure, step by step:

1. Confirm it's yours: `git stash`, rebuild, `replaytest.sh` must pass; `git stash pop`.
2. Bisect your change: for a tool-made change, rerun the tool on half the files (or half the
   subsystem's symbols) until one file or symbol is left.
3. Look at the first differing update with `gta2 --replay R --break-at N` on both builds (before and
   after); it dumps the full state as text. `diff` the two dumps: the first differing field usually
   names the culprit.
4. Typical causes: a widened type that no longer wraps; a reordered pair of calls that both draw
   random numbers (function arguments!); a `memset`/`sizeof` that covered a different size; an
   initialiser added to a field the original left with the previous object's value; a `const&` that
   aliased a value the function changes.
5. Make a version of the change that keeps the old behaviour explicitly (`static_cast<u8>(x + 1)`, two
   locals to fix an order, a comment). If you can't, it's a question (§0).

## 7. The task list

Ids: `R<phase>.<n>`. Status is kept in the phase files' checkboxes and `PROGRESS.md`; this list is the
order. Columns:

- **Kind**: **M** mechanical (a tool makes the change, the check judges it), **J** judgement (you
  decide per site; the check judges), **H** needs the user. Do **H** tasks only up to the point where
  the user is needed, then write the question.
- **Size** in commits, roughly.
- ⛳ = checkpoint: stop the session after this task and report.

### Phase 0: finish the safety net (`0-safety-net.md`)

Phase 0 is mostly done. What's open either needs the user (recordings) or is small tooling. Phases 1
and 2 may start before it is complete, because they are compile-checked renames and formatting; phases
3 and 4 may not start until R0.5 and R0.7 are done.

| Id | Task | Kind | Size | Done when |
|---|---|---|---|---|
| R0.1 | Write `port/tools/refactor/check.sh` (§2, with `--quick`) and create `port/refactor/PROGRESS.md` (headings: Done, Notes) and `QUESTIONS.md` (empty). Document `check.sh` in `port/TESTING.md`. | J | 1 | `check.sh` prints `CHECK OK` on the current tree; with `<` changed to `<=` in `CarAI_78::DetectCarAhead_44D1D0` it prints `CHECK FAILED: replays` (revert afterwards). |
| R0.2 | `port/tools/refactor/requirements.txt` with the pins of §1.2, and a note in `TESTING.md` on making the venv. | M | 1 | Fresh venv from the file passes §1.2's three checks. |
| R0.3 | Needs `valgrind` (§1.2). Task 0.9 Valgrind: `port/tools/refactor/valgrind.sh BUILD_DIR` runs each corpus replay under `valgrind --error-exitcode=1 --track-origins=yes` (linux64, `skip_draw`). Each finding: `Fix:` (§0.1) or an entry in `known-ub.md`. | J | 1 + one per finding | All replays exit 0 under Valgrind. Expect hours of runtime: run it in the background. |
| R0.4 | Task 0.3 null renderer: `--replay` gets a renderer that does nothing (`port_renderer_lookup` hands out no-op `gbh_*`/`Vid_*`), so it runs without Xvfb. | J | 1–2 | `replaytest.sh` gives identical logs with no `DISPLAY` set; `--selftest` unchanged. |
| R0.5 | Task 0.10 golden frames, reusing the world3d frame capture (`gta2 --replay R --draw --screenshot N:FILE`, which writes PPM; `port/world3d/PLAN.md` task 3.5): `replaytest.sh --frames` captures 10 frames per replay with the classic renderer under Xvfb and compares them with `expected/NAME/frame-N.ppm` (exact, 0 differing pixels). First check that two runs and the three builds give identical frames. If they don't, write a question and don't commit a tolerance. | J | 2 | A change of one HUD colour constant turns it red; reverted, green. |
| R0.6 | Task 0.7 network desync: hashes in the per-frame network message, `nettest.sh` compares them. | J | 2 | `nettest.sh` fails when one side's `rand` call is skipped (temporary patch). |
| R0.7 | Task 4.1 coverage (moved here because phases 3 to 5 need it too): preset `coverage64`, `port/tools/refactor/coverage.sh` writes `build_port/coverage64/coverage.csv` (function, lines, line %, branch %). Add the opcode report for the script VM. | J | 2 | `coverage.csv` exists; `Ped::Threat_Reaction_AI_465270` and `CarAI_78::DetectCarAhead_44D1D0` appear with their numbers. |
| R0.8 | Corpus recordings (task 0.5): **H**. Write the list of sessions to record, from the coverage report's biggest uncovered subsystems, into `QUESTIONS.md` as one question addressed to the user, with the exact command per session (`gta2-launch --record ABS.rep --map wil|ste|bil`, what to do in it, 2–3 minutes each). When the user has recorded them: `replaytest.sh --add` + `--update`, commit the manifest change. | H | 1 | The user has recorded; every replay passes on all three builds twice. |
| R0.9 | ⛳ Re-run the mutation checks of phase 0's exit criteria (`DetectCarAhead` `<`, `Threat_Reaction_AI` constant, a skipped `rand` in `Particle_8`) and record the results in `0-safety-net.md`. | J | 1 | Each mutation is red, or the gap is written down with the replay that would close it. |

### Phase 1: tooling (`1-tooling.md`)

Order matters: formatting first (every later diff is then clean), then scaffolding, then warnings,
then tidy.

| Id | Task | Kind | Size | Done when |
|---|---|---|---|---|
| R1.1 | ⛳ **Ask first**: the format commit touches 182 files and conflicts with every open branch (world3d!). Write a question asking the user to confirm that no branch with `port/game` changes is open. Wait. | H | 0 | The user said go. |
| R1.2 | Format (task 1.1), one commit: `bash -O globstar -c '$T/clang-format -i port/game/*.{cpp,hpp,h} port/platform/**/*.{cpp,h} port/tests/*.{cpp,hpp}'` (without `globstar`, `**` doesn't recurse). Not `port/world3d/`. Add the commit's hash to `.git-blame-ignore-revs` in a second commit. *Trial run on 2026-10-16: 205 files, +11,726/−5,738 lines, 840 `#include` lines reordered (`SortIncludes: true`); linux64 built with no new warnings and all 4 replays passed. The `// port: world3d` tags stayed on their lines.* | M | 2 | `clang-format --dry-run -Werror` passes on all three trees; `check.sh` OK. **Replays must be identical.** Include sorting is the one part that could change code (an include that depends on an earlier one's macros). If a build breaks, separate the blocks with a blank line, which stops sorting across it; never reorder by hand. |
| R1.3 | CI job `port-format` in `.github/workflows/` (new file `port.yml`, task 1.7). Optional pre-commit hook. | M | 1 | The workflow parses (`python3 -c "import yaml,sys; yaml.safe_load(open(sys.argv[1]))" port.yml`). |
| R1.4 | Scaffolding (task 1.2): one commit per table row, in the table's order. For `NOT_IMPLEMENTED` sites, look in `Source/` whether the function is implemented there now; if not, it's a question. The 9.6f/decomp.me comments stay until R2.2 moves them into `address_map.csv`. | J | ~10 | Each file of the table is gone or reduced as the table says. |
| R1.5 | Duplicate copies (task 1.3): list with the plan's `grep`, then one commit per pair. Identical bodies: delete one, point callers at the other. Different: a question if the difference isn't obviously a decompilation artefact. | J | ~20 | The `grep` lists only pairs with a question attached. |
| R1.6 | Warnings (task 1.4): turn on, one per commit, in this order: `-Wextra-semi`, `-Wundef`, `-Wformat=2`, `-Wimplicit-fallthrough` (`[[fallthrough]]` where intended; a missing `break` that is a bug is a question), `-Wcomma`, `-Wzero-as-null-pointer-constant`, `-Wwrite-strings` (and drop `-Wno-write-strings`), drop `-Wno-multichar`, `-Wshadow` (rename the inner variable), `-Wold-style-cast` (850; clang-tidy `cppcoreguidelines-pro-type-cstyle-cast` / `google-readability-casting` `-fix`, per subsystem). Measure first with `strict_warnings.py`. | M/J | ~15 | `strict_warnings.py` reports 0 for each flag; flags added to `game_warnings` in `port/CMakeLists.txt`; GCC and Clang builds 0 new warnings. |
| R1.7 | `GTA2_WERROR` option (task 1.4) and use it in `check.sh`. | M | 1 | A deliberately unused variable fails the build with `-DGTA2_WERROR=ON`. |
| R1.8 | `port/.clang-tidy` (task 1.5) with the check list, the "disabled for good" list with reasons, `HeaderFilterRegex`. Leave the naming options out until R2.1. | J | 1 | `$T/clang-tidy -p build_port/clang64 port/game/rng.cpp` runs with no config error. |
| R1.9 | `port/tools/refactor/tidy_ratchet.py --baseline/--check` (task 1.5), baseline committed as `port/refactor/tidy-baseline.csv`. Use `$T/clang-tidy`, 8 jobs. | J | 2 | `--check` fails when one C cast is added to a file, passes after removing it. |
| R1.10 | Tidy `-fix` (always with `tidy_fix.sh`, §4.1), one check per commit, per subsystem when large: `readability-braces-around-statements`, `readability-redundant-inline-specifier`, `readability-uppercase-literal-suffix`, `misc-use-anonymous-namespace`, then the `<10 findings` checks listed as phase 1. Re-run `tidy_ratchet.py --baseline` in each commit. | M | ~25 | Those checks at 0 in the baseline. |
| R1.11 | Includes (task 1.6): `misc-include-cleaner -fix`, per subsystem, `fix16.hpp`'s `<windows.h>` first. Then the "every header compiles alone" CI check (`port/tools/refactor/header_check.sh`). Measure build time before and after into `PROGRESS.md`. | M/J | ~15 | `misc-include-cleaner` 0; `header_check.sh` passes. |
| R1.12 | CI `port.yml` (task 1.7): build, tests, sanitizers, format, tidy ratchet on PRs, metrics. Presets `tsan64`, `coverage64` already or now in `CMakePresets.json`. | J | 2 | Workflow parses; the jobs it runs pass locally with the same commands. |
| R1.13 | ⛳ Re-run `metrics.py` and the tidy baseline; update "Where it stands" in `README.md`; tick phase 1's exit criteria. | M | 1 | Exit criteria ticked or each open one has a reason. |

### Phase 2: names (`2-names.md`)

The biggest phase by edit count, and mostly mechanical once two tools exist. Build and test the tools
first, on a fixture, before they touch the game.

| Id | Task | Kind | Size | Done when |
|---|---|---|---|---|
| R2.1 | `port/STYLE.md` (naming table from 2-names §2.1 with decision 1, plus the `// 0xADDR` rule) and the `readability-identifier-naming` options in `port/.clang-tidy`, **disabled at the top level** (each finished subsystem directory turns it on, R2.x.8). Write `port/refactor/decisions/0001..0006-*.md` from §0's table (context, decision, consequences; 10–20 lines each). | J | 2 | Files exist. |
| R2.2 | `port/tools/refactor/address_map.py --init/--lookup` (task 2.2). Moves the `// 9.6f 0x...`, `// FUNCTION: 96f 0x...`, `// https://decomp.me/scratch/...` comments into the CSV and out of the code (one commit for the tool + CSV, one for the comment removal). | J | 2 | Row count ≥ functions with addresses (`metrics.py`); `--lookup 0x4B8BD0` prints `ExitGame_4B8BD0`. |
| R2.3 | `port/tools/refactor/rename.py` (task 2.3), USR-based with libclang, with `-resource-dir` (§1.2). **Write its tests first**: `port/tools/refactor/tests/rename/` with a small C++ tree and `compile_commands.json` covering: same member name in two classes, an overload set, a method called through a base pointer, a constructor/destructor, a template argument, a friend declaration, a use inside a macro (must be refused), a collision (must be refused), **a member used through a dependent name in a template**, and **a use in an inactive `#if` branch**. `pytest` runs them. See the two blind spots below the table. | J | 3 | All fixture tests pass; `--dry-run` on one real type (`Garage_48`) lists the same files as `grep -rlw Garage_48 port/` minus comments; `--dry-run` on `Ped::mpNext` refuses and names `Pool.hpp`. |
| R2.4 | `rename.py --propose <subsystem> <kind>` with the rules of task 2.3's table. Unresolvable names get an empty `new_name` and a `TODO` reason. | J | 1 | `--propose core functions` writes a CSV whose `TODO` rows are exactly the `sub_` and colliding names. |
| R2.5 | `port/tools/refactor/global_users.py` (task 2.5): per global, the functions that read and write it. | J | 1 | Output lists `gGame_0x40_67E008` with its users. |
| R2.6 | ⛳ Pilot: run the whole per-subsystem loop below on **core** only. Report the commits and how long each step took. | M/J | ~8 | Core is done per the loop's exit check. The user reviews before the other 13 subsystems. |

**The rename tool's two blind spots**, measured on 2026-10-16 on `Ped.cpp`:

1. **Dependent names in templates.** In `Pool.hpp`, all 44 lines that use `mpNext` and
   `PoolAllocate` (`pNew->mpNext`, `pNew->PoolAllocate()`) produce no cursor that resolves to a USR.
   The member belongs to the template parameter, so libclang can't say which class it is, and a USR
   rename skips those lines silently. The build then fails, which is caught but not fixed. The rule:
   after the USR pass, `rename.py` scans the text of every template definition (`FunctionTemplate`,
   `ClassTemplate` extents) in the touched files for the old spelling. A hit refuses the batch unless
   the batch renames that member in **every** class that the template is used with (all pooled types'
   `mpNext` together). Then a textual edit inside the template is correct, and the tool makes it.
   Expect this for `mpNext`, `PoolAllocate`, `PoolDeallocate` and the other members that `Pool.hpp`
   and the other 6 templates in `game/*.hpp` reach.
2. **Inactive preprocessor branches.** libclang only sees the branches that the `clang64`
   configuration compiles. The `#if defined(GTA2_PORT_64BIT)` blocks (`Ptr32.hpp`, `NetPlay.cpp`,
   `gtx_0x106C`, `map_0x370.hpp`, `text_0x14.cpp`) have 32-bit `#else` branches that it never sees,
   and the `IS_WINDOWS`/`IS_DOS`-style blocks in the Miles headers are never compiled at all. After
   every batch, `rename.py` greps the touched files for the old names outside comments and lists the
   hits. Edit them by hand in the same commit (they're few), and the `linux32` build proves them.

**The per-subsystem loop (R2.x)**, x running over the subsystem order: core, data, world, sprites,
objects, vehicles, peds, services, script, game, audio, presentation, network, app. One commit per step.

| Step | What | Kind |
|---|---|---|
| R2.x.1 | Types: `rename.py --propose <sub> types`; fill the `TODO` rows (collisions from task 2.4's table: decide from what the type does; its methods, who creates it in `Game_0x40::Game_0x40`); `rename.py renames.csv`; check. A type you can't name with confidence: a question, but still give it a provisional name with `// provisional name: ...` so the batch can run. | M + J |
| R2.x.2 | Methods and free functions. Same procedure. `sub_` functions: task 2.5's procedure; provisional names allowed with the comment. | M + J |
| R2.x.3 | Data members. Unnamed `field_XX` ones: name by use, or `unused_<hex>_` when written but never read. | M + J |
| R2.x.4 | Enumerators and constants. | M |
| R2.x.5 | Globals; raw data globals per task 2.5 (single-function users become locals only when the function never reads the previous call's value: check with `global_users.py` and the code). | J |
| R2.x.6 | Locals and parameters: `rename.py --propose <sub> locals` (Hungarian `p`/`pp`/`b`/`n`/`u` prefixes dropped, `snake_case`; locals have USRs too), review the collisions, apply. Don't use `readability-identifier-naming -fix` for this: it only changes case (`pCar` becomes `p_car`, not `car`). | M + J |
| R2.x.7 | Files and directories (task 2.6): `git mv` plus the include and CMake fixes in **one** commit, because a move-only commit doesn't build. Git still detects the renames (`git log --follow`) as long as the edits are only include lines. Use one include root (`port/game/`), so files that haven't moved yet keep working. | M |
| R2.x.8 | A `.clang-tidy` in `port/game/<sub>/` turning `readability-identifier-naming` on; ratchet baseline lowered. | M |

**Loop exit check for a subsystem**: `check.sh` OK; `readability-identifier-naming` has 0 findings in
`port/game/<sub>/`; the 2-names exit `grep` finds nothing in that directory outside comments;
`address_map.csv` has a `new_name` for every row whose `file` is in the subsystem.

| Id | Task | Kind | Size | Done when |
|---|---|---|---|---|
| R2.7 | Loop for data, world, sprites. | M/J | ~25 | Loop exit check for each. |
| R2.8 | ⛳ Loop for objects, vehicles, peds (the big ones; one ⛳ after each is fine too). | M/J | ~30 | |
| R2.9 | Loop for services, script, game. | M/J | ~25 | |
| R2.10 | Loop for audio, presentation, network, app. | M/J | ~30 | |
| R2.11 | ⛳ Phase 2 exit: top-level `readability-identifier-naming` on, `metrics.py`, README "Where it stands", exit criteria ticked. Collect all `// provisional name` comments into one question for the user. | M | 1 | |

### Phase 3: types and data (`3-types-and-data.md`)

Needs R0.7 (coverage, to know which conversions the replays reach) and R0.5 (golden frames, for
R3.11's text layout). Per subsystem, in the same order as phase 2, one warning kind or tidy check per
commit. Within the phase, this order:

| Id | Task (phase file section) | Kind | Size | Notes |
|---|---|---|---|---|
| R3.1 | 3.2 `readability-implicit-bool-conversion -fix` per subsystem | M | 14 | Purely mechanical; replays must stay identical. |
| R3.2 | 3.2 `misc-const-correctness`, `readability-make-member-function-const`, `-non-const-parameter` `-fix` per subsystem | M | ~30 | `const` can change overload choice: replays. |
| R3.3 | 3.2 `[[nodiscard]]`, member initialisers, `-init-variables` | M/J | ~20 | An added initialiser that changes a hash is §0.1 territory only if a sanitizer agrees it was an uninitialised read; otherwise a question. |
| R3.4 | 3.8 errors and logging: `core/log.hpp`, `FatalError` table, `Gta2Error` `enum class` | J | ~6 | Error message texts may change; behaviour may not. |
| R3.5 | 3.9 macros and constants (`constexpr`, `std::size`, enum underlying types after R3.10) | M/J | ~10 | |
| R3.6 | 3.1 conversions: `-Wsign-conversion`, `-Wimplicit-int-conversion`, `-Wshorten-64-to-32`, `-Wimplicit-int-float-conversion`, `-Wdouble-promotion`, per subsystem and warning. Decide each site with the phase file's three-case table. | J | ~60 | The slowest part of the phase. Widening a type is the risky case: replays decide. |
| R3.7 | 3.3 `std::array`, `std::span`, range-for; the `InputRecorder::GetInputBits` out-of-range read made explicit | M/J | ~20 | |
| R3.8 | 3.6 strong types: `core/strong_id.hpp` with unit tests, then one id type per commit | J | ~15 | |
| R3.9 | 3.5 unions (accessors, `std::bit_cast`) | J | ~10 | |
| R3.10 | 3.7 file readers, per format in the phase file's order (replay, text, kanji, car info, player data, saves, script, style, map, audio): parser + unit tests, parse-all-real-files check, switch loader, fuzz target (1 h, background), remove old structs. Decision 4: loading the original files must keep working; saves round-trip byte-identical. | J | ~50 | One ⛳ after the first format (replay), so the user sees the pattern. |
| R3.11 | 3.4 strings and text: logging, paths, names, then game text to UTF-8 (decision 5); remove `-fshort-wchar`, `port_wchar.h`, `port_crt.h` macros | J | ~30 | Golden frames (R0.5) must stay identical: text layout depends on character counts. |
| R3.12 | ⛳ Phase 3 exit criteria, metrics, README. | M | 1 | |

### Phase 4: control flow (`4-control-flow.md`)

Needs R0.7 (coverage). **The riskiest phase and the least mechanical**: every function is a small
project. Rules on top of the phase file:

- Before touching a function, look it up in `coverage.csv`. Under 80% line coverage, or a branch you
  change not covered: don't change it. Add it to a list in `PROGRESS.md` (`needs coverage: <function>,
  <missing branches>`); either write a unit test that reaches it, or collect it into a recording
  request for the user (R0.8's format).
- One function per commit. Before committing, write in the commit body the check "every path still
  reaches the same calls in the same order" and how you verified it.
- Never change the order of two calls that draw random numbers or change state; when splitting an
  expression whose operands are such calls, keep the order the reference build (linux32) used, and
  check by replay.

| Id | Task | Kind | Size |
|---|---|---|---|
| R4.1 | 4.2 mechanical tidy: `readability-else-after-return`, `readability-simplify-boolean-expr`, `bugprone-assignment-in-if-condition` (`-fix` where offered) | M | ~15 |
| R4.2 | 4.6 `goto`: the `ret_true`/`ret_false` ones first (mechanical), then `LABEL_nn` tails per function | M/J | ~60 |
| R4.3 | 4.7 dead code: `callgraph.py` (libclang), delete per subsystem; debug settings triage (the `do_brian_test`-style ones: ask before deleting, one question listing them); `TODO`/`FIXME` triage | J | ~25 |
| R4.4 | 4.7 unused parameters; drop `-Wno-unused-parameter` | M/J | ~10 |
| R4.5 | ⛳ 4.4 script VM dispatch table and opcode files (`--dump-script` first, it helps the tests) | J | ~15 |
| R4.6 | 4.3 state machines, one per ⛳: `CarAI_78::UpdateStateMachine`, `Char_B4::UpdateAnimState` (+ its states), police, ambulance, emergency crew, ped attack | J | ~40 |
| R4.7 | 4.5 the other giant functions, longest first, each covered first | J | ~80 |
| R4.8 | 4.8 file splits (`git mv`-style moves, no edits in the same commit) | M | ~30 |
| R4.9 | ⛳ Phase 4 exit criteria; the `NOLINT` exceptions list under 20 | M | 1 |

### Phases 5 to 8

Do not start these from this runbook yet. They depend on decisions that phases 2 to 4 will inform
(which pools are hot, which globals remain, how the platform interfaces look after phase 3's readers).
When R4.9 is done, the next session's first task is **R5.0: write this runbook's phase 5 table** in
the same form as above, from `5-ownership.md`, and stop for review. The same for 6 and 7. Phase 8
(documentation) runs alongside: each task above updates `STYLE.md`, `TESTING.md`, `CONTRIBUTING.md`
and the subsystem `README.md`s it affects in the same commit.

Things already known to need the user in those phases: play-testing each platform interface (6.4,
every row of its table), the stale-reference decisions (5.3, each is a behaviour choice), and the
Windows/macOS CI runners.

## 8. Effort

| Phase | Commits | Mechanical share | Executor sessions (estimate) | User input |
|---|---|---|---|---|
| 0 rest | ~12 | ~10% | 3–4 | Recordings (R0.8); one review |
| 1 | ~95 | ~80% | 6–8 | Go-ahead for formatting (R1.1); one review |
| 2 | ~130 + tools | ~90% of edits | 15–25 | Pilot review; names it can't decide |
| 3 | ~270 | ~50% of edits | 25–40 | Questions on wraps and conversions; one review per file format |
| 4 | ~280 | ~5% | 40+ | Recordings for uncovered code; a review per state machine |

A session here means one context window of steady work. These counts come from the task tables above
and replace the rougher ones in README.md's order table (phase 2: 300, 3: 150, 4: 250). The numbers
are guesses from the measured volume; record actual counts per task in `PROGRESS.md` so the next estimate is better.

## 9. Things that went wrong before (read once)

- The 64-bit build run from the wrong folder shows an "Error!" dialog and hangs; replays and the game
  run with `--game-dir ~/Games/gta2/game` (the scripts do this). Kill stuck processes with `kill -9`.
- Static initialisation order differs between GCC and Clang; a global initialised from another file's
  global was 0 in one build (`kFpEight_6FED48`). `tools/refactor/init_order.py` checks it: run it after
  any change to globals (phase 2 R2.x.5, phase 3 R3.5).
- The attract replays were recorded by the 9.6 build and drift from their recorded random numbers at
  updates 191/581: they test the port against itself, which is all a refactor needs.
- Clang warns where GCC doesn't (unused trivially constructible locals after `= default`): always build
  `clang64` before committing.
- A header change rebuilds everything: ~1 min per preset. Use `check.sh --quick` while iterating.
