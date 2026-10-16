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

## Measurements

Mesh build (task 2.5, linux64, replays with `--draw`):

| Level | Build | Vertices | Triangles | Batches |
|---|---|---|---|---|
| 00-synthetic-walk | 49 ms | 551,030 | 275,280 | 855 |
| 11-attract-downtown | 50 ms | 551,030 | 275,280 | 855 |
| 11-attract-industrial | 52-67 ms (rebuilt several times: blocks change in play) | 702,983 | 350,607 | 947 |

Camera check (task 3.3, `GTA2_WORLD3D_CHECK=1`, three corpus replays with `--draw`): the clip matrix
reproduces the game's own screen positions of unsloped blocks to at most 0.12 / 0.24 / 0.23 px
(walk / downtown / industrial). Blocks with a slope are not compared: for them the game's world
vertices (`verts[4..7]`) are not the corners it projected (a ramp's z is fractional), so the check
counts them apart (errors of hundreds of px there are expected, not a camera error).

Frame comparison (task 3.6, `tools/world3d/compare_frames.sh`, classic against world3d, share of pixels
differing by more than 24 in a channel; only edges and seams differ in the difference pictures):

| Replay | update 200 | update 400 | update 600 |
|---|---|---|---|
| 00-synthetic-pad | 0.47% | 0.50% | 0.38% |
| 00-synthetic-walk | 0.60% | 0.38% | 0.81% |
| 11-attract-downtown | 0.06% | 0.59% | 0.35% |
| 11-attract-industrial | 0.01% | 0.12% | 0.07% |

A task that raises any of these by more than 2 points must say why in its commit message.
