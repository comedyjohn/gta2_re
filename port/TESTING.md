# Testing the native port

| Test | Needs game data | Checks | Run |
|---|---|---|---|
| Unit tests (`port/tests/`) | no | `Fix16`, angles and tables, `rand()`, pools, `BitSet32` | `ctest --test-dir build_port/linux64 --output-on-failure` |
| `gta2 --selftest` | no | the renderer, pixel by pixel | `xvfb-run -a build_port/linux64/gta2 --selftest` |
| `gta2 --audiotest` | no | the sound mixer | `build_port/linux64/gta2 --audiotest` |
| `tools/nettest.sh` | no | network games, also between 32 and 64-bit builds | `port/tools/nettest.sh build_port/linux32 build_port/linux64` |
| `tools/refactor/init_order.py` | no | no global is initialised from another file's global | `python3 port/tools/refactor/init_order.py` |
| **Replay tests** (`tools/replaytest.sh`) | yes | **what the game computes**: the simulation's state after every update | `port/tools/replaytest.sh build_port/linux32 build_port/linux64` |

CI runs everything that needs no game data, the unit tests also under AddressSanitizer and
UndefinedBehaviorSanitizer (presets `asan64`, `ubsan64`). The replay tests run locally.

## Replay tests

The game is deterministic: a replay is the player's input, keyed to the update it happened in,
so playing it back computes the same game again. `gta2 --replay` plays one headless (no window
content, no waiting for the clock: about 1,000 updates a second) and logs a hash of the
simulation after every update: one per component (random numbers, sprites, peds, cars, objects,
players, script, police) and their total. A refactor must leave every log exactly as it was. Every
build logs the same: GCC and Clang, 32 and 64 bits.

```bash
port/tools/replaytest.sh build_port/linux32 build_port/linux64 build_port/clang64
# PASS 11-attract-downtown (build_port/linux32)
# FAIL 11-attract-industrial (build_port/linux64): first difference at update 3892: sprites, cars
```

The corpus lives outside the repository, since it holds recordings of the game: `$GTA2_CORPUS`,
by default `~/Games/gta2/corpus` (`replays/NAME.rep`, `expected/NAME.log`, and `corpus.txt` with
each replay's length, SHA-256 and what it covers). The game folder is `$GTA2_GAME_DIR`, by default
`~/Games/gta2/game`.

### Recording a replay

```bash
build_port/linux32/gta2 --record session.rep --map wil --game-dir ~/Games/gta2/game
```

starts straight in Downtown (`ste` Residential, `bil` Industrial) with your settings and controls,
and writes the replay when you quit, or press Ctrl+C in the terminal. Then:

```bash
port/tools/replaytest.sh --add 02-cars session.rep "stealing and crashing cars in Downtown"
port/tools/replaytest.sh --update build_port/linux32
```

`--update` writes the expected log of every replay. It says when a replay doesn't play back the
way it was recorded (each input record holds the random number of its moment, and playback
compares them); a replay recorded with this port must not. `--record ... --hash-log live.log`
also logs the hashes while you play: the playback's log must equal it.

Use `--update` after adding a replay, and in a `Fix:` commit that changes what the game does
(port/refactor/README.md, rule 2); never to make a refactor pass.

### When a replay fails

1. The report names the first update that differs and the components. Dump the state there from a
   build that passes and one that fails, and diff:
   ```bash
   build_port/linux32/gta2 --replay R.rep --game-dir G --hash-every 1000000 --break-at 888 --hash-log good.txt
   build_port/clang64/gta2 --replay R.rep --game-dir G --hash-every 1000000 --break-at 888 --hash-log bad.txt
   diff good.txt bad.txt      # peds.ped[24].objective 35 / 20 ...
   ```
   The dump has every hashed value by name (`peds.ped[24].objective`, `police.crew[0].field_28`).
2. Between two compilers (GCC and Clang objects link together on x86-64), link the game with some
   objects from one build and the rest from the other, and bisect the files; then in the file, try
   `-O0`, `-ftrivial-auto-var-init=zero`, `__attribute__((optnone))` per function. That is how
   `kFpEight_6FED48` (a static initialiser reading another file's global) was found.
3. Before a refactor of a subsystem, check that the corpus reaches it: a deliberate one-character
   change in it must turn a replay red. Record a session that exercises it if not.

### What the hash covers

`port/platform/statehash.cpp`. Pointers are hashed as the pool and index they point at, never as
addresses, and structures field by field, so the hash is the same across runs and builds. The
mission script's data block is hashed whole, with the object pointers its commands keep (`PTR32`)
translated the same way. Not covered yet: collision shapes, weapons, gangs, the HUD, sound,
garages, cranes and other tier 2 state (port/refactor/0-safety-net.md, task 0.2), and drawing
(task 0.10).
