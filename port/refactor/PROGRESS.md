# Refactor progress

## Done

- R0.1 (2026-10-16): `port/tools/refactor/check.sh` (full ~57 s, `--quick` ~9 s). Baseline before it: all
- R0.2 (2026-10-16): `requirements.txt`; a fresh venv from it passes the three checks of EXECUTOR.md §1.2.
  replays PASS on linux32, linux64, clang64.

- R0.3 (2026-10-16): `valgrind.sh`; 3 findings fixed (Q1, A); all 4 replays CLEAN under valgrind on linux64.

- R0.4 (2026-10-16): headless replays: no-op GL functions + SDL dummy video driver; `replaytest.sh` uses Xvfb only with `--draw`. 12 PASS with no DISPLAY; `--draw` and `--selftest` unchanged.

## Notes

- R0.1 mutation check: `<` to `<=` at `CarAI_78::DetectCarAhead_44D1D0` (the
  `GetCarLinearSpeed_43A240() < dword_677A8C` compare) leaves `check.sh` green: no replay reaches equality
  there. The check does go red on a covered change in the same function (`field_2B_ticks_since_alloc <= 20`
  to `<= 19`: first difference at update 3892 of 11-attract-industrial). Changes at `>` / `>=` of
  `dword_677A74 > kF16Zero` and `dword_677A8C > pOtherAI->field_74_max_speed` are also not caught. This
  gap belongs to R0.9.
