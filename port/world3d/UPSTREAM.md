# Upstream code in world3d

Source: https://github.com/gebdag/gta2-rtx-remix, commit `d51b19b` (2026-10-05), MIT licence.

Copyright (c) 2026 gebdag (the full licence text is `upstream/LICENSE`; the notice must be kept).

| File (in `upstream/`) | Copied from | Changed since |
|---|---|---|
| math3d.h | src/math3d.h | no |
| alpha_bleed.h | src/alpha_bleed.h | no |
| gta2_map.h, gta2_map.cpp | src/ | no |
| gta2_style.h, gta2_style.cpp | src/ | no |
| world_mesh.h, world_mesh.cpp | src/ | no |
| ground.h, ground.cpp | dll/src/ | ground.h: 2 includes |

## Changes

- ground.h: its two includes of `../../src/` now name `gta2_map.h` and `world_mesh.h` (task 2.2).
- CMake compiles the four .cpp files with `-w`, no source changes.
