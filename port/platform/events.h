// The event pump shared by the message queue (win32.cpp) and DirectInput (dinput.cpp)
#pragma once
#include <stdint.h>

// Drains the platform's events: window events become window messages, key events also reach
// DirectInput's keyboard buffers. Safe to call often.
void port_pump_events();

// Called by the pump for every key press and release, with its DIK code
void port_dinput_key_event(uint8_t dik, bool down);
