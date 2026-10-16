# world3d: GTA2 RTX Remix in the port, the plan

Plan for bringing [gebdag/gta2-rtx-remix](https://github.com/gebdag/gta2-rtx-remix) (MIT, measured at upstream
commit `d51b19b`, 2026-10-05) into the native port. Like the pause menu (`esc_menu = 0`), it has to be
**switchable in a settings file and in game**, and **removable** by deleting one directory, one header and a
short list of tagged lines.

Measurements come from `port/tools/world3d/survey.py` (re-run it against a newer upstream checkout before
starting a phase):

```bash
git clone --depth 1 https://github.com/gebdag/gta2-rtx-remix /tmp/rtx
python3 -I port/tools/world3d/survey.py /tmp/rtx --addrs
```

## 1. What upstream is

A replacement for GTA2's renderer DLL (`gta2dx9.dll`, the 40 `gbh_*` exports) for the original
`gta2.exe` **v9.6f**. Rather than draw the game's screen-space stream, it rebuilds the world in 3D and
hands it to Direct3D 9 so RTX Remix can path trace it:

| Feature | How upstream does it | Renderer only, or changes the game? |
|---|---|---|
| World mesh | Reads the live map (`Map_0x370`) at fixed addresses, works out which `.gmp`/`.sty` it is by comparing blocks to the files on disk, builds every block as world-space triangles (`src/world_mesh.cpp`, 1,105 lines, all slope types) | renderer |
| Camera | Perspective, top-down (pitch -90), zoom taken from the camera's view rectangle (`+0x78..+0x84`) | renderer |
| Sprites in 3D | Cars, peds, pickups: reads the world position the game stores 4 vertices after each projected one (`SaveUnprojectedVertex_4B9990`), tells world sprites from UI by `_ReturnAddress()`, one draw per sprite with a stable object-space quad so Remix gets motion vectors | renderer |
| Partial and corner blocks | Captured from the game's own `gbh_DrawTile` stream the first time they are seen | renderer |
| HUD on a wide screen | Patches the 19 calls in `Hud_2B00::DrawGui_5D6860` to know what is HUD, anchors each part left or right | renderer |
| Game lights | `gbh_AddLight` lights become Remix sphere lights; classified (street, neon, traffic signal, car lamp) by colour and motion | renderer |
| Invented lights | Muzzle flash, bullets, sparks, cigarettes, fire, headlights, brake lights: read from the particle and car pools | renderer |
| Time of day | Sun position from solar equations, pushed to Remix config variables; night-only lights fade at dusk | renderer |
| Effect sprites | Fire and explosions drawn additively, picked by analysing the artwork (`texture_store.cpp`); alpha bleed | renderer |
| **16:9 without pop-in** | Patches the `0.75` in `Camera_0xBC::UpdateBoundaries_435B90` so the view rectangle the game spawns and culls by is as wide as the screen | **game** (where traffic and peds spawn) |
| **Blood** | Sets the debug flag `bDo_blood_67D5C5` | **game** |
| **Gibs** | Re-sprites the blood burst particles of exploded or run-over peds with the unused limb sprites | **game** |
| Frame cap, intro movie, video device proxy | Workarounds for the Windows DLL setup | not needed in the port |
| F4 panel | Dear ImGui 1.91.7 over the D3D9 device: every tuning value, saved to `gta2dx9_settings.ini` | tooling |

Its `dll/README.md` is a valuable reverse-engineering record (the HUD's 19 parts, the light format,
Remix's motion-vector rules). It goes into the port as `port/world3d/UPSTREAM_NOTES.md`, with every
address translated to the port's function names.

## 2. What it means for the port

**The port has the source, so most of upstream's hardest code disappears.** Upstream reads 185 named code
addresses, 62 more in its address table (`game_access.h`), and patches machine code in over 20 places. In the port every
one of those is a function or a struct field: `survey.py --addrs` translates 75 of them directly (9.6f
address to 10.5 name through `Scripts/ida/functions_data.json`); the rest are call sites inside those
functions or 9.6f helpers inlined in 10.5 (`docs/inlines_96f.md`). Examples:

| Upstream (9.6f) | Port |
|---|---|
| patch `PUSH 0x005E3F34` at `0x0041E8E4` (the 0.75) | `Camera_0xBC::UpdateBoundaries_435B90`: `v *= dword_6768E0` |
| `_ReturnAddress()` == object draw `FUN_004BE060` | `SpriteTree_4::Draw_5C5DF0` (10.5 inlined the 9.6f helper), which calls `Sprite::Draw_59EFF0` per object: a scope flag around that call |
| patch the call to `FUN_004CA440` | `Game_0x40::Draw_4B92D0` calls `Hud_2B00::DrawGui_5D6860`: scope flag |
| `*(u8*)0x005EAD51` | `bDo_blood_67D5C5` (`debug.cpp:168`) |
| hook `FUN_004411B0` | `Ped::Kill_46F9D0` |
| particle pool at `0x00669E70`, car pool at `0x005E4CA0` | the pools' own classes (`Particle_8`, `Car_BC`), read-only |
| find the `.gmp` by comparing blocks | `gMap_0x370_6F6268` and `gGtx_0x106C_703DD4` are loaded already |

**RTX Remix is Windows-only.** NVIDIA's runtime (`dxvk-remix`) builds for Windows x64 only and there is
no native Linux build ([NVIDIAGameWorks/dxvk-remix](https://github.com/NVIDIAGameWorks/dxvk-remix)). On
Linux, Remix games run as Windows programs under Wine/Proton, where `dxvk-remix` reaches Vulkan through
winevulkan. So path tracing needs a **Windows build of the port** and, on this machine, Wine. This machine
has the hardware for it (GeForce RTX 3060 Laptop, driver 580, plus an AMD iGPU: PRIME offload needed).

That splits the work into a part every build gets and a part only Remix gets:

- **Native (every build, Linux/ARM, no Remix):** a world-space 3D renderer over OpenGL that uses
  upstream's mesh, camera, sprite, HUD and light logic. Real 3D sprites, any window size, 16:9, per-pixel
  lighting from the game's lights, day/night tint. No path tracing. This also does the TODO item
  "Higher resolutions than 640x480".
- **Remix (Windows x64 build, Windows or Wine/Proton):** the same scene drawn through Direct3D 9
  fixed-function (upstream's `src/renderer.cpp`) plus the Remix API for lights and the sun. As a 64-bit
  program the port loads Remix's x64 `d3d9.dll` directly: **no 32-bit bridge**, so none of upstream's
  bridge round-trip throttling or its topmost-window and second-device workarounds apply.

### Upstream code by fate (survey.py)

| Category | Files | Lines | What happens |
|---|---|---|---|
| keep | 12 | 2,673 | mesh, ground sampling, camera maths, alpha bleed, time of day, settings table: taken over with small changes |
| adapt | 18 | 7,979 | sprites, partial blocks, texture store, invented lights, gibs, HUD pass, 2D pass, world view: logic kept, every address read and patch replaced by the game's structs and hooks |
| remix | 6 | 2,276 | D3D9 renderer, Remix API, Remix lights: only in the Windows build |
| devtools | 4 | 2,155 | the F4 ImGui panel and light benchmark: optional, behind its own option |
| drop | 10 | 2,770 | address table, DLL proxy, export forwarding, frame limiter, Bink workaround, video device proxy |
| **total** | 50 | 17,853 | plus upstream's `tools/` (Python and C++, ~1,300 lines): only `make_water_mod.py` and `lamp_emissives.py` (the Remix mods) are of use |

## 3. Design

### 3.1 Where the code goes

```
port/world3d/                 everything of this feature; delete it and it is gone
  PLAN.md  UPSTREAM.md         provenance: upstream commit, file-by-file origin, MIT notice
  UPSTREAM_NOTES.md            upstream's dll/README.md, addresses translated to port names
  CMakeLists.txt               library gta2_world3d (GL backend), gta2_world3d_remix (Windows only)
  scene/                       backend-free: map adapter, world mesh, ground, camera, sprites,
                               partial-block capture, lights (game + invented), time of day, settings
  gl/                          the OpenGL 2.1 backend (Linux, ARM, Windows)
  remix/                       the D3D9 + Remix API backend (Windows x64 only)
  devtools/                    ImGui tuning panel (optional, GTA2_WORLD3D_DEVTOOLS)
  tests/                       unit tests, linked into gta2_tests when the option is on
port/include/port_world3d.h   the only header game/ includes: hooks, no-ops when the feature is off
port/tools/world3d/           survey.py, check_removable.sh
```

`scene/` reads the game through a small read-only query layer (`scene/game_view.cpp`, the only file there
that includes `game/` headers; `platform/statehash.cpp` and `controls.cpp` already do the same). It
replaces upstream's 583-line `game_access.h`.

### 3.2 Seams in the game and the platform

Everything game/ calls goes through `include/port_world3d.h`. With `GTA2_WORLD3D=OFF` its functions are
`inline` and empty, so the call sites stay and cost nothing. Every call site carries `// port: world3d`.

| # | Where (port/game) | Hook | Affects the simulation? |
|---|---|---|---|
| 1 | `Game_0x40::Draw_4B92D0` around `Hud_2B00::DrawGui_5D6860` | `port_world3d_hud(true/false)` | no |
| 2 | `SpriteTree_4::Draw_5C5DF0`, around each `Sprite::Draw_59EFF0` | `port_world3d_sprite_begin(sprite)` / `_end()`; gives the `Sprite*` itself as the identity, so stable per-object quads need no matching heuristic | no |
| 3 | `ExplodingScorePool::DrawExplodingScores_5969E0` | `port_world3d_world_text(true/false)` (score popups stay over the world, not with the HUD) | no |
| 4 | `MapRenderer::RenderBlockAt_4F6880` | `port_world3d_block(x, y, z)` before a block is drawn (partial-block capture knows its cell) | no |
| 5 | `Map_0x370::AddOrUpdateDmapInfo_4E80E0` and the other map writes (script block changes; the full list is task 2.3) | `port_world3d_block_changed(x, y, z)`; upstream never rebuilds after a change | no |
| 6 | `Camera_0xBC::UpdateBoundaries_435B90` | `v *= port_world3d_view_aspect()` (returns `dword_6768E0`, 0.75, unless widescreen spawning is on) | **yes** |
| 7 | `Debug::Init_4ABBD0` | `bDo_blood_67D5C5 \|= port_world3d_blood()` | **yes** |
| 8 | `Ped::Kill_46F9D0` | `port_world3d_gibs(ped, cause)` | **yes** |

In `platform/` the GL backend works inside the existing renderer instead of beside it, the way the overlay
does: `renderer.cpp`'s `gbh_*` functions call `world3d::` first when it is active (draw a tile → skip it,
the mesh has it, or capture it if it is a partial block; draw a quad → a world sprite, or queue it for the
2D pass; `gbh_AddLight` → also to the light list; `gbh_EndScene` → world, then 2D pass, then overlay).
About 15 tagged lines in `renderer.cpp` and one in `CMakeLists.txt`.

### 3.3 Settings and switching

A file like `pad.txt`: `~/.config/gta2-native/graphics.txt`, written through a temporary file like the
registry, and a **Graphics** page in the overlay (reached from the pause menu and the settings screen).

```ini
renderer = classic        # classic | world3d | remix (remix only offered in a Windows build with Remix present)
widescreen_spawning = 0   # 1: the game spawns and culls for the window's width (changes the game)
blood = 0                 # 1: blood pools (changes the game)
gibs = 0                  # 1: limbs from explosions and run-overs (changes the game; needs blood)
lights_invented = 1       # muzzle flash, bullets, sparks, cigarettes, fire, headlights
time_of_day = 0           # 0 off, else the starting hour; day_minutes = length of a day
effects_additive = 1      # fire and explosions drawn additively
```

- `classic` is the default and is exactly today's renderer: no world3d code runs.
- Renderer-only settings change live. The three that change the game are read **when a game starts**,
  never while it runs, and are written into the replay as a port record (the mechanism the pad uses,
  `kPortRecord`), so a replay plays back with the values it was recorded with. The aspect for
  `widescreen_spawning` is fixed at game start too (from the window then), never taken from the live
  window: resizing the window must not change the game.
- **Network games** force the three game-changing settings off, as the pause menu keeps the original
  quit question there: every machine simulates every player's camera, so they would all have to agree.
  (Later: the host's values in the session settings, see section 6.)

### 3.4 How it is removed

1. Off for one player: `renderer = classic` (or the Graphics page). Nothing of world3d runs.
2. Off for a build: `cmake -DGTA2_WORLD3D=OFF`. Nothing of world3d is compiled; the hooks are empty
   inlines.
3. Gone from the tree: `git rm -r port/world3d port/include/port_world3d.h port/tools/world3d`, then
   delete the lines `grep -rn "port: world3d" port/` lists (target: under 40) and the CMake option.

`port/tools/world3d/check_removable.sh` proves 1 and 2 in CI: it builds with the option on and off and runs
the replay corpus on both with `renderer = classic`; the state-hash logs must be byte-identical, and equal
to the logs in the corpus. A second run with `renderer = world3d` (and the three game settings off) must
give the same hashes too: drawing in 3D may not change the game. That check is the main guard on every
phase below.

## 4. Phases

Effort is focused working days; "mechanical" is the share that is copying, renaming and wiring rather
than judgement. Each phase ends in commits that build and pass `ctest`, `--selftest`, `--audiotest` and the
replay corpus on linux32, linux64 and clang64.

| Phase | Delivers | Effort | Mechanical |
|---|---|---|---|
| 0 | Spikes: Remix under Wine here, upstream release runs; decisions taken | 1 | 30% |
| 1 | Seams, settings, Graphics page, removability check; `world3d` draws exactly what classic does | 2 | 70% |
| 2 | Scene layer: map adapter, mesh, ground, camera, unit tests | 3-4 | 50% |
| 3 | OpenGL world renderer: world, sprites, partial blocks, 2D pass, any window size, golden frames | 5-7 | 25% |
| 4 | Lighting in GL: game lights, invented lights, time of day, additive effects | 3-4 | 35% |
| 5 | The three game-changing options: widescreen spawning, blood, gibs | 2-3 | 40% |
| 6 | Windows x64 build of the port (MinGW-w64 cross), runs under Wine | 4-8 | 40% |
| 7 | Remix backend: D3D9 scene, Remix lights and sun, Remix mods packaged | 3-5 | 50% |
| 8 | Dev tuning panel (ImGui 1.91, SDL2 + OpenGL and D3D9 backends) | 2-3 | 60% |
| | **Total** | **25-37** | **~40%** |

Phases 1-5 give every build the 3D renderer and stand on their own. 6-7 are the Remix part and can be
dropped without losing anything from 1-5. 8 is optional at any point after 3.

### Phase 0: spikes (1 day)

0.1 **Does Remix run on this machine at all?** Install upstream's release on the original 9.6f
`gta2.exe` under Wine with `dxvk-remix` (Remix runtime 1.1.x), PRIME offload
(`__NV_PRIME_RENDER_OFFLOAD=1 __VK_LAYER_NV_optimus=NVIDIA_only`). Record what works in
`UPSTREAM.md`. If Remix cannot be made to run under Wine here, phases 6-7 stop until there is a Windows
machine to test on; 1-5 are unaffected.
0.2 A stock x64 D3D9 sample (any small SDL2 or plain Win32 D3D9 triangle) built with MinGW-w64 and run
under Wine with Remix's x64 `d3d9.dll` in place: confirms the bridge-less x64 route before phase 6.
0.3 Decisions in section 7 answered.

Exit: a written result for 0.1 and 0.2, and the answers.

### Phase 1: seams and switches (2 days, 70% mechanical)

1.1 `port/world3d/` with `CMakeLists.txt`, `UPSTREAM.md` (commit, licence, file map), option
`GTA2_WORLD3D` (default ON once phase 3 lands, OFF before).
1.2 `include/port_world3d.h` with the eight hooks of 3.2 as no-ops; the call sites in game/, tagged.
1.3 `graphics.txt` load/save (model it on `padinput.cpp`'s `Load`/`Save`, same temp-file write), unit test
for the round trip.
1.4 Graphics page in the overlay (`platform/overlay.cpp`), entry from the pause menu and settings.
1.5 `renderer.cpp`: the dispatch points into `world3d::`, which in this phase forwards everything (upstream
calls this "proxy" mode).
1.6 `tools/world3d/check_removable.sh` and a CI job running it.

Exit: `renderer = world3d` draws pixel-identical frames to classic (`--selftest` passes in both), the
corpus hashes are identical with the option on and off, `grep -c "port: world3d"` is recorded in
`UPSTREAM.md`.

### Phase 2: scene layer (3-4 days, 50% mechanical)

2.1 `scene/game_view.cpp`: read-only access to the map (`gMap_0x370_6F6268`: columns, blocks, slope table), the
style's tile remap (`gGtx_0x106C_703DD4->GetTile_5AA870`), the view camera (`gViewCamera_676978`: position,
zoom, view bounds), particles and cars. Plain structs out, no game types in the rest of `scene/`.
2.2 Take over `math3d.h`, `camera.{h,cpp}`, `world_mesh.{h,cpp}`, `ground.{h,cpp}`, `alpha_bleed.h`.
Replace `gta2::Map::Load(path)` by a `Map` built from `game_view` (the `.gmp` detection goes away). Keep
upstream's formatting and names inside these files so later upstream fixes can be merged by diff; record
any change in `UPSTREAM.md`.
2.3 Mesh per 16x16-cell chunk instead of one buffer, so hook 5 can rebuild one chunk. List every function
that writes the map at runtime (`AddOrUpdateDmapInfo_4E80E0`, the script's block commands) and hook them.
2.4 Unit tests (doctest, `world3d/tests/`): triangle counts and normals per slope type (0-63) from a
synthetic map, lid heights from `ground.cpp` against hand-computed ramps, the camera's visible rectangle
against `Camera_0xBC`'s `field_78_view_bounds` for several zooms.

Exit: tests pass on 32 and 64 bits; building the mesh for each of the three districts (`wil`, `ste`, `bil`)
from a running game takes under 100 ms and is logged once.

### Phase 3: OpenGL world renderer (5-7 days, 25% mechanical)

3.1 Window and context: a depth buffer in `plat::OpenWindow` (the port asks for none today), VBOs (add to `gl.h`'s function list), GLSL
120 kept so the port's GL 2.1 floor and ARM handhelds still work.
3.2 Tile textures: upstream's `texture_store` keyed on the game's `STexture*` (the renderer sees every
`gbh_RegisterTexture`/`gbh_UnlockTexture` already), one texture array page per 256x256 style page,
half-texel inset (upstream's seam note).
3.3 World pass: perspective camera from `scene/camera`, pitch from settings (default -90, upstream's
choice), chunk VBOs, alpha test at 0.5 like the classic shader.
3.4 Sprite pass: in `gbh_DrawQuad` with hook 2 active, take the world corners the game stored 4 vertices
on (`SaveUnprojectedVertex_4B9990`), draw as a depth-tested quad lifted by upstream's ground conform
(`live_geometry.cpp`: `kConformClearance`, `kDefaultSpriteHeight`). The `Sprite*` keys the stored quad.
3.5 Partial blocks (slope types 49-61): upstream's capture from `gbh_DrawTile` with hook 4's cell.
3.6 2D pass: everything else in `gbh_DrawQuad`/`DrawTriangle`/`DrawFlatRect`/`BlitImage` queued during the
scene and drawn after the world, scaled from 640x480; HUD parts anchored per upstream's `hud_pass` table
(the 19 parts of `Hud_2B00::DrawGui_5D6860` by name).
3.7 Any window size: the world renders at the drawable size; classic keeps its 4:3 bars.
3.8 Golden frames (phase 0 task 0.10 of the refactor plan, still open): render frames N of two corpus
replays headless under Xvfb with `renderer = world3d`, compare to stored PNGs with a per-pixel tolerance
(Mesa's llvmpipe in CI, so no driver differences).

Exit: the three districts play with `renderer = world3d` at 1280x720 and 1920x1080, sprites stand on
ramps without clipping, HUD sits at the edges, the corpus hashes are unchanged, golden frames pass.
Play-test by the user.

### Phase 4: lighting in GL (3-4 days, 35% mechanical)

4.1 Game lights from `gbh_AddLight` (already decoded in `renderer.cpp`) as per-pixel point lights: the
16 nearest the camera in the shader, the rest dropped; ambient from `gbh_SetAmbient`.
4.2 `ClassifyGameLight` (from `remix_lights.cpp`, backend-free) and the night gate by sun elevation.
4.3 Invented lights (`synthetic_lights.cpp`): particle types bound to categories (upstream binds them in
its panel; here the bindings found in its shipped `package/tuning/gta2dx9_effects.ini` become defaults),
car headlights and brake lights from the car pool.
4.4 Time of day (`time_of_day.cpp`): sun direction and colour drive a directional term and the ambient,
instead of Remix config variables.
4.5 Effect sprites additive (`texture_store` analysis), unit test on the luminance thresholds with a few
synthetic textures.

Exit: night scenes lit by street lights and headlights at 60 fps on the laptop's iGPU at 1080p; turning
`time_of_day` off gives the classic brightness.

### Phase 5: options that change the game (2-3 days, 40% mechanical)

5.1 Widescreen spawning (hook 6). The aspect is set at game start and recorded. Check: a recorded 16:9
replay plays back equal; a 4:3 replay is unaffected; in a 16:9 window no car appears inside the frame.
5.2 Blood (hook 7).
5.3 Gibs (hook 8, upstream's `gibs.cpp`: re-sprite the six particles of `Particle_8::EmitBloodBurst_53E450`
with the limb sprites, code_obj 389-399). Check it uses no extra random numbers, or record that it does.
5.4 The port record for the three settings, written at game start, read back in replays; replay test with
each on (new corpus entries `00-synthetic-wide`, `00-synthetic-gibs`).
5.5 Network games: forced off, with a line in the lobby output saying so.

Exit: corpus passes; the new corpus entries pass on all three builds; network test (`tools/nettest.sh`)
unchanged.

### Phase 6: Windows x64 build (4-8 days, 40% mechanical, the most uncertain)

The port is Linux-only today: its `include/` stand-ins for `windows.h` and friends stand in for the
Windows API that `platform/` implements over SDL. A Windows build keeps that design (the game still sees
the stand-ins, `platform/` still implements them) and only the SDL side (`platform/sdl/`, which does not
see the prelude) and the Remix backend see the real Windows headers.

6.1 Toolchain: `cmake/x86_64-w64-mingw32.cmake`, `apt install g++-mingw-w64-x86-64`, SDL2's MinGW
development package (like `setup_sdl32.sh`).
6.2 Symbol clashes: the port defines `CreateWindowExA`, `GetProcAddress`, `timeGetTime`, ...; make sure
the linker takes the port's (static definitions win over import libraries, but `-Wl,--exclude-libs` or a
rename prefix may be needed). `port/platform/crt.cpp`'s path mapping becomes a no-op on Windows.
6.3 `-fshort-wchar` is the native `wchar_t` there; the `wcs*` redirections in `wchar.cpp` stay harmless.
6.4 CI job: cross-build, then `--selftest` and `--audiotest` under Wine with Mesa's software OpenGL.

Exit: `gta2.exe` (x64) plays the three districts with `renderer = classic` and `world3d` under Wine, and
the corpus hashes equal the Linux ones (same sim on every system, see `MULTIPLAYER.md`).

This also does the "Portability: Windows" row of the refactor plan's bar (`refactor/README.md`).

### Phase 7: Remix backend (3-5 days, 50% mechanical)

7.1 `remix/`: upstream's `src/renderer.{h,cpp}` (D3D9 fixed function, untransformed vertices, the form
Remix path traces) drawing `scene/`'s world, sprite and 2D passes. The D3D9 device on the SDL window's
`HWND` (`SDL_GetWindowWMInfo`), the only device in the process, so upstream's own-topmost-window and
`d3d9_remix.dll` rename are not needed.
7.2 `remix_api.cpp` and `remix_lights.cpp` taken over: game and invented lights as Remix sphere lights,
the sun through `rtx.atmosphere.sunElevation`/`sunRotation`, without the 50 ms throttle upstream needed
for the 32-bit bridge (keep it as a setting until measured).
7.3 Sprites as one draw each with the transform in `D3DTS_WORLD` (upstream's motion-vector fix), quads
keyed on `Sprite*` from hook 2.
7.4 Packaging: `rtx.conf` (with `rtx.zUp = False`), upstream's Remix mods (emissive lamps, water) built by
its `tools/make_water_mod.py` and `lamp_emissives.py`, a README for players: Windows, or Linux with
Proton/Wine.
7.5 `renderer = remix` offered only when `d3d9.dll` loads and reports Remix (`remixapi_InitializeLibrary`).

Exit: path-traced districts on the RTX 3060 under Wine (or Windows), lights and sun from the game, frame
generation without sprite ghosts; with Remix absent the option is hidden and nothing else changes.

### Phase 8: dev tuning panel (2-3 days, 60% mechanical, optional)

8.1 Dear ImGui 1.91.x vendored as `port/3rdParty/imgui` (the GTA2Hax copy is 1.50 and must stay: the VC6
build of `Source/ImGuiDebug.cpp` uses it). Backends: SDL2, OpenGL2, DX9.
8.2 Upstream's `debug_overlay.cpp` tabs that apply (Lights, Sprites, ToD, Camera), editing the same
settings table as `graphics.txt` plus a `world3d_tuning.txt` for the expert values. F4, the key upstream
uses (check it against the overlay's F1/F2/F5 and the radio keys).
8.3 Behind `GTA2_WORLD3D_DEVTOOLS`, off by default.

## 5. Testing, in one place

| Check | What it guards | When |
|---|---|---|
| corpus, option on vs off, `classic` | removability, nothing leaks into the game | every commit (CI) |
| corpus, `world3d`, game options off | 3D drawing never changes the game | every commit from phase 3 |
| corpus entries with widescreen/gibs | the game-changing options replay exactly | phase 5 on |
| `world3d/tests/` unit tests | mesh, ground, camera rectangle, light classification, effect thresholds, settings round trip | every commit |
| golden frames | the 3D picture | phase 3 on |
| `--selftest` with both renderers | GL setup, 2D pass | every commit |
| Windows build under Wine: selftest + corpus | phase 6 on | CI |
| play-test by the user | what tests can't see | end of phases 3, 4, 5, 7 |

## 6. Risks

| Risk | Likelihood | What then |
|---|---|---|
| Remix doesn't run under Wine on a hybrid-GPU laptop | medium | phases 6-7 need a Windows machine; 1-5 unaffected. Found out in phase 0, before any port work |
| Windows build: stand-in headers clash with MinGW's | medium | the SDL side is already separate; worst case a symbol prefix for the emulated Win32 API (mechanical, ~80 functions) |
| Upstream keeps moving (last commit 11 days before this plan) | high | provenance per file in `UPSTREAM.md`, kept names in `keep` files, re-run `survey.py`; merge upstream fixes by diff, not by re-import |
| Draw order: the game paints back to front, the 3D pass depth-tests | medium | upstream solved it for sprites (lift, conform); transparent effects keep the game's order inside the sprite pass |
| `widescreen_spawning` changes how dense traffic feels | low | off by default; replays record it |
| Refactor phase 2 renames `game/` (addresses out of names) | certain | hooks are 8 tagged call sites; the rename tool renames through them. Do phase 1 before or after the rename, not during |
| Gibs use the random number generator | medium | recorded as a game option, so replays still match; network games force it off |
| Performance on ARM handhelds | low | world3d is opt-in; classic stays the default |

## 7. Decisions for you

1. **Order.** Native GL renderer first (phases 1-5), Remix after (6-7)? Recommended: yes. The scene layer
   is shared, the GL version can be tested headless here, and phase 0 tells whether Remix works on this
   machine before the Windows build is started.
2. **Windows build route.** MinGW-w64 cross-compiled on Linux, 64-bit only (no bridge), tested under
   Wine? Recommended: yes. Alternative: MSVC on a Windows machine.
3. **Default renderer.** `classic` stays the default until you have play-tested `world3d`? Recommended: yes.
4. **Game-changing options** (widescreen spawning, blood, gibs): in, off by default, forced off in network
   games? Or leave gibs out (new behaviour the original never had)?
5. **Camera.** Straight down like the original (upstream's default), or offer a tilt? A tilt shows building
   sides the game never draws textures for well; recommended: straight down, tilt only in the dev panel.
6. **Vendoring.** Copy and adapt upstream's files into `port/world3d/` with provenance (recommended:
   the address reads have to change anyway), rather than a submodule plus patches.
7. **Dev panel** (phase 8): wanted at all?
