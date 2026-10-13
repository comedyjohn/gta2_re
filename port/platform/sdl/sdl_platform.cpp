// SDL2 implementation of port/platform/platform.h. Compiled without port_prelude.h.
#include "../platform.h"

#include <SDL.h>

#include <stdio.h>
#include <string.h>

namespace
{
SDL_Window* gWindow = nullptr;
SDL_GLContext gContext = nullptr;
SDL_GameController* gPads[8] = {};
uint8_t gScancodeToDik[SDL_NUM_SCANCODES] = {};

struct ScancodeDik
{
    SDL_Scancode mScancode;
    uint8_t mDik;
};

// Values from the DirectX SDK's DINPUT.H, matched by key name
const ScancodeDik kScancodeDik[] = {
    {SDL_SCANCODE_ESCAPE, 0x01}, // DIK_ESCAPE
    {SDL_SCANCODE_1, 0x02}, // DIK_1
    {SDL_SCANCODE_2, 0x03}, // DIK_2
    {SDL_SCANCODE_3, 0x04}, // DIK_3
    {SDL_SCANCODE_4, 0x05}, // DIK_4
    {SDL_SCANCODE_5, 0x06}, // DIK_5
    {SDL_SCANCODE_6, 0x07}, // DIK_6
    {SDL_SCANCODE_7, 0x08}, // DIK_7
    {SDL_SCANCODE_8, 0x09}, // DIK_8
    {SDL_SCANCODE_9, 0x0A}, // DIK_9
    {SDL_SCANCODE_0, 0x0B}, // DIK_0
    {SDL_SCANCODE_MINUS, 0x0C}, // DIK_MINUS
    {SDL_SCANCODE_EQUALS, 0x0D}, // DIK_EQUALS
    {SDL_SCANCODE_BACKSPACE, 0x0E}, // DIK_BACK
    {SDL_SCANCODE_TAB, 0x0F}, // DIK_TAB
    {SDL_SCANCODE_Q, 0x10}, // DIK_Q
    {SDL_SCANCODE_W, 0x11}, // DIK_W
    {SDL_SCANCODE_E, 0x12}, // DIK_E
    {SDL_SCANCODE_R, 0x13}, // DIK_R
    {SDL_SCANCODE_T, 0x14}, // DIK_T
    {SDL_SCANCODE_Y, 0x15}, // DIK_Y
    {SDL_SCANCODE_U, 0x16}, // DIK_U
    {SDL_SCANCODE_I, 0x17}, // DIK_I
    {SDL_SCANCODE_O, 0x18}, // DIK_O
    {SDL_SCANCODE_P, 0x19}, // DIK_P
    {SDL_SCANCODE_LEFTBRACKET, 0x1A}, // DIK_LBRACKET
    {SDL_SCANCODE_RIGHTBRACKET, 0x1B}, // DIK_RBRACKET
    {SDL_SCANCODE_RETURN, 0x1C}, // DIK_RETURN
    {SDL_SCANCODE_LCTRL, 0x1D}, // DIK_LCONTROL
    {SDL_SCANCODE_A, 0x1E}, // DIK_A
    {SDL_SCANCODE_S, 0x1F}, // DIK_S
    {SDL_SCANCODE_D, 0x20}, // DIK_D
    {SDL_SCANCODE_F, 0x21}, // DIK_F
    {SDL_SCANCODE_G, 0x22}, // DIK_G
    {SDL_SCANCODE_H, 0x23}, // DIK_H
    {SDL_SCANCODE_J, 0x24}, // DIK_J
    {SDL_SCANCODE_K, 0x25}, // DIK_K
    {SDL_SCANCODE_L, 0x26}, // DIK_L
    {SDL_SCANCODE_SEMICOLON, 0x27}, // DIK_SEMICOLON
    {SDL_SCANCODE_APOSTROPHE, 0x28}, // DIK_APOSTROPHE
    {SDL_SCANCODE_GRAVE, 0x29}, // DIK_GRAVE
    {SDL_SCANCODE_LSHIFT, 0x2A}, // DIK_LSHIFT
    {SDL_SCANCODE_BACKSLASH, 0x2B}, // DIK_BACKSLASH
    {SDL_SCANCODE_Z, 0x2C}, // DIK_Z
    {SDL_SCANCODE_X, 0x2D}, // DIK_X
    {SDL_SCANCODE_C, 0x2E}, // DIK_C
    {SDL_SCANCODE_V, 0x2F}, // DIK_V
    {SDL_SCANCODE_B, 0x30}, // DIK_B
    {SDL_SCANCODE_N, 0x31}, // DIK_N
    {SDL_SCANCODE_M, 0x32}, // DIK_M
    {SDL_SCANCODE_COMMA, 0x33}, // DIK_COMMA
    {SDL_SCANCODE_PERIOD, 0x34}, // DIK_PERIOD
    {SDL_SCANCODE_SLASH, 0x35}, // DIK_SLASH
    {SDL_SCANCODE_RSHIFT, 0x36}, // DIK_RSHIFT
    {SDL_SCANCODE_KP_MULTIPLY, 0x37}, // DIK_MULTIPLY
    {SDL_SCANCODE_LALT, 0x38}, // DIK_LMENU
    {SDL_SCANCODE_SPACE, 0x39}, // DIK_SPACE
    {SDL_SCANCODE_CAPSLOCK, 0x3A}, // DIK_CAPITAL
    {SDL_SCANCODE_F1, 0x3B}, // DIK_F1
    {SDL_SCANCODE_F2, 0x3C}, // DIK_F2
    {SDL_SCANCODE_F3, 0x3D}, // DIK_F3
    {SDL_SCANCODE_F4, 0x3E}, // DIK_F4
    {SDL_SCANCODE_F5, 0x3F}, // DIK_F5
    {SDL_SCANCODE_F6, 0x40}, // DIK_F6
    {SDL_SCANCODE_F7, 0x41}, // DIK_F7
    {SDL_SCANCODE_F8, 0x42}, // DIK_F8
    {SDL_SCANCODE_F9, 0x43}, // DIK_F9
    {SDL_SCANCODE_F10, 0x44}, // DIK_F10
    {SDL_SCANCODE_NUMLOCKCLEAR, 0x45}, // DIK_NUMLOCK
    {SDL_SCANCODE_SCROLLLOCK, 0x46}, // DIK_SCROLL
    {SDL_SCANCODE_KP_7, 0x47}, // DIK_NUMPAD7
    {SDL_SCANCODE_KP_8, 0x48}, // DIK_NUMPAD8
    {SDL_SCANCODE_KP_9, 0x49}, // DIK_NUMPAD9
    {SDL_SCANCODE_KP_MINUS, 0x4A}, // DIK_SUBTRACT
    {SDL_SCANCODE_KP_4, 0x4B}, // DIK_NUMPAD4
    {SDL_SCANCODE_KP_5, 0x4C}, // DIK_NUMPAD5
    {SDL_SCANCODE_KP_6, 0x4D}, // DIK_NUMPAD6
    {SDL_SCANCODE_KP_PLUS, 0x4E}, // DIK_ADD
    {SDL_SCANCODE_KP_1, 0x4F}, // DIK_NUMPAD1
    {SDL_SCANCODE_KP_2, 0x50}, // DIK_NUMPAD2
    {SDL_SCANCODE_KP_3, 0x51}, // DIK_NUMPAD3
    {SDL_SCANCODE_KP_0, 0x52}, // DIK_NUMPAD0
    {SDL_SCANCODE_KP_PERIOD, 0x53}, // DIK_DECIMAL
    {SDL_SCANCODE_F11, 0x57}, // DIK_F11
    {SDL_SCANCODE_F12, 0x58}, // DIK_F12
    {SDL_SCANCODE_F13, 0x64}, // DIK_F13
    {SDL_SCANCODE_F14, 0x65}, // DIK_F14
    {SDL_SCANCODE_F15, 0x66}, // DIK_F15
    {SDL_SCANCODE_KP_EQUALS, 0x8D}, // DIK_NUMPADEQUALS
    {SDL_SCANCODE_KP_ENTER, 0x9C}, // DIK_NUMPADENTER
    {SDL_SCANCODE_RCTRL, 0x9D}, // DIK_RCONTROL
    {SDL_SCANCODE_KP_COMMA, 0xB3}, // DIK_NUMPADCOMMA
    {SDL_SCANCODE_KP_DIVIDE, 0xB5}, // DIK_DIVIDE
    {SDL_SCANCODE_PRINTSCREEN, 0xB7}, // DIK_SYSRQ
    {SDL_SCANCODE_RALT, 0xB8}, // DIK_RMENU
    {SDL_SCANCODE_PAUSE, 0xC5}, // DIK_PAUSE
    {SDL_SCANCODE_HOME, 0xC7}, // DIK_HOME
    {SDL_SCANCODE_UP, 0xC8}, // DIK_UP
    {SDL_SCANCODE_PAGEUP, 0xC9}, // DIK_PRIOR
    {SDL_SCANCODE_LEFT, 0xCB}, // DIK_LEFT
    {SDL_SCANCODE_RIGHT, 0xCD}, // DIK_RIGHT
    {SDL_SCANCODE_END, 0xCF}, // DIK_END
    {SDL_SCANCODE_DOWN, 0xD0}, // DIK_DOWN
    {SDL_SCANCODE_PAGEDOWN, 0xD1}, // DIK_NEXT
    {SDL_SCANCODE_INSERT, 0xD2}, // DIK_INSERT
    {SDL_SCANCODE_DELETE, 0xD3}, // DIK_DELETE
    {SDL_SCANCODE_LGUI, 0xDB}, // DIK_LWIN
    {SDL_SCANCODE_RGUI, 0xDC}, // DIK_RWIN
    {SDL_SCANCODE_APPLICATION, 0xDD}, // DIK_APPS
};

// Windows virtual-key codes for the keys the game's window procedure looks at
uint8_t VirtualKey(SDL_Keycode key)
{
    if (key >= SDLK_a && key <= SDLK_z)
    {
        return static_cast<uint8_t>('A' + (key - SDLK_a));
    }
    if (key >= SDLK_0 && key <= SDLK_9)
    {
        return static_cast<uint8_t>('0' + (key - SDLK_0));
    }
    if (key >= SDLK_F1 && key <= SDLK_F12)
    {
        return static_cast<uint8_t>(0x70 + (key - SDLK_F1));
    }
    switch (key)
    {
        case SDLK_BACKSPACE:
            return 0x08;
        case SDLK_TAB:
            return 0x09;
        case SDLK_RETURN:
            return 0x0D;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            return 0x10;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            return 0x11;
        case SDLK_LALT:
            return 0xA4; // VK_LMENU
        case SDLK_RALT:
            return 0xA5; // VK_RMENU
        case SDLK_ESCAPE:
            return 0x1B;
        case SDLK_SPACE:
            return 0x20;
        case SDLK_LEFT:
            return 0x25;
        case SDLK_UP:
            return 0x26;
        case SDLK_RIGHT:
            return 0x27;
        case SDLK_DOWN:
            return 0x28;
        default:
            return 0;
    }
}

void OpenPads()
{
    for (int i = 0; i < SDL_NumJoysticks() && i < 8; i++)
    {
        if (!gPads[i] && SDL_IsGameController(i))
        {
            gPads[i] = SDL_GameControllerOpen(i);
        }
    }
}
} // namespace

namespace plat
{
bool Init()
{
    for (const ScancodeDik& m : kScancodeDik)
    {
        gScancodeToDik[m.mScancode] = m.mDik;
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    OpenPads();
    return true;
}

void Shutdown()
{
    for (SDL_GameController*& pad : gPads)
    {
        if (pad)
        {
            SDL_GameControllerClose(pad);
            pad = nullptr;
        }
    }
    if (gContext)
    {
        SDL_GL_DeleteContext(gContext);
        gContext = nullptr;
    }
    if (gWindow)
    {
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
    }
    SDL_Quit();
}

bool OpenWindow(int width, int height, const char* title)
{
    if (gWindow)
    {
        SDL_SetWindowSize(gWindow, width, height);
        return true;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    gWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!gWindow)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    gContext = SDL_GL_CreateContext(gWindow);
    if (!gContext)
    {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetSwapInterval(1);
    return true;
}

void SetWindowTitle(const char* title)
{
    if (gWindow)
    {
        SDL_SetWindowTitle(gWindow, title);
    }
}

void ToggleFullscreen()
{
    if (gWindow)
    {
        const bool full = (SDL_GetWindowFlags(gWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
        SDL_SetWindowFullscreen(gWindow, full ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void GetDrawableSize(int& width, int& height)
{
    width = 640;
    height = 480;
    if (gWindow)
    {
        SDL_GL_GetDrawableSize(gWindow, &width, &height);
    }
}

void SwapWindow()
{
    if (gWindow)
    {
        SDL_GL_SwapWindow(gWindow);
    }
}

void* GLGetProcAddress(const char* name)
{
    return SDL_GL_GetProcAddress(name);
}

void SetCursorVisible(bool visible)
{
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void ShowMessage(const char* title, const char* text)
{
    fprintf(stderr, "[%s] %s\n", title, text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, text, gWindow);
}

bool PollEvent(Event& out)
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        out = Event();
        switch (e.type)
        {
            case SDL_QUIT:
                out.type = EventType::Quit;
                return true;
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        out.type = EventType::FocusGained;
                        return true;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        out.type = EventType::FocusLost;
                        return true;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        out.type = EventType::Minimized;
                        return true;
                    case SDL_WINDOWEVENT_RESTORED:
                        out.type = EventType::Restored;
                        return true;
                    default:
                        break;
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (e.key.repeat)
                {
                    break;
                }
                out.type = e.type == SDL_KEYDOWN ? EventType::KeyDown : EventType::KeyUp;
                out.dik = gScancodeToDik[e.key.keysym.scancode];
                out.vk = VirtualKey(e.key.keysym.sym);
                out.alt = (e.key.keysym.mod & KMOD_ALT) != 0;
                return true;
            case SDL_CONTROLLERDEVICEADDED:
                OpenPads();
                break;
            default:
                break;
        }
    }
    return false;
}

void GetKeyboardState(uint8_t state[256])
{
    memset(state, 0, 256);
    int count = 0;
    const Uint8* keys = SDL_GetKeyboardState(&count);
    for (int sc = 0; sc < count && sc < SDL_NUM_SCANCODES; sc++)
    {
        if (keys[sc] && gScancodeToDik[sc])
        {
            state[gScancodeToDik[sc]] = 0x80;
        }
    }
}

int PadCount()
{
    int n = 0;
    for (SDL_GameController* pad : gPads)
    {
        n += pad ? 1 : 0;
    }
    return n;
}

const char* PadName(int index)
{
    return (index >= 0 && index < 8 && gPads[index]) ? SDL_GameControllerName(gPads[index]) : "";
}

bool GetPadState(int index, PadState& out)
{
    memset(&out, 0, sizeof(out));
    out.pov = -1;
    if (index < 0 || index >= 8 || !gPads[index])
    {
        return false;
    }
    SDL_GameController* pad = gPads[index];
    out.axisX = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
    out.axisY = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);

    // Button order as a typical DirectInput pad of the time reports them
    const SDL_GameControllerButton order[] = {
        SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,          SDL_CONTROLLER_BUTTON_X,
        SDL_CONTROLLER_BUTTON_Y,         SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_BACK,      SDL_CONTROLLER_BUTTON_START,      SDL_CONTROLLER_BUTTON_LEFTSTICK,
        SDL_CONTROLLER_BUTTON_RIGHTSTICK};
    int b = 0;
    for (SDL_GameControllerButton button : order)
    {
        out.buttons[b++] = SDL_GameControllerGetButton(pad, button) ? 0x80 : 0;
    }
    // Triggers as two more buttons
    out.buttons[b++] = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000 ? 0x80 : 0;
    out.buttons[b++] = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000 ? 0x80 : 0;

    const bool up = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP);
    const bool down = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    const bool left = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    const bool right = SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    if (up || down || left || right)
    {
        const int dx = (right ? 1 : 0) - (left ? 1 : 0);
        const int dy = (down ? 1 : 0) - (up ? 1 : 0);
        static const int kPov[3][3] = {{31500, 0, 4500}, {27000, -1, 9000}, {22500, 18000, 13500}};
        out.pov = kPov[dy + 1][dx + 1];
        // The d-pad also moves the stick, for bindings that only read the axes
        if (dx)
        {
            out.axisX = dx * 32767;
        }
        if (dy)
        {
            out.axisY = dy * 32767;
        }
    }
    return true;
}
} // namespace plat
