// Port hooks from the game code into the pad input (platform/controls.cpp, platform/padinput.h).
//
// The game's InputRecorder_67F8B0 reads the keyboard as it always did; the pad is read here once
// per game update. Its analog part (where the stick points, how far, the gas trigger, the turret)
// and a car spawned from the overlay's menu are recorded in the replay as port records, so a replay
// plays back the pad exactly. What the game does with them (port_input_apply, port_gas_scale,
// port_walk_speed) depends only on recorded values and game state.
#pragma once

// A port record's field_4_inputs in a replay (InputRecord_C). The original game's never have bit 31.
const unsigned int kPortRecord = 0x80000000u;
const unsigned int kPortWithPrevious = 0x40000000u; // applied together with the record before it
const unsigned int kPortSpawn = 0x20000000u; // a car spawn (model in the low byte), not analog state
const unsigned int kPortPayload = 0x0001FFFFu;

struct PortPadFrame
{
    unsigned int bits; // the controls the pad holds
    unsigned int analog; // padinput::Pack of this update's analog state
    bool captured; // a menu of the overlay has the input: the game sees the controls at rest
    int spawn; // a car model the spawn menu asked for, or -1
};

// Live: reads the pad for this update
PortPadFrame port_input_read();

// The analog state in force (packed), and changing it (live when it changes, in a replay from the
// record). Reset to rest when a game or a replay starts.
unsigned int port_input_analog();
void port_input_set_analog(unsigned int packed);
void port_input_reset();

// Puts a car of `model` down next to the player
void port_input_spawn(int model);

// The control bits the game acts on this update: with a heading, Forward, Left and Right turn the
// character to it. Also turns a turret.
unsigned int port_input_apply(unsigned int bits);

// How hard the gas pedal of the car this ped drives is pressed, as a Fix16 raw value (16384 is full).
// Only the local player's car on a pad's analog trigger gets less than full.
int port_gas_scale(const void* driverPed);

// The walking speed of this ped (raw Fix16) with the stick's push applied, for the local player
int port_walk_speed(const void* ped, int speed);
