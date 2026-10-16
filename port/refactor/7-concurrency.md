# Phase 7: Concurrency and performance

**Goal:** use more than one core where it helps the player (smooth drawing, no hitches when loading or
changing the radio station) while the simulation stays single-threaded and deterministic, as lockstep
network play and replays require.

**Depends on:** phase 6 (the simulation is a separate `World`; presentation reads it, never the other
way round). **Blocks:** nothing.

**Exit criteria**

- [ ] The simulation, rendering, audio mixing, asset loading and networking run on their own threads,
      connected only through the channels in 7.2.
- [ ] ThreadSanitizer clean on the corpus (preset `tsan64`), and Clang's `-Wthread-safety` clean.
- [ ] The replay hashes are unchanged (threads must not change what the simulation computes).
- [ ] The benchmark (7.5) is no worse than before the phase, and frame-time spikes on level load and
      radio changes are gone.

## 7.1 Threads today

- Main thread: the window loop, the simulation, drawing, sound logic.
- SDL's audio callback thread: mixing (`platform/mss.cpp` through `sdl_platform.cpp`'s `OpenAudio`).
- `platform/net`: `Client` and `Server` each run a `std::thread` for their sockets.
- Loading is synchronous: style, map and script load on the main thread when a level starts; radio
  streams are decoded whole when opened (`SDL_LoadWAV`), which is the radio hitch in `TODO.md`.

## 7.2 Target

```
             FrameInputs                         FrameSnapshot (triple buffer)
input ─────────────────────►  simulation  ─────────────────────────────►  render thread ──► GPU
(main thread: window,          (World::Step,       SoundCommands (SPSC queue)
 events, app states)            fixed 33 ms)  ─────────────────────────►  audio thread ──► device
                                    ▲
               net thread ──────────┘  (inputs of other players, lockstep)
               loader thread ──► finished assets handed over between steps
```

- [ ] **Simulation thread.** Runs `World::Step` at the fixed rate, from inputs collected by the main
      thread. Owns `World` exclusively.
- [ ] **`FrameSnapshot`.** After each step, the simulation writes what presentation needs into a
      snapshot: visible sprites (position, rotation, sprite and remap index, z, flags), lights, changed
      map tiles and animations, cameras, HUD state (score, wanted level, messages, arrows), and the
      frame's events (phase 6.2). A triple buffer: the simulation writes one, the renderer reads the
      newest complete one, neither waits.
- [ ] **Render thread.** Owns the OpenGL context; draws the newest snapshot; never touches `World`.
      Optional, after the base: interpolate between the two newest snapshots so drawing runs at the
      display's refresh rate while the simulation stays at 30 steps a second (presentation only; the
      hashes don't change).
- [ ] **Audio.** Sound logic (which sound plays where, today's `sound_obj`) runs on the simulation
      thread from the frame's events and state, and sends commands through a lock-free single-producer
      single-consumer queue to the mixer; streams decode in chunks on the audio side.
- [ ] **Loader thread.** Reads and parses files (`gta2_data` readers) in the background: the next
      level's style and map while the front end shows, radio stream chunks ahead of playback.
- [ ] **Network.** `platform/net`'s threads hand received inputs to the simulation through a queue;
      the lockstep wait stays where it is (the simulation waits for every player's inputs of a frame).

Parallelism inside the simulation (path finding, collision broad phase) is out of scope unless the
benchmark shows the simulation itself is too slow; if it ever comes, results must be merged in a fixed
order so the hashes don't change.

## 7.3 Rules

Written into `STYLE.md`:

- Threads are `std::jthread` with `std::stop_token`; nothing detaches.
- Data crosses threads only through the channels above (snapshot buffer, SPSC queues), never by
  sharing an object.
- Mutexes only inside those channels, wrapped in annotated types (`GTA2_GUARDED_BY`, `GTA2_REQUIRES`)
  so Clang's `-Wthread-safety` checks them.
- `std::atomic` with explicit memory orders, commented.
- The simulation never reads the clock, never waits on another thread except for network inputs, and
  never sees a snapshot.

## 7.4 Steps

- [ ] `tsan64` preset and a CI job running unit tests and (self-hosted) the corpus under TSan.
- [ ] The snapshot and the renderer reading only the snapshot, still on one thread (proves the
      separation; golden frames unchanged).
- [ ] Move rendering to its thread.
- [ ] Sound commands queue; chunked stream decoding (fixes the radio hitch).
- [ ] Loader thread for level data and streams.
- [ ] Simulation on its own thread; the main thread keeps the window and input.
- [ ] Optional interpolation.

## 7.5 Benchmark

- [ ] `gta2 --replay <long replay> --benchmark` (headless): simulation time per step (mean, 99th
      percentile, max) and total; with drawing under Xvfb: frame time distribution.
- [ ] Run on CI (self-hosted, fixed machine) per commit; keep the history; fail on a regression over 10%
      in the mean or 25% in the 99th percentile.
- [ ] Optional: Tracy profiler zones behind a build option, for finding where time goes.

## Risks

| Risk | Mitigation |
|---|---|
| A data race | Channels only; TSan; `-Wthread-safety` |
| Non-determinism (the simulation reads something another thread changes) | The simulation owns `World`; replays run in CI with threads on |
| The snapshot misses something the renderer needs | Golden frames; the single-thread snapshot step comes first |
| Latency: input to screen gets a frame longer | Measure; the triple buffer always shows the newest step |
