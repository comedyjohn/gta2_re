# Phase 4: Control flow and function size

**Goal:** functions short enough to read in one screen and test in isolation, state machines that look
like state machines, no `goto`, no dead code, and files that hold one topic.

Unlike phase 2, almost nothing here is mechanical. Every change can alter behaviour in a subtle way
(evaluation order, an early return that skipped a side effect, a shared tail block), so each one leans
on phase 0's replays, and before that on knowing that the replays actually run the code being changed.

**Depends on:** phase 0, phase 2. **Blocks:** phase 6.

**Exit criteria**

- [ ] `readability-function-size` (`LineThreshold: 150`) and `readability-function-cognitive-complexity`
      (`Threshold: 40`) pass, except for a list of justified exceptions in `port/.clang-tidy`
      (`NOLINT` with a reason), which should be under 20.
- [ ] No file over 2,000 lines (`metrics.py --files`).
- [ ] 0 `goto` (`cppcoreguidelines-avoid-goto`).
- [ ] 0 findings for `bugprone-branch-clone`, `bugprone-assignment-in-if-condition`,
      `readability-simplify-boolean-expr`, `readability-else-after-return`, `misc-unused-parameters`;
      `-Wno-unused-parameter` removed.
- [ ] Every `TODO`/`FIXME` is resolved or is a tracked issue referenced from the comment.
- [ ] Every function changed in this phase was covered by the replay corpus or by a unit test when it
      changed (4.1).

## 4.1 Coverage first

A refactor is only as safe as the tests that run the code. Before restructuring a function, know whether
the corpus reaches it.

- [ ] Clang build with `-fprofile-instr-generate -fcoverage-mapping` (preset `coverage64`).
- [ ] `port/tools/refactor/coverage.sh`: runs the whole corpus, merges the profiles (`llvm-profdata
      merge`), and writes per-function line and branch coverage (`llvm-cov report
      --show-functions`) to `coverage.csv`, plus an HTML report.
- [ ] Rule: a function is restructured only if the corpus covers at least 80% of its lines and all of
      its branches that the change touches. Otherwise: record a replay that reaches it, or write a unit
      test that drives it, first. The coverage report says which branches are missing.
- [ ] The script VM gets its own report: which of the 449 opcodes (`SCRCMD`) the corpus executes. The
      ones it doesn't are covered by hand-assembled scripts in `test_script_vm.cpp`.

## 4.2 Patterns and what replaces them

| Pattern in the decompiled code | Replacement |
|---|---|
| `goto ret_true;` / `goto ret_false;` to a label that only returns | `return true;` / `return false;` |
| `goto LABEL_27;` to a shared tail block several branches reach | Extract the tail into a function (`return FinishTurn(...)`), or restructure so the branches fall through to it |
| `goto` backwards to re-run a loop body | `continue`, or a `while`/`for` with the right condition |
| A flag set in one branch and tested much later in the same function | Split the function at that point; the flag becomes a return value |
| A long `if`/`else if` chain on one variable | `switch` |
| The same block repeated in several branches (`bugprone-branch-clone`, 70) | One block; merged case labels or a combined condition |
| `if ((x = f()) != 0)` (`bugprone-assignment-in-if-condition`, 57) | `x = f(); if (x != 0)` or `if (auto x = f(); x != 0)` |
| A local computed once and written to a global "for later" (decompiled scratch globals) | Return value or parameter (globals themselves: phase 6) |
| Reading a member through `this->` in one place and directly in another | One style (no `this->`) |

Evaluation order: C++ leaves the order of function arguments unspecified, so GCC, Clang and MSVC may
each pick a different one, and none has to match the original's. When two calls that draw random
numbers or change state are arguments of the same call, the port's result already depends on the
compiler (a lockstep hazard between builds), and splitting them into locals fixes one order. Pick the
order the replays recorded with the reference build, and do it consciously, never as a side effect of
tidying. `port/tools/refactor/callgraph.py` (4.7) can list calls with two or more state-changing
arguments.

## 4.3 State machines

Several large functions are state machines written as one big `switch` with fall-through, shared code
between states and flags: `CarAI_78::UpdateStateMachine_44E560` (1,114 lines),
`Char_B4::UpdateAnimState_546360` (964) with `state_0_54DDF0` (483), `state_1_5504F0` (408),
`state_8_5520A0` (470), `PoliceCrew_38::State5_PursueOrChase_572920` (400),
`Ambulance_20::HandleObjectiveState_4FAAC0` (358), `EmergencyCrew_30::UpdateStateMachine_5CBD50` (274),
`Ped::AttackTargetStateMachine_46D460` (308).

Target shape:

```cpp
enum class CarAiState : u8 { kIdle, kCruise, kFollowRoute, kChase, kAvoid, ... };

void CarAi::Update()
{
    switch (state_)
    {
        case CarAiState::kIdle: UpdateIdle(); break;
        case CarAiState::kCruise: UpdateCruise(); break;
        ...
    }
}

void CarAi::UpdateCruise()
{
    ...
    if (BlockedAhead())
    {
        EnterState(CarAiState::kAvoid);
        return;
    }
    ...
}
```

- [ ] One method per state, named after what the state does.
- [ ] State changes go through `EnterState()`, which is where entry actions live (today they are
      copied into every place that changes the state).
- [ ] Code shared by several states becomes named helpers.
- [ ] The order of side effects inside a frame stays exactly as it was (the replays prove it).

## 4.4 The script VM

`miss2_0x11C::ExecOpCode_5061C0` and `PreExecOpCode_5108D0` (733 lines) dispatch 449 opcodes through
573 `case SCRCMD_...` labels; the opcode implementations are about 250 methods of `miss2_0x11C`
(7,869 lines in one file).

- [ ] A dispatch table: `constexpr std::array<OpcodeHandler, kOpcodeCount>`, built from one list of
      `{opcode, handler, name}`, replacing the `switch`. Unknown opcodes hit one `FatalError`.
- [ ] Handlers grouped by topic into files: `script/ops_flow.cpp` (goto, gosub, if, wait), `ops_cars.cpp`,
      `ops_peds.cpp`, `ops_objects.cpp`, `ops_player.cpp`, `ops_hud.cpp`, `ops_mission.cpp`,
      `ops_world.cpp`, ...
- [ ] Each handler takes a typed view of its command (phase 3.5) instead of reading the union.
- [ ] A script disassembler (`gta2 --dump-script data/xxx.scr`), useful for writing the opcode tests
      and for debugging missions.

## 4.5 The other giant functions

Over 250 lines (48 functions); each gets its own plan when its turn comes. The approach column is the
starting point.

| Function | Lines | Approach |
|---|---|---|
| `ImGuiDebugDraw` (`ImGuiDebug.cpp`) | 2,274 | One function per debug window; the overlay is a developer tool, so it can also be cut down |
| `FatalError_4A07C0` (`error.cpp`) | 1,532 | A table of messages (phase 3.8) |
| `CarAI_78::UpdateStateMachine_44E560` | 1,114 | State machine (4.3) |
| `Char_B4::UpdateAnimState_546360` | 964 | State machine (4.3) |
| `miss2_0x11C::PreExecOpCode_5108D0` | 733 | Dispatch table (4.4) |
| `Map_0x370::CanMoveOntoSlopeTile_4E0130` | 655 | A table of slope types and a few geometric helpers (the slope cases repeat with mirrored signs) |
| `PoliceRoadblock_A4::CreateRoadblock_575FF0` | 623 | Split by orientation into helpers; layouts as data |
| `sound_obj::ProcessObject_Type12_41E850` | 610 | One function per sound object kind |
| `Frontend::SetupMenuStringsOptionsElements_4B0220` | 572 | Menu definitions as data (tables of pages and elements), one builder |
| `CarAI_78::ManageTrafficCarDirection_448CE0` | 571 | Per-direction helpers; the four directions are mirrored copies |
| `Char_B4::state_0_54DDF0`, `state_1_5504F0`, `state_8_5520A0` | 483, 408, 470 | Part of the `Char_B4` state machine |
| `TrafficSpawner_14::SpawnTrafficCar_582480` | 475 | Find spawn point / choose model / create car / set up AI |
| `Frontend::DrawMenu_4AD140` | 421 | One function per element type |
| `Ped::GotoAreaByAnyMeans_469060` | 412 | Steps: choose means (walk, car, ...), path, follow |
| `PoliceCrew_38::State5_PursueOrChase_572920` | 400 | State machine |
| `CarAI_78::AlignToLaneCenter_44AF00`, `FollowRoadDirection_44A1F0`, `DetectCarAhead_44D1D0`, `ScanAheadForObstacles_452060` | 386, 384, 378, 320 | Per-direction helpers, shared geometry |
| `Char_B4::CanStepDiagonal_54EF60` | 382 | Per-diagonal helpers |
| `PoliceRoadblock_A4::Update_5757B0` | 365 | State machine |
| `Ambulance_20::HandleObjectiveState_4FAAC0` | 358 | State machine |
| `Char_B4::HandleGenericCollision_54A530` | 357 | One function per collider kind (car, object, ped) |
| `Ped::Threat_Reaction_AI_465270` | 347 | Decision (pure function returning a reaction) and action |
| `Police_7B8::UpdatePursuitTargets_56FBD0` | 344 | Per-target update plus crew assignment |
| `Game_0x40::Game_0x40` | 332 | Becomes `World`'s construction in phase 6 |
| `Frontend::ChangeMenuPage_4B3170`, `UpdatePageFromUserInput_4AE2D0` | 331, 268 | Menu data plus small handlers |
| `PlayerScoreTracker_36C::AwardPedKilledScore_592660` | 329 | Score rules as a table |
| `PublicTransport_181C::PublicTransportService_57A7A0` | 312 | Trains, buses, stations: one function each |
| `SpawnPedestrianAt_46E380` | 309 | Choose type / create / set up |
| `Ped::IsPedAThreat_465D00` | 296 | Merged with its inline copy in phase 1.3; then split into rules |
| `ObjectDefinitions_8CA8::CloneAnimatedDefinitions_533420` | 294 | Data-driven |
| `WinMain_5E53F0` | 285 | Becomes the app's start-up sequence (phase 6) |
| `Player::Hud_Controls_565890` | 282 | One handler per control |
| `sound_obj::ProcessActiveQueues_41AB80` | 279 | Per queue |
| `Network_20324::OnWmCommand_519FE0` | 272 | Goes with the Win32 lobby dialog (phase 6) |
| `Ped::UpdateMovementTowardsTarget_4672E0` | 271 | Movement helpers |

The full list (functions over 60 lines: 439) comes from `metrics.py --functions` (to add: the same
scan as the table above, written to `functions.csv` with lines and cognitive complexity).

## 4.6 `goto`

224 in total: `Ped.cpp` 147, `CarAI_78.cpp` 27, `PathFinder_2FD4.cpp` 21, `char.cpp` 18,
`Police_38.cpp` 15, `sound_obj.cpp` 11, the rest scattered. Most are `ret_true`/`ret_false` labels
(4.2's first row) and `LABEL_nn` tails left by the decompiler.

- [ ] Per file, one commit per function.

## 4.7 Dead code and debug code

- [ ] `port/tools/refactor/callgraph.py` (libclang): functions with no callers that are not
      address-taken, members never read, globals never read. Delete them, or document why they stay.
- [ ] Debug settings (`debug.cpp`, 76 settings read from the registry): keep the ones that are useful
      for development and testing (`skip_*`, `do_exit_after_replay`, `play_replay`, ...) and collect them
      into a `DebugSettings` struct (phase 6); remove the ones tied to the original developers' private
      tests (`do_brian_test`, `do_iain_test`, `do_mike`/`Mike_A80`) once checked to do nothing useful.
- [ ] `TODO`/`FIXME` (253): triage each into done, issue, or delete.
- [ ] Parameters that were kept because the original signature had them (`-Wunused-parameter`, 71;
      `misc-unused-parameters`, 34): remove them.

## 4.8 Split the large files

Once functions are small, split files along topic lines (one commit per split, `git mv` plus moves, no
other edits):

| File | Lines | Split into (proposal) |
|---|---|---|
| `Ped.cpp` | 12,128 | `ped.cpp` (lifecycle, state), `ped_movement.cpp`, `ped_combat.cpp`, `ped_threat.cpp`, `ped_car.cpp` (entering and leaving cars), `ped_objectives.cpp`, `ped_death.cpp` |
| `Car_BC.cpp` | 7,973 | `car.cpp`, `car_damage.cpp`, `car_doors.cpp`, `car_weapons.cpp`, `traffic_spawner.cpp`, `trailer.cpp` |
| `miss2_0x11C.cpp` | 7,869 | `script_thread.cpp` plus the opcode files of 4.4 |
| `sound_obj.cpp` | 7,703 | `sound_manager.cpp`, one file per sound object kind, `sound_queues.cpp` |
| `char.cpp` | 6,885 | `ped_body.cpp`, `ped_body_states.cpp`, `ped_body_collision.cpp`, `ped_body_animation.cpp` |
| `CarAI_78.cpp` | 5,412 | `car_ai.cpp`, `car_ai_states.cpp`, `car_ai_lanes.cpp`, `car_ai_obstacles.cpp` |
| `Frontend.cpp` | 5,346 | `frontend.cpp`, `menu_data.cpp`, `menu_draw.cpp`, `menu_input.cpp`, `save_select.cpp` |
| `map_0x370.cpp` | 4,858 | `map.cpp`, `map_slopes.cpp`, `map_queries.cpp`, `map_zones.cpp` |
| `CarPhysics_B0.cpp` | 3,952 | `car_physics.cpp`, `car_collision.cpp`, `car_skids.cpp` |
| `Object_5C.cpp`, `MapRenderer.cpp`, `ImGuiDebug.cpp`, `Hud.cpp`, `Police_38.cpp`, `Player.cpp` | 3,000 to 3,800 | by topic when their turn comes |

## Risks

| Risk | Mitigation |
|---|---|
| A restructured function changes order of side effects | Replays; coverage rule (4.1) so the replays reach it |
| An early return skips code that ran before | Replays; review checklist item "every path still reaches the same calls in the same order" |
| Argument evaluation order changes when splitting expressions | 4.2's rule; replays |
| Code paths the corpus never reaches get broken | Coverage rule; unit tests for those paths; extend the corpus |
| Deleted "dead" code was reached through a function pointer or the script | `callgraph.py` treats address-taken functions as used; script opcodes are covered by the opcode report |
