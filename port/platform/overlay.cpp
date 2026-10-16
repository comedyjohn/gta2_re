#include "overlay.h"
#include "overlay_font.h"

#include "Game_0x40.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "registry.hpp"

#include <dinput.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace overlay
{
namespace
{
const int kWidth = 640;
const int kHeight = 480;
const int kCharW = 8;
const int kCharH = 16;

// A game pad binding that is not a button (InputDevices_1::read_input_device_498DA0)
const int kStickLeft = 224;
const int kStickRight = 225;
const int kStickUp = 226;
const int kStickDown = 227;
const int kStickThreshold = 16000; // of 32767
const int kButtonSlots = 12; // PadState::buttons the game can bind: the ten buttons and the triggers

// The registry's control settings 0..11 are the game's; 12 is the overlay's own
const int kLegendSetting = 12;
const int kWorldDirectionsSetting = 13;

// What each control does, from Player::CharacterControls_566520
const char* const kActionNames[kActions] = {"Forward",     "Backward",     "Left",
                                            "Right",       "Fire",         "Enter/exit car",
                                            "Handbrake/jump", "Prev weapon", "Next weapon",
                                            "Special 1 (horn)", "Special 2 (camera)", "Special 3"};

// A pad preset: stick and d-pad drive, face buttons act (slots as plat::PadButtonName names them)
const int kPadPreset[kActions] = {kStickUp, kStickDown, kStickLeft, kStickRight,
                                  2 /* X */, 3 /* Y */, 0 /* A */, 4 /* LB */, 5 /* RB */,
                                  1 /* B */, 10 /* LT */, 11 /* RT */};

const uint32_t kPanel = 0xC0101820;
const uint32_t kText = 0xFFFFFFFF;
const uint32_t kDim = 0xFF98A0B0;
const uint32_t kAccent = 0xFFFFD040;
const uint32_t kSelect = 0xFF2848A0;

uint32_t gCanvas[kWidth * kHeight];

bool gSettingsOpen = false;
bool gLegendOn = true;
bool gLegendLoaded = false;
bool gWorldDirections = true;
bool gWorldLoaded = false;
bool gLegendShown = false; // F1 pressed this run: show it even without a pad
int gRow = 0;
bool gListening = false;
int gListenFrames = 0;
std::string gStatus;

plat::PadState gPrevPad;
bool gHavePrev = false;
int gNavHeld = 0; // frames the stick or d-pad has been held up or down

void LoadLegendSetting()
{
    if (!gLegendLoaded)
    {
        gLegendLoaded = true;
        gLegendOn = gRegistry_6FF968.Set_Control_Setting_587010(kLegendSetting, 1) != 0;
    }
}

// The game reads the registry into its binding arrays when it starts a game, and the options menu
// changes both; read them again so the overlay shows what the registry says
void Refresh()
{
    InputDevices_1 devices;
    devices.get_registry_controls_498C00();
}

void LoadWorldSetting()
{
    if (!gWorldLoaded)
    {
        gWorldLoaded = true;
        gWorldDirections = gRegistry_6FF968.Set_Control_Setting_587010(kWorldDirectionsSetting, 1) != 0;
    }
}

void ToggleWorldDirections()
{
    LoadWorldSetting();
    gWorldDirections = !gWorldDirections;
    gRegistry_6FF968.Write_Control_Setting(kWorldDirectionsSetting, gWorldDirections ? 1 : 0);
    gStatus = gWorldDirections ? "Stick: the character walks the way the stick points"
                               : "Stick: left and right turn the character, up walks forward";
}

int FirstPad(plat::PadState& pad)
{
    for (int i = 0; i < 8; i++)
    {
        if (plat::GetPadState(i, pad))
        {
            return i;
        }
    }
    return -1;
}

int StickDirection(const plat::PadState& pad)
{
    if (pad.axisX < -kStickThreshold)
    {
        return kStickLeft;
    }
    if (pad.axisX > kStickThreshold)
    {
        return kStickRight;
    }
    if (pad.axisY < -kStickThreshold)
    {
        return kStickUp;
    }
    if (pad.axisY > kStickThreshold)
    {
        return kStickDown;
    }
    return 0;
}

// ---------------------------------------------------------------- the controls

void Store(int action, int device, int binding)
{
    gMaybeDeviceType_67B91C[action] = device;
    gPlayerControlsBinding_67B6E8[action] = static_cast<uint8_t>(binding);
    gRegistry_6FF968.Write_Control_Setting(action, (device << 15) | binding);
}

// Gives `action` the binding. Another control that had it takes the old one, so none is left dead.
void Assign(int action, int device, int binding)
{
    const int oldDevice = gMaybeDeviceType_67B91C[action];
    const int oldBinding = static_cast<int>(gPlayerControlsBinding_67B6E8[action]);
    for (int other = 0; other < kActions; other++)
    {
        if (other != action && gMaybeDeviceType_67B91C[other] == device &&
            static_cast<int>(gPlayerControlsBinding_67B6E8[other]) == binding)
        {
            Store(other, oldDevice, oldBinding);
        }
    }
    Store(action, device, binding);
}

void ResetKeyboard(int action)
{
    Assign(action, 0, gDefaultControls_61A9E4[action]);
}

void ApplyPadPreset()
{
    // Every control moves at once, so there are no swaps to make
    for (int i = 0; i < kActions; i++)
    {
        Store(i, 1, kPadPreset[i]);
    }
    gStatus = "Pad preset applied";
}

void ApplyKeyboardDefaults()
{
    for (int i = 0; i < kActions; i++)
    {
        Store(i, 0, gDefaultControls_61A9E4[i]);
    }
    gStatus = "Keyboard defaults restored";
}

void Move(int delta)
{
    gRow = (gRow + delta + kActions) % kActions;
    gStatus.clear();
}

void StartListening()
{
    gListening = true;
    gListenFrames = 0;
    gStatus = std::string("Press a pad button or a key for ") + kActionNames[gRow];
}

void CloseSettings()
{
    gSettingsOpen = false;
    gListening = false;
    gStatus.clear();
}

void ToggleSettings()
{
    if (gSettingsOpen)
    {
        CloseSettings();
        return;
    }
    Refresh();
    gSettingsOpen = true;
    gListening = false;
    gStatus.clear();
}

// ---------------------------------------------------------------- drawing

void Fill(int x, int y, int w, int h, uint32_t argb)
{
    for (int j = y < 0 ? 0 : y; j < y + h && j < kHeight; j++)
    {
        for (int i = x < 0 ? 0 : x; i < x + w && i < kWidth; i++)
        {
            gCanvas[j * kWidth + i] = argb;
        }
    }
}

void Text(int x, int y, uint32_t argb, const std::string& s)
{
    for (size_t n = 0; n < s.size(); n++)
    {
        const unsigned char c = static_cast<unsigned char>(s[n]);
        if (c < 32 || c > 126)
        {
            continue;
        }
        const uint8_t* glyph = kOverlayFont[c - 32];
        for (int row = 0; row < kCharH; row++)
        {
            for (int col = 0; col < kCharW; col++)
            {
                const int px = x + static_cast<int>(n) * kCharW + col;
                const int py = y + row;
                if ((glyph[row] & (0x80 >> col)) && px >= 0 && px < kWidth && py >= 0 && py < kHeight)
                {
                    gCanvas[py * kWidth + px] = argb;
                }
            }
        }
    }
}

std::string Pad(const std::string& s, size_t width)
{
    return s.size() >= width ? s : s + std::string(width - s.size(), ' ');
}

void DrawRow(int x, int y, int action, bool selected, int panelW)
{
    if (selected)
    {
        Fill(x - 4, y, panelW - 8, kCharH, kSelect);
    }
    const bool pad = gMaybeDeviceType_67B91C[action] == 1;
    Text(x, y, kText, Pad(kActionNames[action], 19));
    Text(x + 19 * kCharW, y, pad ? kAccent : kDim,
         BindingLabel(gMaybeDeviceType_67B91C[action], static_cast<int>(gPlayerControlsBinding_67B6E8[action])));
}

void DrawLegend()
{
    const int w = 19 * kCharW + 12 * kCharW + 16;
    const int h = (kActions + 3) * kCharH + 8;
    const int x = kWidth - w - 8;
    const int y = 8;
    Fill(x, y, w, h, kPanel);
    Text(x + 8, y + 4, kText, "Controls");
    for (int i = 0; i < kActions; i++)
    {
        DrawRow(x + 8, y + 4 + (i + 1) * kCharH, i, false, w);
    }
    Text(x + 8, y + 4 + (kActions + 1) * kCharH, kDim, "F1 hide  F2 settings");
}

void DrawSettings()
{
    const int w = 592;
    const int h = (kActions + 12) * kCharH + 16;
    const int x = (kWidth - w) / 2;
    const int y = (kHeight - h) / 2;
    Fill(x, y, w, h, kPanel);
    int line = y + 8;
    Text(x + 8, line, kAccent, "Controller settings");
    line += kCharH;
    plat::PadState pad;
    const int index = FirstPad(pad);
    Text(x + 8, line, kDim, index >= 0 ? std::string("Pad: ") + plat::PadName(index) : "No controller connected");
    line += kCharH * 2;
    for (int i = 0; i < kActions; i++)
    {
        DrawRow(x + 8, line, i, i == gRow, w - 8);
        line += kCharH;
    }
    line += kCharH;
    Text(x + 8, line, kAccent, gStatus);
    line += kCharH * 2;
    Text(x + 8, line, kDim, "Up/Down select   Enter or A rebind   Del or X reset this one");
    line += kCharH;
    Text(x + 8, line, kDim, "P or Y pad preset   K keyboard defaults   Esc, B or F2 close");
    line += kCharH;
    Text(x + 8, line, kDim, std::string("M or R3 stick moves the way it points: ") + (gWorldDirections ? "on" : "off"));
    line += kCharH;
    Text(x + 8, line, kDim, "Pad buttons by position: A bottom, B right, X left, Y top");
}

// ---------------------------------------------------------------- pad navigation

void UpdatePad()
{
    plat::PadState pad;
    if (FirstPad(pad) < 0)
    {
        gHavePrev = false;
        return;
    }
    if (!gHavePrev)
    {
        gPrevPad = pad;
        gHavePrev = true;
    }
    const plat::PadState& prev = gPrevPad;
    auto pressed = [&](int slot) { return pad.buttons[slot] && !prev.buttons[slot]; };

    const bool chord = (pad.buttons[6] && pad.buttons[7]) && (pressed(6) || pressed(7));
    const bool guide = pad.guide && !prev.guide;
    if (chord || guide)
    {
        ToggleSettings();
    }
    else if (gSettingsOpen)
    {
        if (gListening)
        {
            int binding = 0;
            if (CapturePadBinding(prev, pad, binding))
            {
                const int action = gRow;
                Assign(action, 1, binding);
                gStatus = std::string(kActionNames[action]) + ": " + BindingLabel(1, binding);
                gListening = false;
            }
            else if (++gListenFrames > 60 * 6)
            {
                gListening = false;
                gStatus = "Cancelled";
            }
        }
        else
        {
            const int dir = StickDirection(pad);
            const int prevDir = StickDirection(prev);
            int delta = 0;
            if (dir == kStickUp || dir == kStickDown)
            {
                // Once on the press, then repeating while it is held
                gNavHeld = dir == prevDir ? gNavHeld + 1 : 0;
                if (gNavHeld == 0 || (gNavHeld > 20 && gNavHeld % 5 == 0))
                {
                    delta = dir == kStickUp ? -1 : 1;
                }
            }
            else
            {
                gNavHeld = 0;
            }
            if (delta)
            {
                Move(delta);
            }
            if (pressed(0))
            {
                StartListening();
            }
            else if (pressed(1))
            {
                CloseSettings();
            }
            else if (pressed(2))
            {
                ResetKeyboard(gRow);
                gStatus = std::string(kActionNames[gRow]) + ": back to its key";
            }
            else if (pressed(3))
            {
                ApplyPadPreset();
            }
            else if (pressed(9))
            {
                ToggleWorldDirections();
            }
        }
    }
    gPrevPad = pad;
}
} // namespace

const char* ActionName(int action)
{
    return (action >= 0 && action < kActions) ? kActionNames[action] : "";
}

std::string KeyName(int dik)
{
    struct Named
    {
        int dik;
        const char* name;
    };
    static const Named kNames[] = {
        {DIK_ESCAPE, "Esc"},      {DIK_MINUS, "-"},       {DIK_EQUALS, "="},        {DIK_BACK, "Backspace"},
        {DIK_TAB, "Tab"},         {DIK_RETURN, "Enter"},  {DIK_LCONTROL, "Ctrl"},   {DIK_RCONTROL, "Right Ctrl"},
        {DIK_LSHIFT, "Shift"},    {DIK_RSHIFT, "Right Shift"}, {DIK_LMENU, "Alt"},  {DIK_RMENU, "Right Alt"},
        {DIK_SPACE, "Space"},     {DIK_UP, "Up"},         {DIK_DOWN, "Down"},       {DIK_LEFT, "Left"},
        {DIK_RIGHT, "Right"},     {DIK_INSERT, "Insert"}, {DIK_DELETE, "Delete"},   {DIK_HOME, "Home"},
        {DIK_END, "End"},         {DIK_PRIOR, "Page Up"}, {DIK_NEXT, "Page Down"},  {DIK_CAPITAL, "Caps Lock"},
        {DIK_COMMA, ","},         {DIK_PERIOD, "."},      {DIK_SLASH, "/"},         {DIK_SEMICOLON, ";"},
        {DIK_APOSTROPHE, "'"},    {DIK_LBRACKET, "["},    {DIK_RBRACKET, "]"},      {DIK_BACKSLASH, "\\"},
        {DIK_GRAVE, "`"},         {DIK_NUMPADENTER, "Num Enter"}, {DIK_ADD, "Num +"}, {DIK_SUBTRACT, "Num -"},
        {DIK_MULTIPLY, "Num *"},  {DIK_DIVIDE, "Num /"},  {DIK_DECIMAL, "Num ."},
    };
    for (const Named& n : kNames)
    {
        if (n.dik == dik)
        {
            return n.name;
        }
    }
    static const int kLetters[] = {DIK_A, DIK_B, DIK_C, DIK_D, DIK_E, DIK_F, DIK_G, DIK_H, DIK_I,
                                   DIK_J, DIK_K, DIK_L, DIK_M, DIK_N, DIK_O, DIK_P, DIK_Q, DIK_R,
                                   DIK_S, DIK_T, DIK_U, DIK_V, DIK_W, DIK_X, DIK_Y, DIK_Z};
    for (int i = 0; i < 26; i++)
    {
        if (kLetters[i] == dik)
        {
            return std::string(1, static_cast<char>('A' + i));
        }
    }
    static const int kDigits[] = {DIK_1, DIK_2, DIK_3, DIK_4, DIK_5, DIK_6, DIK_7, DIK_8, DIK_9, DIK_0};
    for (int i = 0; i < 10; i++)
    {
        if (kDigits[i] == dik)
        {
            return std::string(1, static_cast<char>(i == 9 ? '0' : '1' + i));
        }
    }
    static const int kFunction[] = {DIK_F1, DIK_F2, DIK_F3, DIK_F4,  DIK_F5,  DIK_F6,
                                    DIK_F7, DIK_F8, DIK_F9, DIK_F10, DIK_F11, DIK_F12};
    for (int i = 0; i < 12; i++)
    {
        if (kFunction[i] == dik)
        {
            return "F" + std::to_string(i + 1);
        }
    }
    static const int kNumpad[] = {DIK_NUMPAD0, DIK_NUMPAD1, DIK_NUMPAD2, DIK_NUMPAD3, DIK_NUMPAD4,
                                  DIK_NUMPAD5, DIK_NUMPAD6, DIK_NUMPAD7, DIK_NUMPAD8, DIK_NUMPAD9};
    for (int i = 0; i < 10; i++)
    {
        if (kNumpad[i] == dik)
        {
            return "Num " + std::to_string(i);
        }
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "Key 0x%02X", dik & 0xFF);
    return buf;
}

std::string BindingLabel(int device, int binding)
{
    if (device == 0)
    {
        return KeyName(binding);
    }
    switch (binding)
    {
        case kStickLeft:
            return "D-pad Left";
        case kStickRight:
            return "D-pad Right";
        case kStickUp:
            return "D-pad Up";
        case kStickDown:
            return "D-pad Down";
        default:
            break;
    }
    if (binding >= 0 && binding < kButtonSlots)
    {
        return plat::PadButtonName(binding);
    }
    return "Button " + std::to_string(binding + 1);
}

bool CapturePadBinding(const plat::PadState& prev, const plat::PadState& now, int& binding)
{
    for (int slot = 0; slot < kButtonSlots; slot++)
    {
        if (now.buttons[slot] && !prev.buttons[slot])
        {
            binding = slot;
            return true;
        }
    }
    const int dir = StickDirection(now);
    if (dir && dir != StickDirection(prev))
    {
        binding = dir;
        return true;
    }
    return false;
}

bool Captured()
{
    return gSettingsOpen;
}

bool OnKey(uint8_t dik, bool down)
{
    if (dik == DIK_F1 || dik == DIK_F2)
    {
        if (down)
        {
            if (dik == DIK_F1)
            {
                LoadLegendSetting();
                // Without a pad the legend is hidden, so the first press shows it
                plat::PadState pad;
                const bool visible = gLegendOn && (FirstPad(pad) >= 0 || gLegendShown);
                gLegendShown = true;
                gLegendOn = !visible;
                gRegistry_6FF968.Write_Control_Setting(kLegendSetting, gLegendOn ? 1 : 0);
            }
            else
            {
                ToggleSettings();
            }
        }
        return true;
    }
    if (!gSettingsOpen || !down)
    {
        return false;
    }
    if (gListening)
    {
        if (dik == DIK_ESCAPE)
        {
            gListening = false;
            gStatus = "Cancelled";
        }
        else
        {
            Assign(gRow, 0, dik);
            gStatus = std::string(kActionNames[gRow]) + ": " + KeyName(dik);
            gListening = false;
        }
        return true;
    }
    switch (dik)
    {
        case DIK_UP:
            Move(-1);
            break;
        case DIK_DOWN:
            Move(1);
            break;
        case DIK_RETURN:
        case DIK_NUMPADENTER:
            StartListening();
            break;
        case DIK_DELETE:
        case DIK_BACK:
            ResetKeyboard(gRow);
            gStatus = std::string(kActionNames[gRow]) + ": back to " + KeyName(gDefaultControls_61A9E4[gRow]);
            break;
        case DIK_P:
            ApplyPadPreset();
            break;
        case DIK_K:
            ApplyKeyboardDefaults();
            break;
        case DIK_M:
            ToggleWorldDirections();
            break;
        case DIK_ESCAPE:
            CloseSettings();
            break;
        default:
            break;
    }
    return true;
}

uint32_t SteerWithStick(uint32_t bits)
{
    static bool injected = false;
    const uint32_t kMoveBits = 0xF; // Forward, Backward, Left, Right
    plat::PadState pad;
    bool active = false;
    uint32_t mine = 0;
    if (FirstPad(pad) >= 0 && !gSettingsOpen)
    {
        LoadWorldSetting();
        Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
        Ped* ped = player ? player->field_2C4_player_ped : nullptr;
        const double x = pad.axisX / 32767.0;
        const double y = pad.axisY / 32767.0;
        // The stick's own trigger point (see dinput.cpp's Trigger)
        if (gWorldDirections && ped && !ped->get_car_416B60() && ped->field_168_game_object && x * x + y * y > 0.4 * 0.4)
        {
            // Rotation 0 faces down the screen and grows to the left (found by playing: up walked down),
            // 1440 to a turn
            const double wanted = atan2(x, y) * 1440.0 / (2.0 * M_PI);
            int diff = static_cast<int>(lround(wanted)) - ped->GetRotation().rValue;
            diff = ((diff % 1440) + 1440 + 720) % 1440 - 720;
            // Stop pressing a turn once the speed it has built up would carry it there
            const int speed = player->field_8_turn_speed.rValue < 0 ? -player->field_8_turn_speed.rValue
                                                                     : player->field_8_turn_speed.rValue;
            const int dead = speed > 12 ? speed : 12;
            if (diff > dead)
            {
                mine |= 4;
            }
            else if (diff < -dead)
            {
                mine |= 8;
            }
            if (diff < 300 && diff > -300)
            {
                mine |= 1;
            }
            active = true;
        }
    }
    if (active)
    {
        injected = true;
        return (bits & ~kMoveBits) | mine;
    }
    if (injected)
    {
        injected = false;
        return bits & ~kMoveBits;
    }
    return bits;
}

const uint32_t* Frame()
{
    plat::PadState pad;
    const bool havePad = FirstPad(pad) >= 0;
    if (havePad)
    {
        LoadLegendSetting();
    }
    if (havePad)
    {
        LoadWorldSetting();
    }
    if (havePad || gSettingsOpen)
    {
        UpdatePad();
    }
    const bool legend = !gSettingsOpen && gLegendOn && gLegendLoaded && (havePad || gLegendShown);
    if (!gSettingsOpen && !legend)
    {
        return nullptr;
    }
    static int frames = 0;
    if (!gSettingsOpen && frames++ % 30 == 0)
    {
        Refresh();
    }
    memset(gCanvas, 0, sizeof(gCanvas));
    if (gSettingsOpen)
    {
        DrawSettings();
    }
    else
    {
        DrawLegend();
    }
    return gCanvas;
}
} // namespace overlay

unsigned int port_steer_with_stick(unsigned int bits)
{
    return overlay::SteerWithStick(bits);
}
