#include "overlay.h"
#include "overlay_font.h"

#include "controls.h"

#include "Car_BC.hpp"
#include "Frontend.hpp"
#include "Game_0x40.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "registry.hpp"
#include "winmain.hpp"

#include <dinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace overlay
{
namespace
{
using padinput::kButtonSlots;
using padinput::kNone;
using padinput::kStickDown;
using padinput::kStickLeft;
using padinput::kStickRight;
using padinput::kStickUp;

const int kWidth = 640;
const int kHeight = 480;
const int kCharW = 8;
const int kCharH = 16;

// What each control does, from Player::CharacterControls_566520
const char* const kActionNames[kActions] = {"Forward",
                                            "Backward",
                                            "Left",
                                            "Right",
                                            "Fire",
                                            "Enter/exit car",
                                            "Handbrake/jump",
                                            "Prev weapon",
                                            "Next weapon",
                                            "Special 1 (horn)",
                                            "Special 2 (camera)",
                                            "Special 3"};
const char* const kContextNames[2] = {"On foot", "In a car"};

const uint32_t kPanel = 0xC0101820;
const uint32_t kText = 0xFFFFFFFF;
const uint32_t kDim = 0xFF98A0B0;
const uint32_t kAccent = 0xFFFFD040;
const uint32_t kSelect = 0xFF2848A0;

uint32_t gCanvas[kWidth * kHeight];

bool gSettingsOpen = false;
bool gSpawnOpen = false; // the car menu
bool gMenuOpen = false; // the Esc menu
bool gSettingsFromMenu = false; // closing the settings goes back to the Esc menu
const SettingsPage* gPage = nullptr; // another part's settings (AddSettingsPage), or none
bool gGraphicsOpen = false; // that page is open
bool gGraphicsFromMenu = false; // closing it goes back to the Esc menu
int gGraphicsRow = 0;
int gMenuRow = 0;
bool gAudioPaused = false;

// The Esc menu's entries. The page's entry (its title) only when a page was added.
enum MenuEntry
{
    MenuSettings,
    MenuGraphics,
    MenuQuit,
    kMenuEntries
};
const char* const kMenuNames[kMenuEntries] = {"Settings", "", "Quit game"};

bool HasGraphics()
{
    return gPage && gPage->rows() > 0;
}

const char* MenuName(MenuEntry entry)
{
    return entry == MenuGraphics ? gPage->title : kMenuNames[entry];
}

// The entries shown, in order
int MenuCount()
{
    return HasGraphics() ? kMenuEntries : kMenuEntries - 1;
}

MenuEntry MenuAt(int row)
{
    return (!HasGraphics() && row >= MenuGraphics) ? static_cast<MenuEntry>(row + 1) : static_cast<MenuEntry>(row);
}
int gTab = padinput::OnFoot; // the pad layout the settings screen shows
int gCar = 0; // the car menu's selected entry

// The cars the menu offers (car_model_enum): not trains, trailers, wrecks or unused slots
struct CarEntry
{
    int model;
    const char* name;
};
const CarEntry kCars[] = {
    {CarModel::kAlfa, "Alfa"},
    {CarModel::kAllard, "Allard"},
    {CarModel::kAmdb4, "Aston Martin DB4"},
    {CarModel::kApc, "APC"},
    {CarModel::kBankVan, "Bank van"},
    {CarModel::kBmw, "BMW"},
    {CarModel::kBoxtruck, "Box truck"},
    {CarModel::kBug, "Bug"},
    {CarModel::kBuick, "Buick"},
    {CarModel::kBus, "Bus"},
    {CarModel::kCopcar, "Police car"},
    {CarModel::kDart, "Dart"},
    {CarModel::kEdsel, "Edsel"},
    {CarModel::kEdselfbi, "Edsel (FBI)"},
    {CarModel::kFiat, "Fiat"},
    {CarModel::kFiretruk, "Fire truck"},
    {CarModel::kGraham, "Graham"},
    {CarModel::kGt24640, "GT 24640"},
    {CarModel::kGtruck, "G-truck"},
    {CarModel::kGunjeep, "Gun jeep"},
    {CarModel::kHotdog, "Hot dog van"},
    {CarModel::kIcecream, "Ice cream van"},
    {CarModel::kIsetlimo, "Isetta limo"},
    {CarModel::kIsetta, "Isetta"},
    {CarModel::kJeep, "Jeep"},
    {CarModel::kJeffrey, "Jeffrey"},
    {CarModel::kKrsnabus, "Krishna bus"},
    {CarModel::kLimo, "Limo"},
    {CarModel::kLimo2, "Limo 2"},
    {CarModel::kMedicar, "Ambulance"},
    {CarModel::kMerc, "Merc"},
    {CarModel::kMesser, "Messerschmitt"},
    {CarModel::kMiura, "Miura"},
    {CarModel::kMonster, "Monster truck"},
    {CarModel::kMorgan, "Morgan"},
    {CarModel::kMorris, "Morris"},
    {CarModel::kPickup, "Pickup"},
    {CarModel::kRtype, "R-type"},
    {CarModel::kSpider, "Spider"},
    {CarModel::kSprite, "Sprite"},
    {CarModel::kStingray, "Stingray"},
    {CarModel::kStratos, "Stratos"},
    {CarModel::kStratosb, "Stratos B"},
    {CarModel::kStripetb, "Stripe TB"},
    {CarModel::kStype, "S-type"},
    {CarModel::kStypecab, "S-type cab"},
    {CarModel::kSwatvan, "SWAT van"},
    {CarModel::kT2000gt, "T2000 GT"},
    {CarModel::kTank, "Tank"},
    {CarModel::kTaxi, "Taxi"},
    {CarModel::kTbird, "T-bird"},
    {CarModel::kTowtruck, "Tow truck"},
    {CarModel::kTranceam, "Trans-Am"},
    {CarModel::kTrukcab1, "Truck cab"},
    {CarModel::kTrukcab2, "Truck cab 2"},
    {CarModel::kTvvan, "TV van"},
    {CarModel::kVan, "Van"},
    {CarModel::kVespa, "Vespa"},
    {CarModel::kVtype, "V-type"},
    {CarModel::kWbtwin, "WB twin"},
    {CarModel::kXk120, "XK 120"},
    {CarModel::kZcx5, "ZCX5"},
};
const int kCarCount = static_cast<int>(sizeof(kCars) / sizeof(kCars[0]));
const int kCarRows = 20; // shown at once
bool gLegendShown = false; // F1 pressed this run: show it even without a pad
int gRow = 0;
bool gListening = false;
int gListenFrames = 0;
std::string gStatus;

plat::PadState gPrevPad;
bool gHavePrev = false;
int gNavHeld = 0; // frames the stick or d-pad has been held up or down

bool InGame()
{
    return gGame_0x40_67E008 && !gFrontend_67DC84;
}

// Esc (Start on the pad) opens the port's menu instead of the game's own quit question. Not in a
// network game, which can't stand still. Off in the settings (or esc_menu = 0 in pad.txt) gives
// the original behaviour back.
bool EscOpensMenu()
{
    return controls::Pad().escMenu && InGame() && !bStartNetworkGame_7081F0;
}

// ---------------------------------------------------------------- the controls

// The keyboard's binding of an action is the game's own (its options menu edits the same setting)
int KeyOf(int action)
{
    return static_cast<int>(gPlayerControlsBinding_67B6E8[action] & 0xFF);
}

void SetKey(int action, int dik)
{
    gMaybeDeviceType_67B91C[action] = 0;
    gPlayerControlsBinding_67B6E8[action] = static_cast<uint32_t>(dik);
    gRegistry_6FF968.Write_Control_Setting(action, dik);
}

// Gives `action` the key. Another control that had it takes the old one, so none is left dead.
void AssignKey(int action, int dik)
{
    const int old = KeyOf(action);
    for (int other = 0; other < kActions; other++)
    {
        if (other != action && KeyOf(other) == dik)
        {
            SetKey(other, old);
        }
    }
    SetKey(action, dik);
}

// The same for a pad source in one of the two pad layouts
void AssignPad(int tab, int action, int source)
{
    padinput::Layout& layout = controls::Pad().layout[tab];
    const int old = layout.source[action];
    for (int other = 0; other < kActions; other++)
    {
        if (other != action && source != kNone && layout.source[other] == source)
        {
            layout.source[other] = old;
        }
    }
    layout.source[action] = source;
    controls::SavePad();
}

void ResetRow(int tab, int action)
{
    AssignKey(action, gDefaultControls_61A9E4[action] & 0xFF);
    AssignPad(tab, action, padinput::DefaultLayout(static_cast<padinput::Context>(tab)).source[action]);
    gStatus = std::string(kActionNames[action]) + ": back to " + KeyName(KeyOf(action)) + " and " +
        BindingLabel(1, controls::Pad().layout[tab].source[action]);
}

void ApplyPadDefaults()
{
    const padinput::Settings defaults = padinput::Defaults();
    controls::Pad().layout[padinput::OnFoot] = defaults.layout[padinput::OnFoot];
    controls::Pad().layout[padinput::InCar] = defaults.layout[padinput::InCar];
    controls::SavePad();
    gStatus = "Pad layouts back to the defaults, for walking and for driving";
}

void ApplyKeyboardDefaults()
{
    for (int i = 0; i < kActions; i++)
    {
        SetKey(i, gDefaultControls_61A9E4[i] & 0xFF);
    }
    gStatus = "Keyboard back to the game's default keys";
}

void ToggleWorldDirections()
{
    padinput::Settings& pad = controls::Pad();
    pad.worldDirections = !pad.worldDirections;
    controls::SavePad();
    gStatus = pad.worldDirections ? "Stick: walks the way it points" : "Stick: left and right turn the character, up walks forward";
}

void SwitchTab(int delta)
{
    gTab = (gTab + delta + 2) % 2;
    gStatus.clear();
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
    gStatus = std::string("Press a key or a pad button for ") + kActionNames[gRow];
}

void CloseSettings()
{
    gSettingsOpen = false;
    gListening = false;
    gStatus.clear();
    gMenuOpen = gSettingsFromMenu && EscOpensMenu();
    gSettingsFromMenu = false;
}

bool Modal()
{
    return gSettingsOpen || gSpawnOpen || gMenuOpen || gGraphicsOpen;
}

void ToggleEscMenuSetting()
{
    padinput::Settings& pad = controls::Pad();
    pad.escMenu = !pad.escMenu;
    controls::SavePad();
    gStatus = pad.escMenu ? "Esc and Start open this menu" : "Esc asks the game's own quit question";
}

// Up or down on the stick or d-pad: -1, 0 or 1, once on the press and then repeating while held
int NavDelta(const plat::PadState& pad, const plat::PadState& prev)
{
    const int dir = padinput::StickDirection(pad);
    const int prevDir = padinput::StickDirection(prev);
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
    if (!InGame())
    {
        return;
    }
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
    controls::RequestSpawn(kCars[gCar].model);
    gSpawnOpen = false;
}

void ToggleSettings()
{
    if (gSettingsOpen)
    {
        CloseSettings();
        return;
    }
    gTab = InGame() ? controls::CurrentContext() : padinput::OnFoot;
    gSpawnOpen = false;
    gGraphicsOpen = false;
    gSettingsFromMenu = gMenuOpen;
    gMenuOpen = false;
    gSettingsOpen = true;
    gListening = false;
    gStatus.clear();
}

void ToggleMenu()
{
    gMenuOpen = !gMenuOpen;
    gMenuRow = 0;
    gSpawnOpen = false;
}

// The added page: rows, each with values to step through
void OpenGraphics()
{
    if (!HasGraphics())
    {
        return;
    }
    gGraphicsFromMenu = gMenuOpen;
    gMenuOpen = false;
    gSettingsOpen = false;
    gListening = false;
    gSpawnOpen = false;
    gGraphicsOpen = true;
    gGraphicsRow = 0;
}

void CloseGraphics()
{
    gGraphicsOpen = false;
    gMenuOpen = gGraphicsFromMenu && EscOpensMenu();
    gGraphicsFromMenu = false;
}

void MoveGraphics(int delta)
{
    const int rows = gPage->rows();
    gGraphicsRow = rows ? (gGraphicsRow + delta + rows) % rows : 0;
}

void ChangeGraphics(int delta)
{
    gPage->change(gGraphicsRow, delta);
}

void MoveMenu(int delta)
{
    gMenuRow = (gMenuRow + delta + MenuCount()) % MenuCount();
}

void ChooseMenu()
{
    switch (MenuAt(gMenuRow))
    {
        case MenuSettings:
            ToggleSettings();
            break;
        case MenuGraphics:
            OpenGraphics();
            break;
        case MenuQuit:
            // As confirming the game's own quit question (Hud_QuitMessage_1::IsOnQuitMessage_5D13C0)
            gMenuOpen = false;
            if (InGame())
            {
                gGame_0x40_67E008->ExitGameNoBonus_4B8C00(1, GameExitType::PlayerQuit_2);
            }
            break;
        default:
            break;
    }
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
void DrawRow(int x, int y, int tab, int action, bool selected, int panelW)
{
    if (selected)
    {
        Fill(x - 4, y, panelW - 8, kCharH, kSelect);
    }
    const int source = controls::Pad().layout[tab].source[action];
    Text(x, y, kText, Pad(kActionNames[action], 19));
    Text(x + 19 * kCharW, y, kDim, Pad(KeyName(KeyOf(action)), 12));
    Text(x + 31 * kCharW, y, source == kNone ? kDim : kAccent, BindingLabel(1, source));
}

void DrawLegend()
{
    const int context = controls::CurrentContext();
    const int w = 44 * kCharW + 16;
    const int h = (kActions + 3) * kCharH + 8;
    const int x = kWidth - w - 8;
    const int y = 8;
    Fill(x, y, w, h, kPanel);
    Text(x + 8, y + 4, kText, std::string("Controls: ") + kContextNames[context]);
    for (int i = 0; i < kActions; i++)
    {
        DrawRow(x + 8, y + 4 + (i + 1) * kCharH, context, i, false, w);
    }
    Text(x + 8, y + 4 + (kActions + 1) * kCharH, kDim, "F1 hide  F2 settings  Back or F5 cars");
}

void DrawSettings()
{
    const int w = 600;
    const int h = (kActions + 14) * kCharH + 16;
    const int x = (kWidth - w) / 2;
    const int y = (kHeight - h) / 2;
    Fill(x, y, w, h, kPanel);
    int line = y + 8;
    Text(x + 8, line, kAccent, "Controls");
    line += kCharH;
    plat::PadState pad;
    const int index = controls::FirstPad(pad);
    Text(x + 8, line, kDim, index >= 0 ? std::string("Pad: ") + plat::PadName(index) : "No controller connected");
    line += kCharH * 2;
    Text(x + 8, line - kCharH, kText, std::string("<  ") + kContextNames[gTab] + "  >   (LB/RB or Left/Right)");
    Text(x + 8, line, kDim, Pad("", 19) + Pad("Key", 12) + "Pad");
    line += kCharH;
    for (int i = 0; i < kActions; i++)
    {
        DrawRow(x + 8, line, gTab, i, i == gRow, w - 8);
        line += kCharH;
    }
    line += kCharH;
    Text(x + 8, line, kAccent, gStatus);
    line += kCharH * 2;
    Text(x + 8, line, kDim, "Up/Down select  Enter or A rebind (a key or a pad button)");
    line += kCharH;
    Text(x + 8, line, kDim, "Del or X reset the row  N or L3 unbind the pad  Esc, B or F2 close");
    line += kCharH;
    Text(x + 8, line, kDim, "P or Y pad defaults   K keyboard defaults   The keys are the same");
    line += kCharH;
    Text(x + 8, line, kDim, "on foot and in a car, as in the game's own options menu.");
    line += kCharH;
    Text(x + 8, line, kDim, std::string("M or R3 stick walks the way it points: ") + (controls::Pad().worldDirections ? "on" : "off"));
    line += kCharH;
    Text(x + 8, line, kDim, std::string("E or Back  Esc and Start open the pause menu: ") + (controls::Pad().escMenu ? "on" : "off"));
    if (HasGraphics())
    {
        line += kCharH;
        Text(x + 8, line, kDim, std::string("G ") + gPage->title);
    }
}

void DrawMenu()
{
    const int w = 280;
    const int h = (MenuCount() + 5) * kCharH + 16;
    const int x = (kWidth - w) / 2;
    const int y = (kHeight - h) / 2;
    Fill(x, y, w, h, kPanel);
    Text(x + 8, y + 8, kAccent, "Paused");
    int line = y + 8 + kCharH * 2;
    for (int i = 0; i < MenuCount(); i++)
    {
        if (i == gMenuRow)
        {
            Fill(x + 4, line, w - 8, kCharH, kSelect);
        }
        Text(x + 12, line, kText, MenuName(MenuAt(i)));
        line += kCharH;
    }
    line += kCharH;
    Text(x + 8, line, kDim, "Enter or A choose");
    line += kCharH;
    Text(x + 8, line, kDim, "Esc, B or Start back to the game");
}

void DrawGraphics()
{
    const int rows = gPage->rows();
    const int w = 520;
    const int h = (rows + 6) * kCharH + 16;
    const int x = (kWidth - w) / 2;
    const int y = (kHeight - h) / 2;
    Fill(x, y, w, h, kPanel);
    Text(x + 8, y + 8, kAccent, gPage->title);
    int line = y + 8 + kCharH * 2;
    for (int i = 0; i < rows; i++)
    {
        if (i == gGraphicsRow)
        {
            Fill(x + 4, line, w - 8, kCharH, kSelect);
        }
        Text(x + 12, line, kText, Pad(gPage->name(i), 14));
        Text(x + 12 + 14 * kCharW, line, kAccent, gPage->value(i));
        line += kCharH;
    }
    line += kCharH;
    Text(x + 8, line, kDim, "Up/Down select  Left/Right, Enter or A change");
    line += kCharH;
    Text(x + 8, line, kDim, std::string("Esc or B back   Saved in ") + gPage->file);
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
    Text(x + 8, line, kDim, "Up/Down choose  LB/RB page  A spawn  B or Back close");
}

// ---------------------------------------------------------------- pad navigation

void UpdatePad()
{
    plat::PadState pad;
    if (controls::FirstPad(pad) < 0)
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
    auto pressed = [&](int slot)
    {
        return pad.buttons[slot] && !prev.buttons[slot];
    };

    const bool guide = pad.guide && !prev.guide;
    if (guide)
    {
        ToggleSettings();
    }
    else if (gGraphicsOpen)
    {
        MoveGraphics(NavDelta(pad, prev));
        const int dir = padinput::StickDirection(pad);
        const bool sideways = dir != padinput::StickDirection(prev) && (dir == kStickLeft || dir == kStickRight);
        if (pressed(0) || sideways)
        {
            ChangeGraphics(dir == kStickLeft ? -1 : 1);
        }
        else if (pressed(1) || pressed(7))
        {
            CloseGraphics();
        }
    }
    else if (gMenuOpen)
    {
        MoveMenu(NavDelta(pad, prev));
        if (pressed(0))
        {
            ChooseMenu();
        }
        else if (pressed(1) || pressed(7))
        {
            gMenuOpen = false;
        }
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
        else if (pressed(1) || pressed(6))
        {
            gSpawnOpen = false;
        }
    }
    else if (!gSettingsOpen && pressed(7) && EscOpensMenu())
    {
        ToggleMenu();
    }
    else if (!gSettingsOpen && (pressed(6) || (pressed(7) && !EscOpensMenu())))
    {
        ToggleSpawnMenu();
    }
    else if (gSettingsOpen)
    {
        if (gListening)
        {
            int source = 0;
            if (CapturePadBinding(prev, pad, source))
            {
                AssignPad(gTab, gRow, source);
                gStatus = std::string(kActionNames[gRow]) + ": " + BindingLabel(1, source) + " (" + kContextNames[gTab] + ")";
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
                ResetRow(gTab, gRow);
            }
            else if (pressed(3))
            {
                ApplyPadDefaults();
            }
            else if (pressed(9))
            {
                ToggleWorldDirections();
            }
            else if (pressed(6))
            {
                ToggleEscMenuSetting();
            }
            else if (pressed(8))
            {
                AssignPad(gTab, gRow, kNone);
                gStatus = std::string(kActionNames[gRow]) + ": no pad input";
            }
            else if (pressed(4))
            {
                SwitchTab(-1);
            }
            else if (pressed(5))
            {
                SwitchTab(1);
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
        {DIK_ESCAPE, "Esc"},     {DIK_MINUS, "-"},
        {DIK_EQUALS, "="},       {DIK_BACK, "Backspace"},
        {DIK_TAB, "Tab"},        {DIK_RETURN, "Enter"},
        {DIK_LCONTROL, "Ctrl"},  {DIK_RCONTROL, "Right Ctrl"},
        {DIK_LSHIFT, "Shift"},   {DIK_RSHIFT, "Right Shift"},
        {DIK_LMENU, "Alt"},      {DIK_RMENU, "Right Alt"},
        {DIK_SPACE, "Space"},    {DIK_UP, "Up"},
        {DIK_DOWN, "Down"},      {DIK_LEFT, "Left"},
        {DIK_RIGHT, "Right"},    {DIK_INSERT, "Insert"},
        {DIK_DELETE, "Delete"},  {DIK_HOME, "Home"},
        {DIK_END, "End"},        {DIK_PRIOR, "Page Up"},
        {DIK_NEXT, "Page Down"}, {DIK_CAPITAL, "Caps Lock"},
        {DIK_COMMA, ","},        {DIK_PERIOD, "."},
        {DIK_SLASH, "/"},        {DIK_SEMICOLON, ";"},
        {DIK_APOSTROPHE, "'"},   {DIK_LBRACKET, "["},
        {DIK_RBRACKET, "]"},     {DIK_BACKSLASH, "\\"},
        {DIK_GRAVE, "`"},        {DIK_NUMPADENTER, "Num Enter"},
        {DIK_ADD, "Num +"},      {DIK_SUBTRACT, "Num -"},
        {DIK_MULTIPLY, "Num *"}, {DIK_DIVIDE, "Num /"},
        {DIK_DECIMAL, "Num ."},
    };
    for (const Named& n : kNames)
    {
        if (n.dik == dik)
        {
            return n.name;
        }
    }
    static const int kLetters[] = {DIK_A, DIK_B, DIK_C, DIK_D, DIK_E, DIK_F, DIK_G, DIK_H, DIK_I, DIK_J, DIK_K, DIK_L, DIK_M,
                                   DIK_N, DIK_O, DIK_P, DIK_Q, DIK_R, DIK_S, DIK_T, DIK_U, DIK_V, DIK_W, DIK_X, DIK_Y, DIK_Z};
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
    static const int kFunction[] = {DIK_F1, DIK_F2, DIK_F3, DIK_F4, DIK_F5, DIK_F6, DIK_F7, DIK_F8, DIK_F9, DIK_F10, DIK_F11, DIK_F12};
    for (int i = 0; i < 12; i++)
    {
        if (kFunction[i] == dik)
        {
            return "F" + std::to_string(i + 1);
        }
    }
    static const int kNumpad[] =
        {DIK_NUMPAD0, DIK_NUMPAD1, DIK_NUMPAD2, DIK_NUMPAD3, DIK_NUMPAD4, DIK_NUMPAD5, DIK_NUMPAD6, DIK_NUMPAD7, DIK_NUMPAD8, DIK_NUMPAD9};
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
        case kNone:
            return "None";
        case kStickLeft:
            return "Stick Left";
        case kStickRight:
            return "Stick Right";
        case kStickUp:
            return "Stick Up";
        case kStickDown:
            return "Stick Down";
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
    const int dir = padinput::StickDirection(now);
    if (dir && dir != padinput::StickDirection(prev))
    {
        binding = dir;
        return true;
    }
    return false;
}

void AddSettingsPage(const SettingsPage* page)
{
    gPage = page;
}

bool Captured()
{
    return Modal();
}

bool FreezesGame()
{
    const bool frozen = Modal() && InGame() && !bStartNetworkGame_7081F0;
    if (frozen != gAudioPaused)
    {
        gAudioPaused = frozen;
        plat::PauseAudio(frozen);
    }
    return frozen;
}

bool OnKey(uint8_t dik, bool down)
{
    if (dik == DIK_F1 || dik == DIK_F2)
    {
        if (down)
        {
            if (dik == DIK_F1)
            {
                // Without a pad the legend is hidden, so the first press shows it
                plat::PadState pad;
                padinput::Settings& settings = controls::Pad();
                const bool visible = settings.legend && (controls::FirstPad(pad) >= 0 || gLegendShown);
                gLegendShown = true;
                settings.legend = !visible;
                controls::SavePad();
            }
            else
            {
                ToggleSettings();
            }
        }
        return true;
    }
    if (gGraphicsOpen)
    {
        if (down)
        {
            switch (dik)
            {
                case DIK_UP:
                    MoveGraphics(-1);
                    break;
                case DIK_DOWN:
                    MoveGraphics(1);
                    break;
                case DIK_LEFT:
                    ChangeGraphics(-1);
                    break;
                case DIK_RIGHT:
                case DIK_RETURN:
                case DIK_NUMPADENTER:
                    ChangeGraphics(1);
                    break;
                case DIK_ESCAPE:
                    CloseGraphics();
                    break;
                default:
                    break;
            }
        }
        return down;
    }
    if (gMenuOpen)
    {
        if (down)
        {
            switch (dik)
            {
                case DIK_UP:
                    MoveMenu(-1);
                    break;
                case DIK_DOWN:
                    MoveMenu(1);
                    break;
                case DIK_RETURN:
                case DIK_NUMPADENTER:
                    ChooseMenu();
                    break;
                case DIK_ESCAPE:
                    gMenuOpen = false;
                    break;
                default:
                    break;
            }
        }
        return down;
    }
    if (dik == DIK_ESCAPE && !gSettingsOpen && !gSpawnOpen && EscOpensMenu())
    {
        if (down)
        {
            ToggleMenu();
        }
        return down;
    }
    if (dik == DIK_F5 && InGame())
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
            AssignKey(gRow, dik);
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
            ResetRow(gTab, gRow);
            break;
        case DIK_N:
            AssignPad(gTab, gRow, kNone);
            gStatus = std::string(kActionNames[gRow]) + ": no pad input";
            break;
        case DIK_LEFT:
            SwitchTab(-1);
            break;
        case DIK_RIGHT:
            SwitchTab(1);
            break;
        case DIK_P:
            ApplyPadDefaults();
            break;
        case DIK_K:
            ApplyKeyboardDefaults();
            break;
        case DIK_M:
            ToggleWorldDirections();
            break;
        case DIK_E:
            ToggleEscMenuSetting();
            break;
        case DIK_G:
            OpenGraphics();
            break;
        case DIK_ESCAPE:
            CloseSettings();
            break;
        default:
            break;
    }
    return true;
}

const uint32_t* Frame()
{
    plat::PadState pad;
    const bool havePad = controls::FirstPad(pad) >= 0;
    if (!InGame())
    {
        gSpawnOpen = false;
        gMenuOpen = false;
        gGraphicsFromMenu = false;
    }
    FreezesGame(); // resumes the sound when a menu closed outside a game frame (quitting)
    if (havePad || Modal())
    {
        UpdatePad();
    }
    const bool legend = !Modal() && InGame() && controls::Pad().legend && (havePad || gLegendShown);
    if (!Modal() && !legend)
    {
        return nullptr;
    }
    memset(gCanvas, 0, sizeof(gCanvas));
    if (gSpawnOpen)
    {
        DrawSpawnMenu();
    }
    else if (gMenuOpen)
    {
        DrawMenu();
    }
    else if (gSettingsOpen)
    {
        DrawSettings();
    }
    else if (gGraphicsOpen)
    {
        DrawGraphics();
    }
    else
    {
        DrawLegend();
    }
    return gCanvas;
}
} // namespace overlay
