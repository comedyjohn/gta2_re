# Phase 6: Architecture

**Goal:** the game is a set of libraries with one-way dependencies; the simulation owns its state in a
`World` object instead of 1,684 globals and runs without any presentation or platform code linked; the
game talks to the machine through a few small interfaces instead of an emulated Win32/DirectX, so the
port builds natively on Windows and macOS too.

**Depends on:** phases 4 and 5. **Blocks:** phase 7.

**Exit criteria**

- [ ] CMake targets `gta2_core`, `gta2_data`, `gta2_sim`, `gta2_present`, `gta2_platform`, `gta2_app`
      with only downward dependencies (6.1); a cycle is a link error.
- [ ] `gta2_sim_replay` (core + data + sim, nothing else) runs the whole replay corpus with the same
      hashes as the full game.
- [ ] Fewer than 20 mutable globals, each documented in `port/ARCHITECTURE.md`
      (`cppcoreguidelines-avoid-non-const-global-variables`).
- [ ] `port/include/` (the fake `windows.h`, `ddraw.h`, `dinput.h`, `mmsystem.h`, ...),
      `port_prelude.h`, `platform/win32.cpp`, the DirectPlay and DirectInput emulation and the GTA2Hax
      headers are gone. No Win32 type or call in `game/`.
- [ ] CI builds and tests on Linux (GCC, Clang), Windows (MSVC `/W4 /permissive- /WX`) and macOS
      (Apple Clang), x86-64 and ARM64.
- [ ] 0 `misc-non-private-member-variables-in-classes`, `cert-err58-cpp` findings.

## 6.1 Target layers

```
gta2_app        start-up, main loop, app states (front end, game, credits), network game flow
   │
gta2_present    renderer front end, HUD, front-end menus, fonts, sound logic, movies
   │     ╲
gta2_sim     │  map, sprites, objects, vehicles, peds, services, script, players, game rules, replays
   │         │
gta2_data    │  file formats (phase 3), settings model
   │         │
gta2_core ───┘  types, fixed point, angles, RNG, pools, handles, strong ids, logging, asserts
   ┊
gta2_platform   window, events, OpenGL, audio device, input devices, sockets, file system, clock
                (implements interfaces declared in core/present/app; only app wires it in)
```

| Library | Subsystems (`subsystems.py`) | May depend on |
|---|---|---|
| `gta2_core` | core | nothing |
| `gta2_data` | data | core |
| `gta2_sim` | world, sprites, objects, vehicles, peds, services, script, game | core, data |
| `gta2_present` | presentation, audio | core, data, sim (read-only) |
| `gta2_platform` | `port/platform/` | core (interfaces), SDL, OS |
| `gta2_app` | app, network, frontend flow | everything |

Namespaces follow: `gta2::core`, `gta2::data`, `gta2::sim`, `gta2::present`, `gta2::platform`,
`gta2::app`.

## 6.2 Dependencies to break

Measured from `#include`s (excluding the scaffolding phase 1 deletes): 107 edges point from the
simulation up into presentation, audio, network or app, plus a few from core and data.

| Edge | Count | Why it exists | Fix |
|---|---|---|---|
| everything → `error.hpp` (app) | ~45 | `FatalError`, the error codes | Move `FatalError`, `Gta2Error` and logging to `core` |
| sim → `winmain.hpp` (app) | ~10 | globals defined there (window, flags, network state) | Those move to `World`, `DebugSettings` or app (6.3) |
| peds, vehicles, script, objects, world → `Hud.hpp` | ~12 | pager messages, score and bonus text, arrows | A `sim::Events` queue (`ShowMessage`, `AddArrow`, `ScorePopup`); the HUD consumes it and reads state |
| objects, peds, vehicles, sprites, script, world, game → `root_sound.hpp`, `SoundObject_10.hpp`, `CollisionSoundQueue_C88.hpp`, `voice_line.hpp` | ~25 | sounds started from game code; `Sprite` owns a `SoundObject_10*` | Events (`PlaySound`, `Collision`, `VoiceLine`) plus the audio side keeping its own sound objects keyed by handle |
| sprites, peds, vehicles, world, data → `Frontend.hpp` | ~8 | globals that live in the front end (selected stage, settings, text) | Into `GameSession`/settings passed in |
| sprites, world → `SpriteRenderer_1C.hpp`, `Draw.hpp`, `gbh_graphics.hpp` | ~4 | drawing helpers called from game code | Drawing moves to present; the sim exposes what to draw |
| game → presentation (`Game_0x40.cpp` → `Frontend`, `FpsCounter`, `ExplodingScore`, `MapRenderer`, ...) | 25 | `Game_0x40` creates and drives everything, including the renderer and the HUD | `Game_0x40` splits into `World` (sim) and the app's game state, which owns the presentation objects |
| `Player.cpp` → `NetPlay.hpp` | 1 | network input | Inputs come in as data (`PlayerInput` per frame) |
| core → sprites, world, game, presentation (`Fix16_Rect.cpp`, `rng.cpp`) | 5 | geometry helpers that know sprites; RNG that shows debug output | Move those helpers into sim; the RNG's debug display moves to present |

- [ ] `port/tools/refactor/deps.py`: this table, generated from includes, with a `--check` mode for CI
      that fails on any new upward edge. It runs from the start of the phase, so the count only goes down.

## 6.3 Globals into `World`

1,684 global definitions today:

| Kind | Count | Destination |
|---|---|---|
| Constants (`k...`) | 490 | `constexpr` in their module (phase 3.9) |
| Raw data (`byte_`, `dword_`, ...) | 450 | Phase 2 named them, made locals of the single-function ones; the rest go to the owning class below |
| Manager and pool pointers (`gPedManager_6787BC`, `gCar_BC_Pool_67792C`, ...) and objects (`g...`) | 474 | Members of `World` (simulation), `Presentation` (renderer, HUD, frontend, sound) or `App` (window, settings, network) |
| Debug flags (`b...`) | 91 | `DebugSettings`, loaded once, passed by `const&` |
| Other (function pointers into the renderer, scratch state, names) | 179 | Case by case: renderer function pointers go with 6.4; scratch state becomes locals or members |

`Game_0x40::Game_0x40` (`Game_0x40.cpp`) creates the world today, in this order: players, RNG, text,
style, map, map renderer, sprite renderer, Mike, car manager, car thread triggers, script manager,
script stack frames, FPS counter, sprite manager, ped manager, collision counters, object definitions,
objects, ped reference table, tile animations, weapons, doors, ambulances, public transport, taxis, HUD,
texture cache, traffic lights, route finder, patrol routes, path finder, car info, particles, the two
explosion pools, cranes, crushers, generators, emergency crews, police, light grid, gangs, garages, car
chase tasks, exploding scores, crime reports, fire engines, collision sounds, Kanji font (about 50
objects, each in a global pointer).

```cpp
// sim/world.hpp
class World
{
  public:
    explicit World(const WorldSetup& setup);  // map, style and script names, players, settings
    void Step(const FrameInputs& inputs);     // today's UpdateGame_4B9410, in the same order
    ...
  private:
    Rng rng_;
    Map map_;
    ObjectPool<Ped, 200> peds_;
    ObjectPool<Car, 306> cars_;
    ...                                       // declared in today's creation order, so construction
                                              // and destruction order stay the same
};
```

Steps, each hash-neutral:

- [ ] **6.3.1** Create `World` with the managers as members (`std::unique_ptr`, created in the same order
      in its constructor). Keep one global `World* gWorld`, and replace each global by a reference through
      it: `gPedManager_6787BC->X()` becomes `gWorld->peds().X()`. The old global names are unique tokens
      (phase 2 kept their uniqueness), so `rename.py` handles it.
- [ ] **6.3.2** Same for `Presentation` and `App`.
- [ ] **6.3.3** Subsystem by subsystem, pass what a function needs instead of reaching through `gWorld`:
      a constructor parameter (`Police(World&)`), or a function parameter for free functions. Start at the
      leaves (map, sprites) and work up (the game loop last).
- [ ] **6.3.4** Debug flags into `DebugSettings`.
- [ ] **6.3.5** `gWorld` itself goes; `App` owns the `World` of the current game.
- [ ] **6.3.6** The "current object" globals the decompiler made visible (`gCurrCarAI_Velocity_677B00`,
      `byte_677BBC`, `byte_677A5D` in `CarAI_78`, `gCollisionTarget_679188` in physics) become members
      of the class or parameters; they are state passed between functions of one update, not world state.

With the world an object, a second `World` can run beside the first: that is what makes tests,
replays in the background and a server without a window possible.

## 6.4 Platform interfaces instead of Win32

What the game uses today, and what replaces it:

| Today (uses / files) | Interface | Implementation |
|---|---|---|
| Renderer DLL functions through `GetProcAddress`: `pgbh_*` (218 / 12), `gbh_*` (56 / 15), `Vid_*` (25 / 2) | `present::Renderer` (draw quad, triangle, sprite batch, texture upload, begin/end frame, set mode) | `platform/renderer.cpp`'s OpenGL code, called directly |
| Miles `AIL_*` (93 in `cSampleManager.cpp`) | `present::AudioDevice` (load sample, play 2D/3D voice, stream, volume, listener) | `platform/mss.cpp`'s mixer, then OpenAL or SDL3 audio |
| DirectInput (110 / 5) | `app::InputDevices` (keyboard state, gamepads, text input for menus) | SDL events and gamepad API |
| DirectPlay (62 in `NetPlay.cpp`) | `app::NetSession` (host, join, send, receive, players) | `platform/net/` directly; `dplay.cpp` and `lobby.cpp` go |
| Registry `Reg*` (75 / 5) | `data::Settings` (typed key/value) | a settings file (today's `registry.txt`, read once) |
| Window and messages: `CreateWindow`, `PeekMessage`, `WndProc`, ... (50 / 5) | `app::Window` plus an event loop in `App` | SDL window and events |
| Lobby dialog: `GetDlgItem`, `SendDlgItemMessage`, ... (95 in `Network_20324.cpp`) | none: the command line lobby (`--host`, `--join`) and later an ImGui lobby | |
| `MessageBox` (12 / 5) | `app::ShowError(text)` | SDL message box |
| `CreateFile`, `FindFirstFile`, `_access`, `GetFileAttributes` (24 / 7), `\\` paths | `data::FileSystem` (open, read whole file, list, exists; case-insensitive lookup in the game directory) | `std::filesystem` |
| `timeGetTime`, `GetTickCount` (`Frontend`, `FpsCounter`, `Mike_A80`, `Network_20324`, `winmain`) | `app::Clock` (monotonic milliseconds) | `std::chrono::steady_clock`; the simulation never reads it |
| Bink (37 in `Bink.cpp`) | `present::MoviePlayer` | stub now; FFmpeg's Bink decoder later (`TODO.md`) |

- [ ] One interface at a time, in the order of the table. After each, delete the matching emulation
      in `port/platform/` and the fake headers in `port/include/` it needed.
- [ ] Interfaces are small abstract classes or concepts in the layer that uses them; `gta2_platform`
      implements them; `gta2_app` wires the implementation in. Tests use fakes (an in-memory file system,
      a recording audio device, a scripted input device).
- [ ] When `port/include/` is empty: remove `-include port_prelude.h`, `_WIN32` and friends, the
      `-fshort-wchar` leftovers, the GTA2Hax include path; move to SDL3 behind the interfaces.
- [ ] Add the Windows (MSVC) and macOS jobs to CI.

## 6.5 The app and its states

`winmain.cpp` (2,478 lines) holds the start-up (`WinMain_5E53F0`, 285 lines), the window procedure, the
main loop (`RunGameFrame_4DA850`), the network game flow, and a state machine spread over globals
(`bDoFrontEnd_626B68`, `bStartNetworkGame_7081F0`, the exit types of `Game_0x40`).

- [ ] `App` with explicit states: `Boot`, `FrontEnd`, `Loading`, `InGame`, `Credits`, `Exit`;
      transitions are function calls, the `GameExitType` handling of today's main loop becomes the
      `InGame` exit transition.
- [ ] The fixed-step loop as a function of the clock: run `World::Step` while simulated time is behind
      wall time (at most 8 steps per draw, as today), then draw. Headless (`skip_draw`) is the same loop
      with drawing off and no waiting.
- [ ] `--replay`, `--selftest`, `--audiotest`, `--simcheck`, `--nettest` become app modes.

## 6.6 Encapsulation

`misc-non-private-member-variables-in-classes`: 2,429 findings; every member is public today.

- [ ] Per class, after its globals are gone: members private; a small public interface named after what
      callers actually do (from `callgraph.py`'s list of outside uses of each member); plain structs
      with no invariants (`InputRecord`, file value structs, events) stay public structs.
- [ ] Per subsystem, a public header (`sim/peds/peds.hpp`) and internal headers (`sim/peds/detail/...`);
      other subsystems include only the public one (checked by `deps.py`).

## Risks

| Risk | Mitigation |
|---|---|
| Construction or destruction order changes when globals become members | Members declared in today's creation order; replays include a full game start and quit |
| Static initialisation order of the remaining globals | `cert-err58-cpp`, constant initialisation (`constinit`) for what stays |
| The platform rewrite breaks a player-visible feature (resolution change, focus handling, gamepads) | Golden frames, `--selftest`, `--audiotest`, a manual play-test checklist per interface |
| Moving sounds and HUD to events changes timing (an event handled one frame later) | Events are consumed in the same frame, in the order they were raised; golden frames and the audio log (phase 0.11) check it |
| The work is large and long | One interface or one subsystem per batch; the dependency check keeps finished parts from regressing |
