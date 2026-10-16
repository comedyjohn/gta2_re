// What the game tells the replay runner (gta2 --replay, port/platform/replay.cpp). Port-only test
// code: the calls do nothing unless the game was started with --replay.
#pragma once

// RunGameFrame_4DA850 (winmain.cpp) calls these around every simulation update of a single-player game
void port_replay_before_update();
void port_replay_after_update();
