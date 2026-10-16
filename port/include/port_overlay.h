// Port hooks from the game code into the controller overlay (platform/overlay.h)
#pragma once

// The control bits of a live game frame, with the stick's walking direction applied
unsigned int port_steer_with_stick(unsigned int bits);
