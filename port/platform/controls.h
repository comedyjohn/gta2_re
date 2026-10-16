// The pad's link to the game (port/include/port_input.h has the game's side): the pad settings in
// use, the overlay's car spawn request, and the pad in the front-end menus.
#pragma once
#include "padinput.h"

#include <stdint.h>

namespace controls
{
// The first connected pad, or -1
int FirstPad(plat::PadState& pad);

// The pad settings (loaded from padinput::SettingsFile() on first use), and saving changes to them
padinput::Settings& Pad();
void SavePad();

// Where the local player is: walking or driving
padinput::Context CurrentContext();

// The overlay's spawn menu: a car of `model` at the next game update (recorded in a replay)
void RequestSpawn(int model);

// While the front end is up, the pad drives its menus as the arrow keys, Enter and Esc do: ORs
// those keys into a keyboard state (256 DirectInput key bytes)
void MenuKeys(uint8_t keys[256]);
} // namespace controls
