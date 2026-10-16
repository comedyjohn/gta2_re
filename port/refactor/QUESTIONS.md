## Q1: Valgrind finds 3 uninitialised reads on all 4 replays (task R0.3, 2026-10-16)
What I tried: `port/tools/refactor/valgrind.sh build_port/linux64` (replays finish in about 1 min each, not hours). Logs: `build_port/linux64/valgrind/*.vg`.
What I saw (the replay hashes are unaffected today, the reads are of heap/static memory that happens to be stable):
1. `PathFinder_2FD4::RemovePed_554620` (`PathFinder_2FD4.cpp:254`): `field_0_ped_id` is compared before anything sets it; `Init_5545E0` doesn't touch it.
2. `Door_38::CanOpen_49C6D0` (`Door_38.cpp:103`, 11-attract-downtown only): `field_10_car_bc`/the door type is read uninitialised.
3. `SetTexture` (`platform/renderer.cpp:484`): `pPixelData` from `load_tiles_5AADF0` (`gtx_0x106C.cpp:669`, `Aligned_malloc`) has bytes never written. Drawing only, no effect on the hash.
Options: A) `Fix:` commits that give the fields the value the original's allocator gave (zero) and rewrite the logs; B) record all three in `known-ub.md` as the original's.
My recommendation: A for 3 (zeroing the buffer, the original zeroed heap pages) and A for 1 and 2 if the user agrees that zero is right. I can't tell whether "zero" is what the original had (§0 rule 2), so I have not changed anything.
Status: answered: fix them (A). Done in the three Fix: commits; valgrind is clean on linux64.

## Q2: Should the game's own network message carry the state hash? (task R0.6, 2026-10-16)
What I tried: `gta2 --nettest` (no game data) now exchanges a per-frame simulation hash (a running hash of one `rand()` draw per frame) and reports `DESYNC at frame N`; `nettest.sh` checks that a skipped `rand()` is caught.
What I saw: the real game's per-frame message (`Net_Send_Our_Inputs_4DACB0`, `Network_InputData_0x8`) only has an rng value, and only when `bDo_sync_check_67D6C1` is on. Adding the `port_state_hash()` total every 30 frames changes the game's wire format (port to port only; the original's network play isn't supported by this port), and a real multi-process game test needs game data on every player.
Options: A) leave it: the harness covers the mechanism, the replays cover the simulation. B) add the hash to the game's message when `bDo_sync_check_67D6C1` is set, with a local two-process test (`gta2 --replay`-style, no UI) later.
My recommendation: A for now. Nothing in phases 1 to 4 changes the message, and B costs a game-wire change plus a test rig.
Status: open (does not block anything)

## Q3: Please record 8 sessions to cover what the corpus misses (task R0.8, 2026-10-16, for the user)
What I tried: `port/tools/refactor/coverage.sh`. The 4 replays cover 68.6% of the lines they link and 124 of 404 script opcodes. The biggest uncovered parts (missed lines, from `coverage.csv`): Ped.cpp 5,174 (of 10,506), the script VM miss2_0x11C.cpp 4,988 (of 6,426; 280 opcodes never run), Frontend.cpp 4,262 (menus: of 4,331), sound_obj.cpp 3,049, MapRenderer.cpp 3,029 (draw only), Car_BC.cpp 2,458, CarAI_78.cpp 1,732, Hud.cpp 1,688, Police_38.cpp 1,582, Player.cpp 1,492, char.cpp 1,364, Weapon_30.cpp 1,264, CarPhysics_B0.cpp 1,090, map_0x370.cpp 1,096.
Each session, 2 to 3 minutes, one command (it starts at once in the map, with your controls; quit with Esc or Ctrl+C):
`build_port/linux64/gta2 --record /ABS/PATH/NAME.rep --map MAP --game-dir ~/Games/gta2/game`
| NAME | MAP | What to do |
|---|---|---|
| 02-weapons | wil | Pick up and fire every weapon you can find or get from the spawn menu: pistol, machine gun, rocket launcher, flamethrower, molotov, shotgun, electro gun; kill peds and cops |
| 03-cars-crash | ste | Steal several different cars (a bus, a truck, a fast car), drive fast, crash into walls, other cars and peds, flip one, get it burning and blown up |
| 04-police | wil | Cause trouble until you have 3 or more wanted stars: run over peds, shoot a cop; get chased by police cars, helicopter and the army if it comes; get busted or killed once |
| 05-gangs-missions | bil | Walk to phone boxes and take a mission or two; fight the gang you meet; drive the mission car |
| 06-peds | wil | Walk through crowds; push, punch and kick peds, steal a car with peds around, jump in water, climb on a train or a bridge, get killed once |
| 07-vehicles-special | ste | Taxi, bus, fire truck, ambulance, tank if reachable; use their special actions (siren, taxi fare, water cannon) |
| 08-death-and-respawn | wil | Die three times in different ways (drowning, explosion, shot) and respawn; collect a power-up and a health pack |
| 09-frontend-menus | (needs `--record` through the menus; not possible yet) | tell me if you want menu recordings: today `--record` skips the front end |
When you have recorded them, tell me the folder. I run `replaytest.sh --add NAME FILE "note"` for each, then `--update` on linux32, check linux64 and clang64 twice, and re-run coverage.
My recommendation: record 02 to 08 (7 sessions, about 20 minutes); skip 09 until `--record` can start in the menu.
Status: open (blocks phase 4 for the functions in these files; phases 1 to 3 can start without it)
