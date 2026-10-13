// The boundary between the game side (which sees port_prelude.h and the Windows-shaped API) and the
// SDL side (port/platform/sdl/, compiled without the prelude, since SDL takes _WIN32 to mean
// Windows). Only plain types cross it.
#pragma once
#include <stdint.h>

namespace plat
{
enum class EventType
{
    None,
    Quit, // the window was closed
    FocusGained,
    FocusLost,
    Minimized,
    Restored,
    KeyDown,
    KeyUp,
};

struct Event
{
    EventType type;
    uint8_t dik; // KeyDown/KeyUp: DirectInput scan code (DIK_*), 0 if the key has none
    uint8_t vk; // KeyDown/KeyUp: Windows virtual-key code for the window procedure, 0 if not mapped
    bool alt; // KeyDown/KeyUp: an Alt key is held
};

// A game controller in the shape the game's DirectInput joystick format reads
struct PadState
{
    int32_t axisX; // -32768..32767
    int32_t axisY;
    int32_t pov; // hundredths of a degree clockwise from up, -1 when centred
    uint8_t buttons[32]; // 0x80 when held
};

bool Init();
void Shutdown();

// Opens the window, with an OpenGL context current on it
bool OpenWindow(int width, int height, const char* title);
void SetWindowTitle(const char* title);
void ToggleFullscreen();
void GetDrawableSize(int& width, int& height);
void SwapWindow();
void* GLGetProcAddress(const char* name);
void SetCursorVisible(bool visible);

// A modal message box (falls back to stderr when there's no display)
void ShowMessage(const char* title, const char* text);

// Pumps the OS queue and returns its events one at a time
bool PollEvent(Event& out);

// DIK-indexed keyboard state, 0x80 when held
void GetKeyboardState(uint8_t state[256]);

int PadCount();
bool GetPadState(int index, PadState& out);
const char* PadName(int index);
} // namespace plat
