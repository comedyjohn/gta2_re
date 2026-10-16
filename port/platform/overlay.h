// The controller overlay: a legend of what each key and pad button does (F1), a settings screen (F2,
// or the pad's Guide / Home button) that rebinds the game's twelve controls to keys and pad inputs, a
// car spawn menu (F5 or Back), and a pause menu (Esc or Start: Settings, Quit game) that stands in for
// the game's own quit question unless switched off. It draws into a 640x480 picture that the renderer
// lays over the game's. In a single player game, the game stands still while any of them is open.
//
// Each control has a key and a pad input, and both work. The key is the game's own binding (the
// registry's control settings, which the game's options menu edits too); the pad input comes from the
// pad's layouts for walking and for driving (platform/padinput.h, saved in pad.txt).
#pragma once
#include "platform.h"

#include <stdint.h>
#include <string>

namespace overlay
{
const int kActions = 12;

// A key press (DirectInput scan code) before the game sees it. True when the overlay took it.
// Releases always go on to the game, so a key held while the overlay opens cannot stay pressed.
bool OnKey(uint8_t dik, bool down);

// True while a menu of the overlay is open: the game gets neither keyboard nor pad input
bool Captured();

// True while a menu is open in a single player game: the game stands still and its sound pauses
bool FreezesGame();

// Once per presented frame. Returns the picture to draw over the game (ARGB, 640x480, transparent
// where nothing is drawn), or null when there is nothing to show.
const uint32_t* Frame();

// What the controls are called, in the game's order (mask indices 0..11 of InputRecorder_67F8B0)
const char* ActionName(int action);
// "Ctrl", "Space", "F5"; "Key 0x7F" for a code without a name
std::string KeyName(int dik);
// A binding as the player reads it: device 0 is a key, 1 a pad source (padinput.h)
std::string BindingLabel(int device, int binding);
// The pad input that went down between `prev` and `now`, as a pad source: a button number, or
// 224..227 for left, right, up, down on the stick or d-pad. False when nothing did.
bool CapturePadBinding(const plat::PadState& prev, const plat::PadState& now, int& binding);
} // namespace overlay
