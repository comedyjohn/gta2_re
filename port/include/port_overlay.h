// Port hooks from the game code into the controller overlay (platform/overlay.h)
#pragma once

// The control bits of a live game frame, with the stick's walking direction applied
unsigned int port_steer_with_stick(unsigned int bits);

// How hard the gas pedal of the car this ped drives is pressed, as a Fix16 raw value (16384 is full).
// Only the local player's car on a pad's analog trigger gets less than full.
int port_gas_scale(const void* driverPed);
