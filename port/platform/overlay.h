// The controller overlay: a legend of what each button does (F1), and a settings screen (F2, or the
// pad's Guide / Home button, or Back + Start together) that rebinds the game's twelve controls to
// pad buttons or keys. It draws into a 640x480 picture that the renderer lays over the game's.
//
// The game keeps one binding per control, a key or a pad button (gMaybeDeviceType_67B91C and
// gPlayerControlsBinding_67B6E8, saved in the registry). The overlay edits those, so what it shows is
// what the game does, and what the game's own options menu sets shows up here.
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

// True while the settings screen is open: the game gets neither keyboard nor pad input
bool Captured();

// Once per presented frame. Returns the picture to draw over the game (ARGB, 640x480, transparent
// where nothing is drawn), or null when there is nothing to show.
const uint32_t* Frame();

// Called with the control bits of a live game frame (bit i is control i) before they are recorded:
// with "world directions" on and the player on foot, the stick or d-pad sets Forward, Left and Right
// so that the character turns to the direction the stick points, whichever way it is facing now.
uint32_t SteerWithStick(uint32_t bits);

// What the controls are called, in the game's order (mask indices 0..11 of InputRecorder_67F8B0)
const char* ActionName(int action);
// "Ctrl", "Space", "F5"; "Key 0x7F" for a code without a name
std::string KeyName(int dik);
// A binding as the player reads it: device 0 is a key, 1 a pad (binding as in the game's setting)
std::string BindingLabel(int device, int binding);
// The pad input that went down between `prev` and `now`, as a game binding: a button number, or
// 224..227 for left, right, up, down on the stick or d-pad. False when nothing did.
bool CapturePadBinding(const plat::PadState& prev, const plat::PadState& now, int& binding);
} // namespace overlay
