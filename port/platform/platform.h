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
    int32_t triggerLeft; // 0..32767, how far the trigger is pressed
    int32_t triggerRight;
    bool guide; // the Guide / Home button, which the game has no slot for
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
// What the pad's button in DirectInput slot `slot` (PadState::buttons) is called, "" past the last one
const char* PadButtonName(int slot);

// Audio output: interleaved signed 16-bit stereo, pulled from a callback on SDL's audio thread.
// OpenAudio asks for `rate` frames a second and returns the rate the device actually took.
typedef void (*AudioCallback)(int16_t* out, int frames, void* user);
bool OpenAudio(int rate, AudioCallback callback, void* user, int& actualRate);
void CloseAudio();
void PauseAudio(bool paused);
// Held while changing anything the callback reads; the callback runs with it held
void LockAudio();
void UnlockAudio();

// A WAV file image (PCM, or Microsoft or IMA ADPCM) decoded to signed 16-bit frames, at its own
// rate and channel count. The samples are released with FreeWav.
struct Wav
{
    int16_t* samples;
    uint32_t frames;
    int channels;
    int rate;
};
bool DecodeWav(const void* data, uint32_t size, Wav& out);
void FreeWav(Wav& wav);
} // namespace plat
