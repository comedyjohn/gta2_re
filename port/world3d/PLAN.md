# world3d: the 3D renderer and RTX Remix, step by step

This plan brings [gebdag/gta2-rtx-remix](https://github.com/gebdag/gta2-rtx-remix) (MIT, measured at
upstream commit `d51b19b`, 2026-10-05) into the native port as `port/world3d/`. It is written so that a
model or person who has not seen the earlier discussion can execute it task by task: every task says
which files to touch, what to write, how to check it and what to commit. **Do the tasks in order. Do not
skip the checks. When a check fails and the task's "If it fails" doesn't cover it, stop and report.**

## 0. Status and decisions

| Phase | What | State |
|---|---|---|
| 1 | Switches and hooks (graphics.txt, Graphics page, `include/port_world3d.h`, removability check) | **done** (commit `589e3ed4`) |
| 2 | Scene layer: upstream's map, style and mesh code, built from the running game | todo |
| 3 | OpenGL world pass: the map drawn as 3D geometry, with the game's own camera, lighting and shading | todo |
| 4 | Depth for the game's 2D sprites, so they hide behind bridges and roofs correctly | todo |
| 5 | Lighting extras (render-only, each a Graphics setting): time of day, night lights, invented lights, additive effects | todo |
| 6 | Widescreen, blood, gibs (change the game) | **blocked: decision 4** |
| 7 | Windows x64 build of the port (MinGW-w64 cross compile) | todo |
| 8 | RTX Remix backend (Direct3D 9) | todo, after 7 |
| 9 | Tuning panel (Dear ImGui) | **blocked: decision 7** |

Decisions taken by the user (2026-10-16), not to be revisited:

1. Native OpenGL renderer first (phases 2-6), then Windows and Remix (7-8).
2. Windows build: MinGW-w64 cross compile on Linux, 64-bit only, tested under Wine.
3. `classic` stays the default renderer.
4. **Open:** widescreen spawning, blood and gibs change what the game computes. In or out? Phase 6 waits.
5. Camera straight down, as the original. No tilt.
6. Upstream files are copied into `port/world3d/` and adapted there (no submodule).
7. **Open:** the tuning panel (upstream's F4 window of sliders for every lighting and sprite value).
   Wanted? Phase 9 waits.

What the player gets, honestly: phases 2-4 reproduce the classic picture with real 3D geometry under it
(the foundation for everything else, and for Remix). Visible gains come with phase 5 (lighting), phase 6
(16:9) and phase 8 (path tracing).

## 1. Rules for the executor

### 1.1 Build and test commands

The shell may carry another toolchain's `CC`, `CXX` and `LDFLAGS` (a clang with a broken sysroot), which
breaks the presets. Always build in a clean environment:

```bash
cd port
E="env -i HOME=$HOME PATH=/usr/bin:/bin"
$E cmake --preset linux32 && $E cmake --build --preset linux32      # the playable build
$E cmake --preset linux64 && $E cmake --build --preset linux64
$E ctest --test-dir ../build_port/linux64 --output-on-failure        # unit tests
$E xvfb-run -a -s "-screen 0 1600x1200x24" ../build_port/linux64/gta2 --selftest
```

A build is good when the output has no `error:` lines and no new `warning:` lines in files you touched.
Two warnings are old and expected: `platform/dinput.cpp:322` (`-Wclass-memaccess`) and
`tests/test_map_reader.cpp:30/32` (`-Wdeprecated-enum-enum-conversion`).

### 1.2 The check after every task

```bash
port/tools/world3d/check_removable.sh --quick   # ~1 min: builds with and without world3d, unit tests, selftests, corpus
port/tools/world3d/check_removable.sh           # ~4 min: also the corpus with the 3D renderer drawing every frame
```

Run `--quick` after every task and the full one before every commit that touches drawing. It must end
with `OK`. Every `PASS` line matters: a `FAIL` in the replay corpus means your change altered what the
game computes, which world3d must never do (except phase 6, behind its own settings). If it fails: undo
your last change, run it again to confirm it passes, then find what differs (see 1.4).

From phase 3 on, also run the frame comparison of task 3.6 after every task that changes drawing.

### 1.3 Hard rules

1. **Never write game state from world3d.** Only read it. No calls to game functions that change anything
   (anything named `Set*`, `Add*`, `Remove*`, `Update*`, `Spawn*`, any `new`). Reading fields and calling
   pure getters (`GetTile_5AA870`, `GetTexture_46BB50`, `get_block_4DFE10`) is fine.
2. **Everything world3d does in `game/` or `platform/` goes through a hook in `include/port_world3d.h`**,
   and every line you add outside `port/world3d/` carries the comment `// port: world3d`. An `if` that
   calls a hook gets the tag on the `if` line; its braced block goes with it on removal. The empty inline
   (the `#else` branch of the header) must be added for every new hook.
3. With `renderer = classic`, nothing may change: same pixels (`--selftest`), same replay hashes.
4. One task, one commit. Commit messages: first line `Port: world3d <task id>, <what>`, a blank line, a
   short body saying what changed and how it was checked, then the two attribution lines this repository
   uses (see the last commits: `git log -3`).
5. Code style: follow the surrounding code (4 spaces, braces on their own line, `PascalCase` functions in
   world3d's own code, comments that say why). Copied upstream files keep upstream's style.
6. Never push. Never touch `Source/` (that is the byte-matching decompilation, not the port).
7. Upstream code goes in `port/world3d/upstream/` **verbatim first** (one commit), then changes in later
   commits, each listed in `port/world3d/UPSTREAM.md`. This keeps later upstream fixes mergeable.

### 1.4 When a replay fails

```bash
cd port
$E tools/replaytest.sh ../build_port/linux64          # which replay, which update, which component
$E ../build_port/linux64/gta2 --replay ~/Games/gta2/corpus/replays/NAME.rep \
   --game-dir ~/Games/gta2/game --break-at UPDATE --hash-log /tmp/dump.txt
```

Then compare against the same command on the commit before yours. The usual cause is a hook that writes
game memory, or a game getter that is not pure. Stop and report if you can't find it in an hour.

### 1.5 Where things are

| Thing | Where |
|---|---|
| The hooks | `port/include/port_world3d.h` (declarations + empty inlines) |
| Their implementation | `port/world3d/world3d.cpp` |
| Settings file (`graphics.txt`) | `port/world3d/settings.{h,cpp}`; the overlay page is at the end of `world3d.cpp` |
| world3d's CMake | `port/world3d/CMakeLists.txt` (library `gta2_world3d`; tests in `port/world3d/tests/` are linked into `gta2_tests`) |
| The classic renderer | `port/platform/renderer.cpp` (`gbh_*` functions, `GLRenderer`) |
| OpenGL function list | `port/platform/gl.h` (`PORT_GL_FUNCTIONS`); loaded by `port_gl_load()` in `renderer.cpp` |
| The window | `port/platform/sdl/sdl_platform.cpp` (`OpenWindow`) |
| Replay runner | `port/platform/replay.cpp` (`gta2 --replay`), `port/tools/replaytest.sh` |
| Upstream clone | `git clone https://github.com/gebdag/gta2-rtx-remix /tmp/rtx && git -C /tmp/rtx checkout d51b19b` |
| Upstream's addresses as port names | `python3 -I port/tools/world3d/survey.py /tmp/rtx --addrs` |

## 2. Facts the tasks rely on

All of these were checked against the code on 2026-10-16. If one turns out wrong, stop and report.

### 2.1 Coordinates

- **Game world:** `x` east, `y` south (map rows), `z` up in block levels (0..7). One block = 1.0. The
  game's `Fix16` is 16.16 fixed point; `.ToFloat()` converts.
- **Upstream mesh (`world_mesh.cpp`):** `X = x`, `Y = z` (level, up), `Z = 256 - y` (north). So a mesh
  vertex `(X, Y, Z)` is game point `(X, 256 - Z, Y)`. Convert once, when the mesh is built.

### 2.2 The game's projection (exact)

From `MapRenderer::Draw_4F6A20` (depths at `MapRenderer.cpp:3606`) and `ProjectWorldPointToScreen_4BA4D0`
(`sprite.cpp:92`). With the view camera `Camera_0xBC* cam = gViewCamera_676978`:

```
camX = cam->field_98_cur_pos.field_0_x.ToFloat()
camY = cam->field_98_cur_pos.field_4_y.ToFloat()
camZ = cam->field_98_cur_pos.field_8_z.ToFloat()
f    = cam->field_60_screen_scale.x.ToFloat()        // = screen width * zoom, in pixels
cx   = cam->field_70_screen_px_center_x, cy = cam->field_74_screen_px_center_y   // 320, 240
D    = camZ + 8 - z                                   // distance from the eye, in blocks
sx   = cx + f * (x - camX) / D
sy   = cy + f * (y - camY) / D
Vert.z (what the game passes as z and rhw) = 1 / D
```

So the game is a pinhole camera straight down, eye at `(camX, camY, camZ + 8)`, focal length `f` pixels,
on a 640x480 screen. The matching OpenGL clip-space transform for game point `(x, y, z)`, screen size
`W x H` = 640 x 480, near `n = 0.05`, far `F = 64`:

```
clip.x = (2f/W) * (x - camX)
clip.y = -(2f/H) * (y - camY)            // screen y grows down, NDC y grows up
clip.w = D = (camZ + 8) - z
clip.z = A * D + B,  A = (F + n)/(F - n),  B = -2*F*n/(F - n)   // NDC z = -1 at D = n, +1 at D = F
```

As a matrix for `glUniformMatrix4fv(loc, 1, GL_FALSE, m)` (column-major: `m[col*4 + row]`), with
`E = camZ + 8`:

```
row 0: [ 2f/W,     0,   0,   -2f/W * camX ]
row 1: [ 0,    -2f/H,   0,    2f/H * camY ]
row 2: [ 0,        0,  -A,    A*E + B     ]
row 3: [ 0,        0,  -1,    E           ]
```

### 2.3 The map, in memory

`Map_0x370* gMap_0x370_6F6268` (`game/map_0x370.hpp`). Its `field_0_pDmap` is a
`gmp_compressed_map_32*` laid out exactly as the `.gmp` file's DMAP chunk:

| Field | Meaning | Upstream `Map::LoadFromDmap` argument |
|---|---|---|
| `field_0_base[256][256]` (u32, `[y][x]`) | column index per cell | `base` |
| `field_40008_pColumn` (`gmp_column_word*`, 4-byte words on both builds; write `* 4`, not `sizeof`), `field_40000_column_words` words | column pool | `columns`, `columnBytes = words * 4` |
| `field_4000C_block` (`gmp_block_info*`, 12 bytes each), `field_40004_num_blocks` | blocks | `blocks`, `blockBytes = num * 12` |

`gmp_block_info`: `field_0_left, field_2_right, field_4_top, field_6_bottom, field_8_lid` (u16 face words:
tile = `& 0x3FF`, flat = `& 0x1000`, flip = `& 0x2000`, rotation = `>> 14`, lid shading level =
`(lid >> 10) & 3`), `field_A_arrows`, `field_B_slope_type` (ground type = `& 3`, slope type = `>> 2`).

The slope table: `gmp_map_slope gGmpSlopes_6F5BA8[64]` (`map_0x370.cpp:30`). Upstream's
`SlopeInfo{direction, steps, step}` = `{field_0_gradient_direction, field_1_gradient_size,
field_2_gradient_level}`.

The map changes at runtime in exactly these functions (`game/map_0x370.cpp`): `ChangeBlock_4E8620`,
`AddNewBlock_4E87C0`, `RemoveBlock_4E8940` (also reached through `LowerColumn_4E8A10` and
`LowerLevel_4E8B70`), `OnModifiedMapDataLoaded_4E8C00` (a saved game) and `LoadMap_4E95B0` (a new level).

### 2.4 Tile textures

A face's tile number `t = word & 0x3FF`. The game draws it with:

```cpp
u16 idx = gGtx_0x106C_703DD4->GetTile_5AA870(t);      // 0: nothing is drawn
STexture* tex = gTextureCache_15D8_705064->GetTexture_46BB50(idx);
```

Both are plain array reads. The remap changes at runtime (animated tiles such as water), so look it up
every frame, not once. The classic renderer has already uploaded every registered `STexture` as a GL
texture: its name is stored in `tex->field_1C_ptr` (`renderer.cpp`, `TextureId`): `GLuint id =
static_cast<GLuint>(reinterpret_cast<uintptr_t>(tex->field_1C_ptr))`. A tile texture is 64x64, so UV
0..1 covers the tile. Palette colour 0 is transparent (alpha 0): discard texels with alpha < 0.5, as the
classic shader does.

The style file the game loaded: `gGameSession_67E8E0.GetStyleName_4C5950()` (a game path like
`data\wil.sty`; `fopen` in code built with `gta2_game_target` resolves such paths).

### 2.5 Shading and lighting of map faces (to match classic)

Each face is drawn with a grey level `shade` (0..255) from `gpMapRenderer_6F66E4` (`game/MapRenderer.hpp`):

| Face | `shade` |
|---|---|
| lid | by `(lid >> 10) & 3`: 0 → 255, 1 → `field_C_colour_t1`, 2 → `field_E_colour_t2`, 3 → `field_F_colour_t3` |
| left (west) side | `field_C_colour_t1` |
| right (east) side | `field_D_right_colour` |
| top (north) side | `field_F_colour_t3` |
| bottom (south) side | `field_E_colour_t2` |
| diagonal walls | `field_10_diag_up_left_colour`, `field_11_diag_up_right_colour`, `field_12_diag_down_left_colour`, `field_13_diag_down_right_colour` |
| ramp lids | `MapRenderer::GetColour_4F0BD0(sel)` (see `DrawGradientSlope*` for `sel`) |

Then the classic renderer (`renderer.cpp`, `RenderVerts` and `ComputeLighting`) colours each vertex:
when the draw flags have `0x8000` and the ambient isn't 255,
`c = min(shade/255 * shade * L + ambient, 255)` per channel, where `L` is the sum over the frame's lights
of `light.rgb * brightness * (radius - dist) / radius` for lights within `radius` whose
`flags & 0x30000 == 0x10000`, `dist` measured from the vertex's **world** position. Otherwise `c = shade`.
Lights arrive through `gbh_ResetLights` / `gbh_AddLight` / `gbh_SetAmbient`; the game submits them **inside
the map draw**, before the first tile of level 0 (`MapRenderer.cpp:3622`), so they are ready by the first
`kPortWorld3dTile` hook call of a frame, not at `gbh_BeginScene`.

### 2.6 Sprites

`SpriteTree_4::Draw_5C5DF0` calls `Sprite::Draw_59EFF0` for each object; hook 2 brackets that call with
`port_world3d_sprite(sprite)` / `port_world3d_sprite(nullptr)`. Inside, `Sprite::Draw_59EFF0`
(`sprite.cpp:1089`) draws up to three quads with `gbh_DrawQuad`:

1. a shadow (flags `8576`, its own vertex copy offset a few pixels; world positions not stored),
2. the body (`gTileVerts_7036D0`, flags from `ComputeDrawFlags_4BAC60`): world positions of its 4 corners
   are in `gTileVerts_7036D0[4..7]` (`.x .y .z` = game x, y, z),
3. for cars with lights on, the lamps: same vertices, flags `128`.

Every one of these has `Vert.z = 1/D` (2.2). Map sprites drawn during the map pass come through the same
path.

### 2.7 Upstream files and their fate

| Upstream file | Lines | Where it goes | Phase |
|---|---|---|---|
| `src/math3d.h`, `src/alpha_bleed.h` | 72, 289 | `world3d/upstream/` as is | 2 |
| `src/gta2_map.{h,cpp}` | 197 | `world3d/upstream/`; `LoadFromDmap` is used, `Load(path)` unused | 2 |
| `src/gta2_style.{h,cpp}` | 479 | `world3d/upstream/`; `Load(path)` with the game's style path | 2 |
| `src/world_mesh.{h,cpp}` | 1,271 | `world3d/upstream/` | 2 |
| `dll/src/ground.{h,cpp}` | 224 | `world3d/upstream/` (phase 8 uses it) | 2 |
| `dll/src/time_of_day.{h,cpp}` | 365 | `world3d/upstream/`, without the Remix push | 5 |
| `dll/src/remix_lights.*` (`ClassifyGameLight` only), `dll/src/synthetic_lights.*` | 2,841 | adapted in `world3d/` | 5 |
| `dll/src/texture_store.*` (effect analysis) | 871 | adapted | 5 |
| `dll/src/hud_pass.*`, `src/camera.*` | 313 | adapted | 6 |
| `dll/src/gibs.*` | 599 | adapted | 6 |
| `src/renderer.*`, `dll/src/remix_api.*`, `dll/src/remix_lights.*`, `dll/src/live_geometry.*` | 3,858 | `world3d/remix/` | 8 |
| `dll/src/debug_overlay.*`, `dll/src/light_bench.*`, `dll/src/settings.*` | 2,520 | `world3d/devtools/` | 9 |
| `dll/src/game_access.h`, `dllmain.cpp`, `gbh_exports.h`, `log.*`, `frame_limiter.*`, `intro_movie.*`, `vid/src/vid_proxy.cpp` | 2,770 | not used: address tables and DLL workarounds the port doesn't need | - |

## 3. Phase 2: the scene layer

Goal: world3d builds upstream's `WorldMesh` from the running game's map and keeps it current. Nothing is
drawn yet.

### Task 2.1: copy upstream's scene files verbatim

1. Clone upstream at `d51b19b` (1.5).
2. `mkdir -p port/world3d/upstream` and copy, unchanged: `src/math3d.h`, `src/alpha_bleed.h`,
   `src/gta2_map.h`, `src/gta2_map.cpp`, `src/gta2_style.h`, `src/gta2_style.cpp`, `src/world_mesh.h`,
   `src/world_mesh.cpp`, `dll/src/ground.h`, `dll/src/ground.cpp`, and upstream's `LICENSE` as
   `port/world3d/upstream/LICENSE`.
3. Write `port/world3d/UPSTREAM.md`: the upstream URL and commit, the MIT notice (it requires the
   copyright line `Copyright (c) 2026 gebdag` to be kept), a table "file | copied from | changed since"
   (all "no" for now), and a "Changes" section (empty for now).
4. Commit (`Port: world3d 2.1, upstream's map, style and mesh code, as is`). Don't build them yet.

Check: `diff -r` of each copied file against the clone shows nothing.

### Task 2.2: compile them

1. Add the four `.cpp` files to `gta2_world3d` in `port/world3d/CMakeLists.txt` and
   `target_include_directories(gta2_world3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/upstream)`.
2. `ground.h` includes `"../../src/gta2_map.h"` and `"../../src/world_mesh.h"`: change those two includes
   to `"gta2_map.h"` and `"world_mesh.h"`.
3. Build linux32 and linux64. Fix what breaks with the smallest change, each listed under "Changes" in
   `UPSTREAM.md`. Expected kinds of trouble: the port's stand-in `windows.h` or `port_prelude.h`
   defining something upstream also defines (rename on upstream's side only if unavoidable), warnings
   from `-Wall -Wextra` (silence them for the upstream files with
   `set_source_files_properties(<files> PROPERTIES COMPILE_OPTIONS "-w")` rather than editing upstream
   code), and 64-bit `size_t`/`int` conversions.
4. Run the check (1.2, `--quick`).

If it fails: if a file needs more than ~20 changed lines to compile, stop and report which and why.

### Task 2.3: read the map and slopes from the game

Create `port/world3d/game_view.h` and `game_view.cpp` (add the `.cpp` to CMake). This is the **only**
file of world3d's scene code that includes game headers (`map_0x370.hpp`, `GameSession_578.hpp`,
`gtx_0x106C.hpp`, `TextureCache_15D8.hpp`, `Camera.hpp`, `MapRenderer.hpp`).

```cpp
// The parts of the running game the 3D renderer reads. Read only: nothing here changes the game.
namespace world3d
{
// The map the game has loaded, as upstream's gta2::Map. False when no map is loaded.
bool ReadMap(gta2::Map& out, std::string* error);
// The slope table (gGmpSlopes_6F5BA8) in upstream's form, 64 entries
void ReadSlopes(gta2::SlopeInfo out[gta2::kSlopeTypeCount]);
// The style file the game loaded ("data\\wil.sty"), or "" when none
std::string StylePath();
// A pointer that changes when the game loads another map (gMap_0x370_6F6268->field_0_pDmap)
const void* MapIdentity();
} // namespace world3d
```

`ReadMap` calls `out.LoadFromDmap(...)` with the arguments of 2.3. `MapIdentity` returns null when
`gMap_0x370_6F6268` or its `field_0_pDmap` is null.

Check: build, `--quick` check.

### Task 2.4: hook the map changes

1. In `include/port_world3d.h`, add `void port_world3d_map_changed();` (and its empty inline).
2. Call it, tagged, as the **last statement** of each function listed at the end of 2.3:
   `ChangeBlock_4E8620`, `AddNewBlock_4E87C0`, `RemoveBlock_4E8940`, `OnModifiedMapDataLoaded_4E8C00`,
   `LoadMap_4E95B0` (if a function has several `return`s, before each). `#include "port_world3d.h" //
   port: world3d` in `map_0x370.cpp`.
3. In `world3d.cpp`, the hook sets a flag `gMapDirty = true`.

Check: build both presets, full check (1.2): the corpus must still pass.

### Task 2.5: build the mesh when the map changes

In `world3d/scene.h` / `scene.cpp`:

```cpp
namespace world3d
{
// The world as triangles, in game coordinates (2.1), rebuilt when the game's map changes
struct Scene
{
    gta2::Map map;
    gta2::Style style;
    gta2::WorldMesh mesh; // vertices converted to game coordinates
    gta2::SlopeInfo slopes[gta2::kSlopeTypeCount];
    const void* identity = nullptr;
    std::string stylePath;
    bool valid = false;
};
// Brings `scene` up to date: reloads the style when its path changed, rebuilds the mesh when the map
// changed (dirty) or another map was loaded. Returns true when the mesh was rebuilt.
bool UpdateScene(Scene& scene, bool dirty);
} // namespace world3d
```

Steps inside `UpdateScene`: read `MapIdentity()`; if null, `valid = false` and return. If the style path
changed, `style.Load(StylePath(), &error)` (log the error and give up on failure). If dirty or the identity
changed: `ReadMap`, `ReadSlopes`, `gta2::SetWorldSeal(false)`, `gta2::SetRiverbedDepth(0)` (the black
background already does what they are for), `gta2::BuildWorldMesh(map, style, slopes, gta2::PartialCuts(),
&mesh)`, then convert every vertex and normal to game coordinates (2.1: `(X, Y, Z)` → `(X, 256 - Z, Y)`,
normals `(nx, ny, nz)` → `(nx, -nz, ny)`). Print once per rebuild:
`world3d: mesh built in N ms: V vertices, T triangles, B batches`.

Call `UpdateScene(gScene, gMapDirty)` from `port_world3d_present()` **only when the renderer is
`world3d`**, then clear `gMapDirty`.

Check:
- `--quick` check.
- Run the game with `GTA2_RENDERER=world3d` on each district (the user's game data:
  `~/Games/gta2/game`; start with `GTA2_RENDERER=world3d ~/Games/gta2/gta2-launch` or the replay runner
  with `--draw`), read the `world3d: mesh built` lines. Expect one build per level load, under 300 ms.
  Write the three times into `UPSTREAM.md` ("Measurements").

If it fails: a rebuild over 300 ms is fine for now (note it); a crash is not: stop and report the stack.

### Task 2.6: unit tests for the mesh

`port/world3d/tests/test_world3d_mesh.cpp`. Build tiny maps in memory with `gta2::Map::LoadFromDmap`
(make a 256x256 `base` of zeros pointing at column word 0, one column `{height, offset, block indices}`,
a few `gmp_block_info`-shaped 12-byte blocks) and a `gta2::Style` with no tiles loaded if the code allows
it (if `BuildWorldMesh` needs tiles, give the test a style built from a small fake `.sty`, or test through
`LidHeights` and `ResolveSlopeTable` only, and say so in the test's comment). Test invariants, not exact
counts:

- an empty map gives no triangles;
- one solid block with all five faces at cell (10, 20), level 0: every vertex lies in
  `x ∈ [10, 11], y ∈ [20, 21], z ∈ [0, 1]` after the conversion of 2.5 (factor the conversion into a
  function so the test can call it);
- a lid-only block gives triangles whose normals all point up (`z` component > 0.99 after conversion);
- `LidHeights` for a ramp rising north (direction 1), 2 steps: step 0 and step 1 share the edge height.

Check: `ctest` passes on linux32 and linux64.

## 4. Phase 3: the OpenGL world pass

Goal: with `renderer = world3d` the map is drawn from the mesh, in 3D, with the game's camera, shading
and lights, and looks like classic. Sprites, HUD and partial blocks still go through the classic path.

### Task 3.1: depth buffer and GL functions

1. `platform/sdl/sdl_platform.cpp`, `OpenWindow`: add `SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);` next
   to the other attributes (untagged: harmless for classic).
2. `platform/gl.h`: add to `PORT_GL_FUNCTIONS`, in the same `X(...)` form: `glGenBuffers`, `glDeleteBuffers`,
   `glBindBuffer`, `glBufferData`, `glBufferSubData`, `glUniformMatrix4fv`, `glDepthFunc`, `glDepthMask`,
   `glClearDepth` (OpenGL 2.1 signatures), and the constants they need (`GL_ARRAY_BUFFER 0x8892`,
   `GL_ELEMENT_ARRAY_BUFFER 0x8893`, `GL_STATIC_DRAW 0x88E4`, `GL_DYNAMIC_DRAW 0x88E8`,
   `GL_DEPTH_BUFFER_BIT 0x00000100`, `GL_LEQUAL 0x0203`, `GL_LESS 0x0201`, `GL_UNSIGNED_INT 0x1405`,
   `GL_TRIANGLES` if missing). Check each value against the OpenGL registry (`gl.xml`) or Mesa's
   `GL/gl.h`/`glext.h`, not from memory.
3. `renderer.cpp`, `GLRenderer::Present`: clear the depth buffer together with the colour
   (`glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)`, `glClearDepth(1.0)` once at open).

Check: `--selftest` passes with both renderers; `--quick` check.

### Task 3.2: the camera matrix

`world3d/camera.h`/`.cpp` (world3d's own, not upstream's `camera.h`):

```cpp
namespace world3d
{
struct GameCamera
{
    float camX, camY, camZ, f, cx, cy; // 2.2
    bool valid;
};
GameCamera ReadCamera(); // in game_view.cpp: from gViewCamera_676978; valid = false when null
// The column-major clip transform of 2.2 for a W x H screen
void ClipMatrix(const GameCamera& cam, float W, float H, float out[16]);
// The game's own projection of 2.2, for tests: screen x, y and 1/D
void GameProject(const GameCamera& cam, float x, float y, float z, float& sx, float& sy, float& invD);
} // namespace world3d
```

Test (`tests/test_world3d_camera.cpp`): for a camera `{camX 100.5, camY 80.25, camZ 2, f 640, cx 320,
cy 240}` and a few points (on the eye's axis, at the screen corners, at z 0..5), multiply by `ClipMatrix`,
divide by w, map NDC to pixels (`px = (ndc.x + 1) * W/2`, `py = (1 - ndc.y) * H/2`) and check equality
with `GameProject` within 0.01 px; check NDC z is in (-1, 1) for D in (n, F).

### Task 3.3: check the camera against the game, live

Before drawing anything, prove the matrix matches the game on real frames. In `port_world3d_draw`, when
the renderer is `world3d`, `kind == kPortWorld3dTile` and `$GTA2_WORLD3D_CHECK` is set: for each of the 4
vertices, project the **world** position (`verts[i + 4].x/.y/.z`) with `ClipMatrix` and compare with the
game's screen position `verts[i].x/.y`. Keep the largest error per frame; print
`world3d: camera error max E px over N tiles` once per second. Still return false (classic draws).

Check: run a replay with `--draw` and `GTA2_RENDERER=world3d GTA2_WORLD3D_CHECK=1`. The maximum error must
stay under 0.5 px. Note the number in `UPSTREAM.md`.

If it fails: compare a single vertex by hand against 2.2. Large constant offsets mean wrong `cx/cy`;
mirrored positions mean a sign in the matrix; errors growing with height mean the `+ 8`. Stop and report
if it is not one of these.

### Task 3.4: draw the mesh

In `world3d/gl/world_pass.{h,cpp}` (include `platform/gl.h`; add `${PORT_DIR}/platform` to the includes,
already there since phase 1):

1. On a mesh rebuild, upload `mesh.vertices` (positions in game coordinates, plus UV) and `mesh.indices`
   into a static VBO/IBO. Add a dynamic colour buffer, one RGBA8 per vertex.
2. A shader (GLSL 1.20, like `renderer.cpp`'s): `attribute vec3 aPos; attribute vec2 aUV; attribute vec4
   aColor; uniform mat4 uClip;` → `gl_Position = uClip * vec4(aPos, 1.0)`. Fragment: sample, `discard` if
   alpha < 0.5, multiply by colour. Bind attributes with `glBindAttribLocation` before linking (as
   `GLRenderer::Link` does).
3. Per frame, at the **first** `kPortWorld3dTile` hook call of the frame (2.5 says why), when the scene is
   valid: compute per-vertex colours for vertices inside the camera's view (cull by batch bounding box if
   it's slow) with the formula of 2.5, using the face shade of 2.5 for the face the vertex belongs to
   (store a "face kind" per vertex when converting the mesh in 2.5: lid, west, east, north, south,
   diagonal, ramp; derive it from the normal and the block's slope). Then set the viewport and the 4:3
   area exactly as `GLRenderer::SetViewport`, enable depth test (`GL_LESS`) and depth writes, draw every
   batch whose tile has a texture (2.4: look up `idx`/`tex` per batch per frame; skip when `idx == 0`),
   then restore the classic renderer's GL state: its program, its vertex attribute pointers, depth test
   off, blend as before. **Restoring state is the most likely bug: the classic renderer assumes its own
   program and client-side vertex arrays; unbind the VBO (`glBindBuffer(GL_ARRAY_BUFFER, 0)`) and call
   `glUseProgram(classic)` and its `glVertexAttribPointer` setup again.** Expose what is needed from
   `renderer.cpp` through a small function in `renderer.h`, e.g. `void port_renderer_restore_state();`,
   tagged.
4. Lighting data: `renderer.cpp` keeps the lights in `gLights`/`gNumLights`/`gAmbient`. Add to
   `renderer.h` a read-only accessor (e.g. `int port_renderer_lights(const PortLight** out, float*
   ambient)` with a plain struct) and use it; don't copy the arrays by hooks.
5. From then on, return **true** from the tile hook for every tile of a block whose slope type is **not**
   49..61 (the hook of `RenderBlockAt_4F6880` gave you the cell: look the block up with
   `gMap_0x370_6F6268->get_block_4DFE10(x, y, z)`; `z` from the hook is the raw `gZCoord_6F63E0`, check in
   `MapRenderer.cpp` how it becomes a level before using it). Partial and corner blocks keep drawing
   through the classic path, painted over the 3D world, which looks right because the camera matches.

Check: full check (1.2): the corpus still passes with world3d drawing. Then task 3.6's comparison.

### Task 3.5: frame capture in the replay runner

`gta2 --replay` gets `--screenshot N:FILE` (repeatable): after update `N` has been drawn, the presented
picture is written to `FILE` as binary PPM (`P6`). In `replay.cpp`, parse it into the options; in
`renderer.cpp`'s `Present`, before `SwapWindow`, ask the replay runner whether a capture is due (a new
function in `include/port_replay.h`, e.g. `const char* port_replay_capture_file();` returning null when
not) and write `port_renderer_read_pixels` (rows are bottom-up RGBA) to it. Only meaningful with
`--draw`.

Check: capture update 300 of `00-synthetic-walk` with `--draw` and open the PPM (or convert with Python
and look at it): it shows the game.

### Task 3.6: compare world3d with classic

`port/tools/world3d/compare_frames.sh BUILD_DIR`: for each corpus replay, capture updates 200, 400 and 600
with `--draw`, once with `GTA2_RENDERER=classic` and once with `world3d`, then run
`port/tools/world3d/compare_frames.py A.ppm B.ppm DIFF.png` which prints `differing pixels: P%` (a pixel
differs when any channel differs by more than 24) and writes a picture of the differences. Use only the
Python standard library (write PNG with `zlib`), run it with `python3 -I`.

Check: run it. Expect a few percent at most (texture filtering and edges). Record the numbers per frame in
`UPSTREAM.md`; from now on a task that raises any of them by more than 2 points must explain why in its
commit message.

If it fails (over 10%): look at the difference picture. Whole faces in the wrong grey: the shade table
(2.5). Everything shifted: the viewport. Missing walls: the batch texture lookup or the slope types left to
classic. Stop and report if none of these.

### Task 3.7: Graphics page text

The renderer row's value for `world3d` currently says "in development: draws as classic". Change it to
`3D`. Add a second row `World` with values `mesh` / `classic tiles` that lets the player switch the mesh off
while keeping the rest of world3d (useful to compare by eye). Store it in `graphics.txt` as
`world_mesh = 1|0`, with a unit test of the round trip in `tests/test_world3d_settings.cpp`.

## 5. Phase 4: depth for the 2D sprites

Problem after phase 3: the mesh is drawn all at once, so a car under a bridge is painted over the bridge.

### Task 4.1: depth-tested sprite draws

When the renderer is `world3d` and a sprite is active (hook 2), the classic path draws its quads with depth
test against the mesh: in `renderer.cpp`'s classic vertex shader, add an optional depth: `gl_Position.z`
from the vertex's `D = 1/Vert.z` with the same `A`, `B` as 2.2 (`z_ndc = A + B/D`, so `gl_Position.z =
(A + B/D) * w` where `w` is what the shader already uses), enabled by a uniform `uDepth` that is 0 for every
draw except sprites in world3d mode. Depth test `GL_LEQUAL`, no depth writes. Pull the sprite towards the
camera by a small bias (`D - 0.05`) so it doesn't fight the ground it stands on. Tag the lines.

Check: frame comparison (3.6) stays within its numbers; find a bridge in a replay (or the user's play
test) and confirm a car under it is hidden. Full check (1.2).

### Task 4.2: the same for score popups and world text

The draws inside `port_world3d_world_text(true)` get the same depth treatment. The HUD (hook 1) never does.

## 6. Phase 5: lighting extras (render only)

Each item is a row on the Graphics page and a key in `graphics.txt` (default off), with a unit test for the
settings round trip, and must keep the full check green (they change pixels only). Do them in this order;
each is one task and one commit.

5.1 **Smooth lighting**: compute the 2.5 lighting per pixel instead of per vertex (pass world position to
the fragment shader; the 16 lights nearest the camera as uniforms, the rest dropped). Key
`smooth_lighting`.

5.2 **Time of day**: copy upstream `dll/src/time_of_day.{h,cpp}` into `upstream/` (verbatim commit
first), remove its Remix push (`remix_api.h`), keep the sun position maths. Use the sun elevation to scale
and tint the ambient: full at noon, the game's ambient at night. Keys `time_of_day` (0 off, else the start
hour) and `day_minutes`.

5.3 **Night lights**: with time of day on, street lights fade with the sun as upstream's night gate does
(`remix_lights.cpp`: copy `ClassifyGameLight` and the gate's smoothstep only).

5.4 **Invented lights**: muzzle flashes, fire, headlights: from upstream's `synthetic_lights.cpp`. Read
particles and cars read-only through new functions in `game_view.cpp` (find the pools from upstream's
addresses with `survey.py --addrs` and the class names it prints). Add them to the light list for the mesh
only. Key `invented_lights`.

5.5 **Additive effects**: upstream's effect-sprite test (`texture_store.cpp`, "edge median < 16"), run on
each sprite texture when the classic renderer uploads it; effect sprites drawn with additive blending in
world3d mode. Key `effects_additive`.

## 7. Phase 6: widescreen, blood, gibs (blocked: decision 4)

Do not start until the user has answered decision 4. Sketch for then: a hook in
`Camera_0xBC::UpdateBoundaries_435B90` replacing `dword_6768E0` (0.75) by `H/W` of the chosen aspect, read
**once when a game starts** and written into the replay as a port record (`include/port_input.h`,
`kPortRecord`), forced off in network games; the world pass renders the full window width; HUD parts
anchored left/right as upstream's `hud_pass.cpp` table says (the 19 calls in `Hud_2B00::DrawGui_5D6860`).
Blood: `bDo_blood_67D5C5` set from the setting in `Debug::Init_4ABBD0`. Gibs: upstream `gibs.cpp` hooked
in `Ped::Kill_46F9D0`. Each needs new corpus replays recorded with it on.

## 8. Phase 7: Windows x64 build

Goal: `gta2.exe` for 64-bit Windows, cross-compiled here, running under Wine with the classic and world3d
renderers.

7.1 Install `g++-mingw-w64-x86-64` and the SDL2 MinGW development package (download
`SDL2-devel-<version>-mingw.tar.gz` from SDL's GitHub releases into `~/sdl-mingw`, like
`tools/setup_sdl32.sh` does for 32-bit). Write `tools/setup_sdl_mingw.sh` for it.
7.2 `port/cmake/x86_64-w64-mingw32.cmake` (toolchain file: `CMAKE_SYSTEM_NAME Windows`, the MinGW
compilers, `CMAKE_FIND_ROOT_PATH`), and a preset `win64`.
7.3 Build. Expected trouble, in order: (a) `port/include/` stand-in headers named like MinGW's
(`windows.h`, `dinput.h`): the game and `platform/` must keep seeing the port's, `platform/sdl/` the real
ones (it already doesn't get the port's include path); (b) the port defines Win32 functions
(`CreateWindowExA`, `GetProcAddress`, ...) that MinGW's import libraries also provide: link with
`-static` and check with `x86_64-w64-mingw32-nm` that the port's definitions are used; (c) POSIX calls in
`platform/` (`mkdir` with a mode, `strcasecmp`, sockets in `platform/net/`): wrap them in a small
`platform/posix.h` with Windows versions. Fix each in its own commit.
7.4 Run under Wine: `wine build_port/win64/gta2.exe --selftest` (needs SDL2.dll next to it), then the
corpus with Wine (`replaytest.sh` takes a build dir; add a `GTA2_RUNNER=wine` variable). The hashes must
equal the Linux ones.

If 7.3 needs more than ~15 commits or touches game code, stop and report: the design may need a
different split.

## 9. Phase 8: RTX Remix backend

8.0 **Spike first:** install the RTX Remix runtime under Wine on this machine (RTX 3060 laptop:
`__NV_PRIME_RENDER_OFFLOAD=1 __VK_LAYER_NV_optimus=NVIDIA_only`) and run any small x64 D3D9 program with
Remix's `d3d9.dll` next to it. If Remix doesn't come up, stop and report: the rest needs a Windows PC.
8.1 `world3d/remix/`: upstream `src/renderer.{h,cpp}` (verbatim commit first), adapted to draw the
`Scene` with Direct3D 9 fixed function on the SDL window's `HWND` (`SDL_GetWindowWMInfo`). Only in the
`win64` build. Renderer value `remix` in `graphics.txt`, offered only when `d3d9.dll` loads and
`remixapi_InitializeLibrary` succeeds.
8.2 Sprites as 3D quads with upstream's `live_geometry.cpp` (world corners from 2.6, the `Sprite*` from
hook 2 as the identity instead of upstream's matching), one draw each, transform in `D3DTS_WORLD`.
8.3 Lights through the Remix API: upstream `remix_api.cpp`, `remix_lights.cpp`; the sun through
`rtx.atmosphere.sunElevation`/`sunRotation` from 5.2's time of day.
8.4 Package `rtx.conf` (`rtx.zUp = False`), upstream's Remix mods, and a player README.

## 10. Phase 9: tuning panel (blocked: decision 7)

Do not start until the user has answered decision 7. Sketch: Dear ImGui 1.91 vendored as
`port/3rdParty/imgui` (not GTA2Hax's 1.50 copy, which the VC6 build needs), SDL2 + OpenGL2 backends, F4,
upstream's `debug_overlay.cpp` tabs that apply, behind `GTA2_WORLD3D_DEVTOOLS`.

## 11. Removing world3d

1. For a player: `renderer = classic` in `graphics.txt` (the default) or on the Graphics page.
2. For a build: `cmake -DGTA2_WORLD3D=OFF` (preset `noworld3d64`).
3. From the tree: `git rm -r port/world3d port/include/port_world3d.h port/tools/world3d`, then delete
   every line `grep -rn "port: world3d" port/ .github/` lists (a tagged `if` with its block), then build
   and run `tools/replaytest.sh`. `check_removable.sh` prints the current list.
