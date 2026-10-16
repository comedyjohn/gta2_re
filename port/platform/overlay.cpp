#include "overlay.h"
#include "overlay_font.h"

#include "Car_BC.hpp"
#include "Game_0x40.hpp"
#include "char.hpp"
#include "winmain.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "registry.hpp"

#include <dinput.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
const int kFull = 16384; // Fix16 1.0
const int kTriggerDead = 2600; // of 32767
int gGasScale = kFull;

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
bool gSpawnOpen = false; // the car menu
int gCar = 0; // its selected entry
int gSpawnRequest = -1; // a model to spawn at the next game frame

// The cars the menu offers (car_model_enum): not trains, trailers, wrecks or unused slots
struct CarEntry
{
    int model;
    const char* name;
};
const CarEntry kCars[] = {
    {car_model_enum::alfa, "Alfa"},
    {car_model_enum::allard, "Allard"},
    {car_model_enum::amdb4, "Aston Martin DB4"},
    {car_model_enum::apc, "APC"},
    {car_model_enum::bank_van, "Bank van"},
    {car_model_enum::bmw, "BMW"},
    {car_model_enum::boxtruck, "Box truck"},
    {car_model_enum::bug, "Bug"},
    {car_model_enum::BUICK, "Buick"},
    {car_model_enum::BUS, "Bus"},
    {car_model_enum::COPCAR, "Police car"},
    {car_model_enum::DART, "Dart"},
    {car_model_enum::EDSEL, "Edsel"},
    {car_model_enum::EDSELFBI, "Edsel (FBI)"},
    {car_model_enum::FIAT, "Fiat"},
    {car_model_enum::FIRETRUK, "Fire truck"},
    {car_model_enum::GRAHAM, "Graham"},
    {car_model_enum::GT24640, "GT 24640"},
    {car_model_enum::GTRUCK, "G-truck"},
    {car_model_enum::GUNJEEP, "Gun jeep"},
    {car_model_enum::HOTDOG, "Hot dog van"},
    {car_model_enum::ICECREAM, "Ice cream van"},
    {car_model_enum::ISETLIMO, "Isetta limo"},
    {car_model_enum::ISETTA, "Isetta"},
    {car_model_enum::JEEP, "Jeep"},
    {car_model_enum::JEFFREY, "Jeffrey"},
    {car_model_enum::KRSNABUS, "Krishna bus"},
    {car_model_enum::LIMO, "Limo"},
    {car_model_enum::LIMO2, "Limo 2"},
    {car_model_enum::MEDICAR, "Ambulance"},
    {car_model_enum::MERC, "Merc"},
    {car_model_enum::MESSER, "Messerschmitt"},
    {car_model_enum::MIURA, "Miura"},
    {car_model_enum::MONSTER, "Monster truck"},
    {car_model_enum::MORGAN, "Morgan"},
    {car_model_enum::MORRIS, "Morris"},
    {car_model_enum::PICKUP, "Pickup"},
    {car_model_enum::RTYPE, "R-type"},
    {car_model_enum::SPIDER, "Spider"},
    {car_model_enum::SPRITE, "Sprite"},
    {car_model_enum::STINGRAY, "Stingray"},
    {car_model_enum::STRATOS, "Stratos"},
    {car_model_enum::STRATOSB, "Stratos B"},
    {car_model_enum::STRIPETB, "Stripe TB"},
    {car_model_enum::STYPE, "S-type"},
    {car_model_enum::STYPECAB, "S-type cab"},
    {car_model_enum::SWATVAN, "SWAT van"},
    {car_model_enum::T2000GT, "T2000 GT"},
    {car_model_enum::TANK, "Tank"},
    {car_model_enum::TAXI, "Taxi"},
    {car_model_enum::TBIRD, "T-bird"},
    {car_model_enum::TOWTRUCK, "Tow truck"},
    {car_model_enum::TRANCEAM, "Trans-Am"},
    {car_model_enum::TRUKCAB1, "Truck cab"},
    {car_model_enum::TRUKCAB2, "Truck cab 2"},
    {car_model_enum::TVVAN, "TV van"},
    {car_model_enum::VAN, "Van"},
    {car_model_enum::VESPA, "Vespa"},
    {car_model_enum::VTYPE, "V-type"},
    {car_model_enum::WBTWIN, "WB twin"},
    {car_model_enum::XK120, "XK 120"},
    {car_model_enum::ZCX5, "ZCX5"},
};
const int kCarCount = static_cast<int>(sizeof(kCars) / sizeof(kCars[0]));
const int kCarRows = 20; // shown at once
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
    gStatus = gWorldDirections ? "Stick: walks the way it points; in cars it steers, RT gas, LT brake"
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

bool Modal()
{
    return gSettingsOpen || gSpawnOpen;
}

// Up or down on the stick or d-pad: -1, 0 or 1, once on the press and then repeating while held
int NavDelta(const plat::PadState& pad, const plat::PadState& prev)
{
    const int dir = StickDirection(pad);
    const int prevDir = StickDirection(prev);
    if (dir == kStickUp || dir == kStickDown)
    {
        gNavHeld = dir == prevDir ? gNavHeld + 1 : 0;
        if (gNavHeld == 0 || (gNavHeld > 20 && gNavHeld % 5 == 0))
        {
            return dir == kStickUp ? -1 : 1;
        }
    }
    else
    {
        gNavHeld = 0;
    }
    return 0;
}

void MoveCar(int delta)
{
    gCar = ((gCar + delta) % kCarCount + kCarCount) % kCarCount;
}

void ToggleSpawnMenu()
{
    gSettingsOpen = false;
    gListening = false;
    gSpawnOpen = !gSpawnOpen;
    gStatus.clear();
}

void ChooseCar()
{
    if (bStartNetworkGame_7081F0)
    {
        gStatus = "Not in a network game: the other players would not see it";
        return;
    }
    gSpawnRequest = kCars[gCar].model;
    gSpawnOpen = false;
}

void ToggleSettings()
{
    if (gSettingsOpen)
    {
        CloseSettings();
        return;
    }
    Refresh();
    gSpawnOpen = false;
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
    Text(x + 8, y + 4 + (kActions + 1) * kCharH, kDim, "F1 hide  F2 settings  Start cars");
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
    Text(x + 8, line, kDim, std::string("M or R3 stick walks as it points, cars: RT/LT: ") + (gWorldDirections ? "on" : "off"));
    line += kCharH;
    Text(x + 8, line, kDim, "Pad buttons by position: A bottom, B right, X left, Y top");
}

void DrawSpawnMenu()
{
    const int w = 360;
    const int h = (kCarRows + 5) * kCharH + 16;
    const int x = (kWidth - w) / 2;
    const int y = (kHeight - h) / 2;
    Fill(x, y, w, h, kPanel);
    Text(x + 8, y + 8, kAccent, "Spawn a car");
    int top = gCar - kCarRows / 2;
    top = top < 0 ? 0 : (top > kCarCount - kCarRows ? kCarCount - kCarRows : top);
    int line = y + 8 + kCharH * 2;
    for (int i = top; i < top + kCarRows && i < kCarCount; i++)
    {
        if (i == gCar)
        {
            Fill(x + 4, line, w - 8, kCharH, kSelect);
        }
        Text(x + 12, line, kText, kCars[i].name);
        line += kCharH;
    }
    line += kCharH / 2;
    Text(x + 8, line, kAccent, gStatus);
    line += kCharH;
    Text(x + 8, line, kDim, "Up/Down choose  LB/RB page  A spawn  B or Start close");
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
    else if (gSpawnOpen)
    {
        MoveCar(NavDelta(pad, prev));
        if (pressed(4))
        {
            MoveCar(-kCarRows);
        }
        else if (pressed(5))
        {
            MoveCar(kCarRows);
        }
        else if (pressed(0))
        {
            ChooseCar();
        }
        else if (pressed(1) || pressed(7))
        {
            gSpawnOpen = false;
        }
    }
    else if (!gSettingsOpen && pressed(7))
    {
        ToggleSpawnMenu();
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
            const int delta = NavDelta(pad, prev);
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

int GasScale()
{
    return gGasScale;
}

bool Captured()
{
    return Modal();
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
    if (dik == DIK_F5)
    {
        if (down)
        {
            ToggleSpawnMenu();
        }
        return true;
    }
    if (gSpawnOpen)
    {
        if (down)
        {
            switch (dik)
            {
                case DIK_UP:
                    MoveCar(-1);
                    break;
                case DIK_DOWN:
                    MoveCar(1);
                    break;
                case DIK_PRIOR:
                    MoveCar(-kCarRows);
                    break;
                case DIK_NEXT:
                    MoveCar(kCarRows);
                    break;
                case DIK_RETURN:
                case DIK_NUMPADENTER:
                    ChooseCar();
                    break;
                case DIK_ESCAPE:
                    gSpawnOpen = false;
                    break;
                default:
                    break;
            }
        }
        return down;
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

static int gLastWanted = 0;

// Puts the chosen car down in front of the player (or of the car the player is in)
void SpawnRequested()
{
    const int model = gSpawnRequest;
    gSpawnRequest = -1;
    Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
    Ped* ped = player ? player->field_2C4_player_ped : nullptr;
    if (!ped || !gCarManager_6C_677930)
    {
        return;
    }
    Fix16 x;
    Fix16 y;
    double ahead = 3.0; // map blocks
    double playerZ = 0.0;
    if (Car_BC* car = ped->get_car_416B60())
    {
        x = car->field_50_car_sprite->GetXPos();
        y = car->field_50_car_sprite->GetYPos();
        playerZ = car->field_50_car_sprite->field_1C_zpos.AsDouble();
        ahead = 4.0;
    }
    else if (ped->field_168_game_object)
    {
        x = ped->field_168_game_object->field_80_sprite_ptr->GetXPos();
        y = ped->field_168_game_object->field_80_sprite_ptr->GetYPos();
        playerZ = ped->field_168_game_object->field_80_sprite_ptr->field_1C_zpos.AsDouble();
    }
    else
    {
        return;
    }
    const Ang16 rotation = ped->GetRotation();
    const double angle = rotation.rValue * 2.0 * M_PI / 1440.0; // as the stick's direction: 0 is down the screen
    // In front if there is open ground there (as high as the player stands), else to either side or behind
    const double kTurns[] = {0, M_PI / 4, -M_PI / 4, M_PI / 2, -M_PI / 2, 3 * M_PI / 4, -3 * M_PI / 4, M_PI};
    Fix16 spawnX = x + Fix16(sin(angle) * ahead);
    Fix16 spawnY = y + Fix16(cos(angle) * ahead);
    bool found = false;
    for (int ring = 0; ring < 2 && !found; ring++)
    {
        for (double turn : kTurns)
        {
            const double dist = ahead + ring * 1.5;
            const Fix16 cx = x + Fix16(sin(angle + turn) * dist);
            const Fix16 cy = y + Fix16(cos(angle + turn) * dist);
            if (gMap_0x370_6F6268->FindGroundZForCoord_4E5B60(cx, cy).AsDouble() <= playerZ + 0.5)
            {
                spawnX = cx;
                spawnY = cy;
                found = true;
                break;
            }
        }
    }
    x = spawnX;
    y = spawnY;
    Car_BC* made = gCarManager_6C_677930->SpawnCarAtCorrectZ_426E40(x, y, rotation, model);
    (void)made;
}

uint32_t SteerWithStick(uint32_t bits)
{
    if (gSpawnRequest >= 0)
    {
        SpawnRequested();
    }
    static bool injected = false;
    static uint32_t injectedClear = 0; // the bits the last injection took over
    static bool scaled = false; // the character's run speed is scaled to the stick
    static Fix16 baseSpeed;
    const uint32_t kMoveBits = 0xF; // Forward, Backward, Left, Right
    plat::PadState pad;
    bool active = false;
    uint32_t mine = 0;
    gGasScale = kFull;
    uint32_t clearBits = kMoveBits;
    if (FirstPad(pad) >= 0 && !gSettingsOpen)
    {
        LoadWorldSetting();
        Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
        Ped* ped = player ? player->field_2C4_player_ped : nullptr;
        const double x = pad.axisX / 32767.0;
        const double y = pad.axisY / 32767.0;
        // The stick's own trigger point (see dinput.cpp's Trigger)
        const double size = sqrt(x * x + y * y);
        if (gWorldDirections && ped && !ped->get_car_416B60() && ped->field_168_game_object && size > 0.25)
        {
            // The game has one walking speed and its ramp up to it; a light push gets a slower one. Not
            // in a network game: the other machines would not know about it.
            Char_B4* body = ped->field_168_game_object;
            if (!scaled)
            {
                baseSpeed = body->field_3C_run_or_jump_speed;
            }
            if (!bStartNetworkGame_7081F0)
            {
                double factor = (size - 0.25) / 0.65;
                factor = factor > 1.0 ? 1.0 : (factor < 0.2 ? 0.2 : factor);
                body->field_3C_run_or_jump_speed = Fix16(static_cast<s32>(baseSpeed.mValue * factor), 0);
                scaled = true;
            }
            // Rotation 0 faces down the screen and grows to the left (found by playing: up walked down),
            // 1440 to a turn
            const double wanted = atan2(x, y) * 1440.0 / (2.0 * M_PI);
            gLastWanted = static_cast<int>(lround(wanted));
            int diff = gLastWanted - ped->GetRotation().rValue;
            diff = ((diff % 1440) + 1440 + 720) % 1440 - 720;
            // Stop pressing a turn once the speed it has built up would carry it there
            // The turn speed is an angle: a speed to the right is just under 1440
            int speed = player->field_8_turn_speed.rValue;
            speed = ((speed + 720) % 1440 + 1440) % 1440 - 720;
            if (speed < 0)
            {
                speed = -speed;
            }
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
        else if (gWorldDirections && ped && ped->get_car_416B60())
        {
            // In a car the stick only steers (the game's steering is left or right, nothing between),
            // and the triggers drive: right is the gas, left the brake and reverse. They are Special 2
            // and 3 on foot, so those two are held back while driving.
            const bool gas = pad.triggerRight > kTriggerDead;
            const bool brake = pad.triggerLeft > kTriggerDead;
            if (size > 0.25 || gas || brake)
            {
                // The trigger's travel sets how hard the engine pulls (CarPhysics_B0::ComputeEngineTorque_561970)
                const int pressed = gas ? pad.triggerRight : pad.triggerLeft;
                double fraction = static_cast<double>(pressed - kTriggerDead) / (32767 - kTriggerDead);
                fraction = fraction > 1.0 ? 1.0 : (fraction < 0.1 ? 0.1 : fraction);
                gGasScale = bStartNetworkGame_7081F0 ? kFull : static_cast<int>(fraction * kFull);
                mine |= gas ? 1 : 0;
                mine |= brake ? 2 : 0;
                mine |= x < -0.25 ? 4 : 0;
                mine |= x > 0.25 ? 8 : 0;
                clearBits |= 0xC00;
                active = true;
            }
        }
    }
    // GTA2_STEERLOG=file: one line per frame the stick steers, for working out why it walks wrong
    static FILE* log = getenv("GTA2_STEERLOG") ? fopen(getenv("GTA2_STEERLOG"), "w") : nullptr;
    if (log && (active || injected))
    {
        Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
        Ped* ped = player ? player->field_2C4_player_ped : nullptr;
        fprintf(log, "stick %6d %6d  rot %4d  turn %4d  wanted %4d  keys %c%c%c  inBefore %x out %x\n", pad.axisX, pad.axisY,
                ped ? ped->GetRotation().rValue : -1, player ? player->field_8_turn_speed.rValue : 0, gLastWanted,
                (mine & 1) ? 'F' : '-', (mine & 4) ? 'L' : '-', (mine & 8) ? 'R' : '-', bits & 0xF, active ? (bits & ~kMoveBits) | mine : bits & ~kMoveBits);
        fflush(log);
    }
    if (!active && scaled)
    {
        Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
        Ped* ped = player ? player->field_2C4_player_ped : nullptr;
        if (ped && ped->field_168_game_object)
        {
            ped->field_168_game_object->field_3C_run_or_jump_speed = baseSpeed;
        }
        scaled = false;
    }
    if (active)
    {
        injected = true;
        injectedClear = clearBits;
        return (bits & ~clearBits) | mine;
    }
    if (injected)
    {
        injected = false;
        return bits & ~injectedClear;
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
    if (havePad || Modal())
    {
        UpdatePad();
    }
    const bool legend = !Modal() && gLegendOn && gLegendLoaded && (havePad || gLegendShown);
    if (!Modal() && !legend)
    {
        return nullptr;
    }
    static int frames = 0;
    if (!Modal() && frames++ % 30 == 0)
    {
        Refresh();
    }
    memset(gCanvas, 0, sizeof(gCanvas));
    if (gSpawnOpen)
    {
        DrawSpawnMenu();
    }
    else if (gSettingsOpen)
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

int port_gas_scale(const void* driverPed)
{
    Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
    return (player && driverPed && driverPed == player->field_2C4_player_ped) ? overlay::GasScale() : 16384;
}
