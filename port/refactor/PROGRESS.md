# Refactor progress

## Done

- R0.1 (2026-10-16): `port/tools/refactor/check.sh` (full ~57 s, `--quick` ~9 s). Baseline before it: all
- R0.2 (2026-10-16): `requirements.txt`; a fresh venv from it passes the three checks of EXECUTOR.md §1.2.
  replays PASS on linux32, linux64, clang64.

- R0.3 (2026-10-16): `valgrind.sh`; 3 findings fixed (Q1, A); all 4 replays CLEAN under valgrind on linux64.

- R0.4 (2026-10-16): headless replays: no-op GL functions + SDL dummy video driver; `replaytest.sh` uses Xvfb only with `--draw`. 12 PASS with no DISPLAY; `--draw` and `--selftest` unchanged.

- R0.5 (2026-10-16): `replaytest.sh --frames`: 10 frames per replay, identical on 3 builds and across runs; changing the HUD `$` colour turned all 4 replays red. Front-end menus are not covered (needs menu recordings, R0.8).

- R0.6 (2026-10-16): `gta2 --nettest` exchanges a per-frame hash of a rand() sequence and reports `DESYNC at frame N`; `nettest.sh` scenario "a skipped rand() call is reported as a desync" passes (and failed without the check). The game's own message is untouched: Q2.

- R0.7 (2026-10-16): preset `coverage64`, `coverage.sh`, `coverage_report.py`. The corpus covers 68.6% of the lines of the code it links (4,554 functions in coverage.csv) and enters 124 of 404 script opcodes. `Ped::Threat_Reaction_AI_465270`: 75.6% lines, 63.3% branches; `CarAI_78::DetectCarAhead_44D1D0`: 76.6% lines, 73.0% branches. Both are under the 80% rule of phase 4.

- R0.8 (2026-10-16): the recording request is Q3 (open, for the user).

- R1.1 (2026-10-16): the user said go for the format commit (reply: "Yes do it").
- R0.9 (2026-10-16): mutation re-run, see the exit criteria of 0-safety-net.md. 2 of 4 mutations red; the 2 gaps are tied to the Q3 recordings. Phase 0 is done except R0.8 (Q3, user) and Q2 (open, no block).

- R1.2 (2026-10-16): clang-format 18.1.8 over port/game, port/platform, port/tests (205 files). Hash in .git-blame-ignore-revs.

- R1.3 (2026-10-16): `.github/workflows/port.yml` (job port-format) and `tools/refactor/pre-commit`. Not run on GitHub yet.

- R1.4 (2026-10-16): scaffolding removed, 14 commits (HookLoader, test.cpp and force_link, globals registry, decompme_context, StdAfx, crt_stubs, NOT_IMPLEMENTED, COMMA, __forceinline/INLINE_MODE, CC, SIZEOF_ALWAYS, pragma pack). The entry point is now port/platform/main.cpp calling WinMain_5E53F0.

## Notes

- R0.1 mutation check: `<` to `<=` at `CarAI_78::DetectCarAhead_44D1D0` (the
  `GetCarLinearSpeed_43A240() < dword_677A8C` compare) leaves `check.sh` green: no replay reaches equality
  there. The check does go red on a covered change in the same function (`field_2B_ticks_since_alloc <= 20`
  to `<= 19`: first difference at update 3892 of 11-attract-industrial). Changes at `>` / `>=` of
  `dword_677A74 > kF16Zero` and `dword_677A8C > pOtherAI->field_74_max_speed` are also not caught. This
  gap belongs to R0.9.
