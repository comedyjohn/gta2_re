# Phase 1: Tooling baseline

**Goal:** the tools enforce the standard, so it can only improve: one format, strict warnings as
errors, a clang-tidy ratchet, sanitizer builds, and none of the decompilation's scaffolding left in the
port.

**Depends on:** phase 0 (every task here is checked with the replays). **Blocks:** phases 2 to 8.

**Exit criteria**

- [ ] `clang-format --dry-run -Werror` passes on `port/game/`, `port/platform/`, `port/tests/`; CI enforces it.
- [ ] CI builds with the phase-1 warning set below and `-Werror`; 0 warnings.
- [ ] `.clang-tidy` is checked in; CI runs the ratchet and fails on any new finding.
- [ ] The scaffolding files below are gone; the 32 duplicate inline copies are merged.
- [ ] `metrics.py` and the tidy baseline are the numbers quoted in [README.md](README.md).
- [ ] Every commit of the phase is hash-neutral (phase 0).

## 1.1 Formatting

The repository's `.clang-format` (Microsoft base, Allman braces, 4 spaces, 140 columns) is what the code
mostly follows already, but 182 of the 265 files in `game/` differ from it (about 10,100 lines).

- [ ] Pin clang-format 18.1.8 (the version clang-tidy uses; `pip install clang-format==18.1.8`).
- [ ] Format `port/game/`, `port/platform/`, `port/tests/` in **one** commit (no other changes):
      `clang-format -i port/game/*.{cpp,hpp,h} port/platform/**/*.{cpp,h}`.
- [ ] Add that commit to `.git-blame-ignore-revs` (and run `git config blame.ignoreRevsFile
      .git-blame-ignore-revs`), so `git blame` skips it.
- [x] CI job `port-format`: `clang-format --dry-run -Werror` on the same globs.
- [x] Optional: `port/tools/refactor/pre-commit` (formats staged files), installed with
      `ln -s ../../port/tools/refactor/pre-commit .git/hooks/pre-commit`.

Merge or rebase open branches before this commit; afterwards they can be rebased with
`git rebase -X theirs` followed by a re-format.

## 1.2 Remove the decompilation's scaffolding

| File / construct | What it is | What happens |
|---|---|---|
| `HookLoader.hpp` | Loads the game into the original exe (hook build) | Delete; `ImGuiDebug.cpp` stops including it |
| `GlobalsRegistry.hpp`, `Globals.cpp`/`.hpp` (`GlobalRef`, `GetGlobalsRegistry`) | Maps globals to original addresses for the hook build | Delete; the ImGui overlay's globals view goes (it doesn't run in the port anyway, `TODO.md`) |
| `decompme_context.hpp` | Context for decomp.me scratches | Delete (nothing includes it) |
| `test.cpp` | The decompilation's old test functions (`test_file`, `test_gtx_0x106C`, ...) | Delete; real tests are in `port/tests/` |
| `StdAfx.h` | Precompiled-header stub | Delete; include what is used |
| `crt_stubs.cpp`/`.hpp` (`crt::free`, `EhVecDtor`) | Calls into the original exe's C runtime; VC6's vector destructor iterator | `std::free`/`delete[]`, a destructor loop or `std::array` |
| `Function.hpp`: `NOT_IMPLEMENTED`, `COMMA`, `INLINE_MODE` | Decompilation markers | Delete (5 `NOT_IMPLEMENTED` left: implement or remove first) |
| `GTA2_ASSERT_SIZEOF_ALWAYS` on memory-only structs (16) | Kept struct sizes matching the original | Delete; keep `GTA2_ASSERT_FILE_SIZEOF` for the 11 structs read from files until phase 3 replaces them |
| `#pragma pack` (25) | Original layouts | Keep only on file-layout structs; remove elsewhere (hash-neutral, but check `sizeof`-dependent code: `memcpy`/`memset` with `sizeof`, file writes) |
| `__forceinline`, `CC`, `__stdcall` remnants | VC6 calling conventions and inlining | Delete or `inline` |
| `// 9.6f 0x...`, `// FUNCTION: 96f 0x...`, `// https://decomp.me/scratch/...` comments (728 + 235) | Research links | Move into `address_map.csv` in phase 2 (columns `addr_96f`, `decompme`); keep the plain `// 0xADDR` |

- [x] One commit per row. Hash-neutral except where marked. (Done 2026-10-16 except the research-link comments, which move in R2.2; `Globals.hpp` stays as an include list until R1.11; `3rdParty/bink` keeps its `__stdcall`.)

## 1.3 Merge duplicate copies

To match VC6's inlining decisions, some functions exist twice: an inline copy and an out-of-line one
(`IsPedAThreat_465D00` and `IsPedAThreat_Inline_465D00`, 294 lines each; `GetLength_inline_442AD0`,
`GetLength_inline_443710`, `GetLength_inline_5620D0`; `Fix16_To_Ang16_inlined_40F540` and
`Fix16_To_Ang16_ool_40F540`; `Add_ool`, `ClampToRangeFlexible_ool_55EEE0`, `ComputeTorqueUnknown_49E8E0_ool`,
...; 32 names).

- [x] List them: `grep -ohE '\b\w+_(Inline|inline|inlined|ool)\w*\(' port/game/*.{cpp,hpp} | sort -u`.
- [x] (2026-10-16; 3 names left, see PROGRESS.md) For each pair, diff the bodies. Identical: keep one, point all callers at it. Different: the
      difference is either a decompilation artefact (pick the correct one, prove it with the replays) or
      real (two functions; give them distinct names in phase 2).

## 1.4 Warnings

Today `game/` builds with `-Wall -Wextra -Wno-unused-parameter -Wno-char-subscripts
-Wno-format-truncation` plus `-Wno-write-strings -Wno-narrowing -Wno-multichar -Wno-unknown-pragmas
-Wno-invalid-offsetof` (`port/CMakeLists.txt`, `gta2_game_target`). A `-fsyntax-only` run with the
target set over Clang 18 finds 3,839 more:

| Warning | Findings | When | Notes |
|---|---|---|---|
| `-Wsign-conversion` | 1,726 | phase 3 | per subsystem |
| `-Wold-style-cast` | 850 | phase 1 | C casts in headers and macros the earlier pass missed |
| `-Wimplicit-int-conversion` (`-Wconversion`) | 748 | phase 3 | |
| `-Wunused-parameter` | 71 | phase 4 | the signatures were the original's; now they can change |
| `-Wwritable-strings` (`-Wwrite-strings`) | 71 | phase 1 | `const char*` for string literals |
| `-Wzero-as-null-pointer-constant` | 66 | phase 1 | `nullptr` |
| `-Wshorten-64-to-32` | 58 | phase 3 | real 64-bit truncations: review each |
| `-Wimplicit-int-float-conversion` | 50 | phase 3 | |
| `-Wcomma` | 47 | phase 1 | split comma expressions |
| `-Wshadow` | 46 | phase 1 | rename the inner variable |
| `-Wdouble-promotion` | 44 | phase 3 | `float` maths promoted to `double`: keep the original's precision, make it explicit |
| `-Wnested-anon-types`, `-Wgnu-anonymous-struct` | 23 | phase 3 | name the anonymous structs in unions |
| `-Wimplicit-fallthrough` | 11 | phase 1 | `[[fallthrough]]` where intended |
| `-Wc++11-narrowing` (once `-Wno-narrowing` is dropped) | 19 | phase 3 | brace-initialisers that narrow |
| other | 9 | phase 1 | |

The full target set, added to `game_warnings` as each one reaches 0:

```cmake
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast -Wcast-align
-Wnull-dereference -Wdouble-promotion -Wimplicit-fallthrough -Wextra-semi -Wnon-virtual-dtor
-Woverloaded-virtual -Wunused -Wformat=2 -Wundef -Wwrite-strings
# Clang also: -Wcomma -Wzero-as-null-pointer-constant -Wshorten-64-to-32 -Wunreachable-code
```

- [ ] Phase 1 turns on: `-Wold-style-cast -Wwrite-strings -Wzero-as-null-pointer-constant -Wcomma -Wshadow
      -Wimplicit-fallthrough -Wextra-semi -Wundef -Wformat=2`, and drops `-Wno-write-strings`,
      `-Wno-multichar`. One commit per warning, as before.
- [ ] `-Wno-narrowing`, `-Wno-invalid-offsetof`, `-Wno-char-subscripts`, `-Wconversion`,
      `-Wsign-conversion`, `-Wdouble-promotion`: phase 3. `-Wno-unused-parameter`: phase 4.
- [ ] CI: `-DGTA2_WERROR=ON` adds `-Werror` (off by default so new compilers don't break users' builds).
- The measurement: `port/tools/refactor/strict_warnings.py <build-dir> "<flags>" [--files]` replays
  `compile_commands.json` with `-fsyntax-only` and the given flags (the table above is its output for
  the full set on `build_port/clang64`).

## 1.5 clang-tidy configuration and ratchet

37,189 unique findings with every relevant check family on. Each check gets a phase:

| Check | Findings | Phase | How |
|---|---|---|---|
| `readability-implicit-bool-conversion` | 7,243 | 3 | `if (p != nullptr)`, `x != 0`; mechanical with `-fix` per subsystem |
| `misc-include-cleaner` | 2,886 | 1 | include what you use, forward declarations; `-fix` per file |
| `readability-uppercase-literal-suffix` | 2,598 | 1 | `1u` to `1U`; `-fix` |
| `misc-non-private-member-variables-in-classes` | 2,429 | 6 | encapsulation, once members have names and owners |
| `cppcoreguidelines-avoid-non-const-global-variables` | 2,378 | 6 | globals into `World` |
| `cppcoreguidelines-pro-bounds-array-to-pointer-decay` | 2,066 | 3 | `std::span`, `std::array` |
| `cppcoreguidelines-pro-type-union-access` | 1,802 | 3 | tagged types, `std::variant` |
| `cppcoreguidelines-pro-bounds-constant-array-index` | 1,750 | 3 | checked indexing |
| `bugprone-narrowing-conversions` | 1,528 | 3 | with `-Wconversion` |
| `misc-const-correctness` | 1,469 | 3 | `const` locals; `-fix` |
| `cppcoreguidelines-init-variables` | 918 | 3 | initialise locals (a `Fix:` where a value was read uninitialised) |
| `cppcoreguidelines-pro-type-vararg` | 865 | 3 | `std::format` instead of the printf family |
| `cert-err58-cpp` | 856 | 6 | globals with throwing constructors go into `World` |
| `cppcoreguidelines-prefer-member-initializer` | 832 | 3 | constructor initialiser lists; `-fix` |
| `cppcoreguidelines-pro-bounds-pointer-arithmetic` | 790 | 3, 5 | spans and pool handles |
| `readability-make-member-function-const` | 733 | 3 | `const` methods; `-fix` |
| `cppcoreguidelines-pro-type-cstyle-cast` | 664 | 1 | C++ casts in headers and macros |
| `modernize-avoid-c-arrays` | 601 | 3 | `std::array` (file layouts last, with the readers) |
| `readability-redundant-inline-specifier` | 545 | 1 | `-fix` |
| `cppcoreguidelines-pro-type-reinterpret-cast` | 514 | 3, 6 | file readers, typed pools, platform interfaces |
| `cert-err33-c` | 438 | 3 | check or explicitly ignore `fread`, `fclose`, ... results |
| `readability-function-cognitive-complexity` | 317 | 4 | split functions |
| `modernize-raw-string-literal` | 300 | 6 | the `\\` paths become `/` paths with `std::filesystem` |
| `readability-convert-member-functions-to-static` | 284 | 2 | static, or free functions |
| `cppcoreguidelines-owning-memory` | 255 | 5 | `unique_ptr`, pools |
| `readability-simplify-boolean-expr` | 236 | 4 | `-fix`, hash-checked |
| `cppcoreguidelines-pro-type-member-init` | 230 | 3, 5 | default member initialisers |
| `readability-else-after-return` | 195 | 4 | `-fix` |
| `cppcoreguidelines-avoid-goto` | 194 | 4 | restructure |
| `misc-use-anonymous-namespace` | 161 | 1 | file-local functions into anonymous namespaces |
| `cppcoreguidelines-special-member-functions` | 134 | 5 | rule of zero/five |
| `performance-enum-size` | 132 | 3 | after the file readers, so no layout depends on enum sizes |
| `performance-no-int-to-ptr` | 120 | 3, 6 | `PTR32` and handles |
| `bugprone-branch-clone` | 70 | 4 | merge identical branches |
| `bugprone-assignment-in-if-condition` | 57 | 4 | split assignment and test |
| `modernize-use-nodiscard` | 50 | 3 | `[[nodiscard]]` on queries |
| `readability-inconsistent-declaration-parameter-name` | 41 | 2 | with the renames |
| `readability-braces-around-statements` | 41 | 1 | `-fix` |
| `misc-unused-parameters` | 34 | 4 | remove or `[[maybe_unused]]` |
| `cppcoreguidelines-macro-usage` | 34 | 1, 6 | constants and inline functions instead of macros |
| `readability-non-const-parameter` | 27 | 3 | `const T*` |
| `modernize-use-std-print` | 22 | 3 | with the printf family |
| `bugprone-signed-char-misuse` | 22 | 3 | explicit `int8_t`/`uint8_t` |
| `bugprone-implicit-widening-of-multiplication-result` | 17 | 3 | with `-Wconversion` |
| `modernize-use-noexcept` | 16 | 3 | |
| `modernize-macro-to-enum` | 13 | 6 | `resource.h` goes with the Win32 dialogs |
| 28 other checks, under 10 findings each | 109 | 1 to 4 | when touching the file |

Disabled for good, with the reason in `.clang-tidy`: `modernize-use-trailing-return-type` (style),
`modernize-return-braced-init-list` (style), `cppcoreguidelines-avoid-do-while` (`do while` is fine),
`readability-identifier-length`, `readability-magic-numbers` and `cppcoreguidelines-avoid-magic-numbers`
(phase 2 names constants; the check is too noisy on game tuning values), `bugprone-easily-swappable-parameters`
(phase 3's strong types solve it properly), `modernize-use-using` on `typedef struct` blocks and
`3rdParty/` headers (it drops calling conventions and expands macros).

- [ ] `port/.clang-tidy` with `Checks:` as above, `WarningsAsErrors: ''`, `HeaderFilterRegex:
      'port/(game|platform|tests)/'`, and the `CheckOptions` for `readability-identifier-naming` (written in
      phase 2), `readability-function-size` (`LineThreshold: 150`), `readability-function-cognitive-complexity`
      (`Threshold: 40`).
- [ ] `port/tools/refactor/tidy_ratchet.py`:
      - `--baseline <build-dir>`: runs clang-tidy on every file (8 jobs in parallel; about 15 minutes),
        writes `port/refactor/tidy-baseline.csv` (`file,check,count`).
      - `--check <build-dir> [files...]`: runs on the given files (or the ones changed since `master`) and
        fails if any `(file, check)` count is above the baseline; prints the lowered counts to commit.
- [ ] CI job `port-tidy` on pull requests: the ratchet on changed files. Nightly: the full run.

## 1.6 Includes

- [ ] `misc-include-cleaner` (2,886) with `-fix`, one subsystem per commit; add forward declarations
      where a header only needs a pointer. Headers that pull in `<windows.h>` without needing it go first
      (`fix16.hpp` does, and every file includes it).
- [ ] Every header compiles on its own: CI builds a generated `.cpp` per header that includes only it.
- [ ] Measure build time before and after (`ninja -t compdb`, `time cmake --build` from clean).

## 1.7 Build and CI

- [ ] `CMakePresets.json`: `linux32`, `linux64`, `clang64`, `asan64`, `ubsan64`, `tsan64` (phase 7),
      `aarch64`, each with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`; `ccache` when found.
- [ ] CI workflow `port.yml` (separate from the decompilation's `ci.yml`):

| Job | Runs | When |
|---|---|---|
| `build` | GCC 32/64, Clang 64, ARM64 cross; `-Werror` | every push |
| `tests` | `ctest` (unit tests), `--selftest`, `--audiotest`, `nettest.sh` | every push |
| `sanitizers` | unit tests under ASan+UBSan | every push |
| `format` | clang-format check | every push |
| `tidy` | ratchet on changed files | pull requests; full run nightly |
| `metrics` | `metrics.py` output as an artefact, with the diff to `master` in the job summary | every push |
| `replays` | `replaytest.sh` on the self-hosted runner | every push (decision 3) |

## 1.8 Windows and macOS builds

The port can't build with MSVC yet: `port/include/port_prelude.h` stops on `_MSC_VER`, and the fake
`windows.h` would clash with the real one. A Windows or macOS build needs phase 6's platform interfaces.
Until then, Clang on Linux stands in for the macOS compiler, and a MinGW-w64 cross build is not
attempted (it would need the same fake-header work phase 6 removes).
