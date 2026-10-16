// The game pad, read for the game: which pad input drives which of the game's twelve controls, and
// the analog part (walking the way the stick points, walking speed, gas pedal, turret).
//
// Plain C++ with no game types, so the unit tests can drive it with made-up pad states. How the input
// reaches the game and its replays is in platform/controls.cpp; the settings screen is overlay.cpp.
//
// The keyboard stays the game's own (its bindings are the registry's control settings, edited by the
// game's options menu). The pad has a layout of its own for walking and one for driving, kept in
// $XDG_CONFIG_HOME/gta2-native/pad.txt. Both work at the same time: a control is on when its key or
// its pad input is.
#pragma once
#include "platform.h"

#include <stdint.h>
#include <string>

namespace padinput
{
const int kActions = 12; // bit i of the game's control bits is action i

// A pad source: a button slot (0..11, as plat::PadButtonName names them; 10 and 11 are the
// triggers), or a direction of the left stick or the d-pad, or nothing
const int kStickLeft = 224;
const int kStickRight = 225;
const int kStickUp = 226;
const int kStickDown = 227;
const int kNone = 255;
const int kButtonSlots = 12;

enum Context
{
    OnFoot = 0,
    InCar = 1,
};

struct Layout
{
    int source[kActions];
};

struct Settings
{
    Layout layout[2];
    bool worldDirections = true; // on foot, the stick walks the way it points
    bool legend = true; // the overlay's binding legend
};

Settings Defaults();
const Layout& DefaultLayout(Context context);

// $XDG_CONFIG_HOME/gta2-native/pad.txt (or ~/.config/...)
std::string SettingsFile();
// Unknown lines and bad values are skipped, so a damaged file gives the defaults for what it lacks
Settings Load(const std::string& file);
// Written to a temporary file and renamed over the old one, so it is never left half written
bool Save(const std::string& file, const Settings& settings);

// One update's pad input in the form a replay records it. Every field is a small integer, so what
// the game computes from it is the same live and in a replay.
struct Analog
{
    int heading = -1; // on foot with world directions: where the stick points, 0..127 (0 is down the
                      // screen, growing to the left as the game's rotations do), -1 when not steering
    int walk = 7; // with a heading: how far the stick is pushed, 0 (a light push) to 7 (all the way)
    int gas = 0; // in a car: how far the pedal trigger is pressed, 1..15, or 0 for "the game's own"
    int turret = 0; // in a car: the right stick turns the turret, 1 left, 2 right

    bool operator==(const Analog&) const = default;
};

// Analog into 17 bits and back
uint32_t Pack(const Analog& analog);
Analog Unpack(uint32_t bits);

struct Frame
{
    uint32_t bits = 0; // the controls the pad holds (bit i is action i)
    Analog analog;
};

// Reads the pad once per game update. It keeps what it needs between updates: the stick's
// hysteresis, and the inputs to ignore until they are let go.
class Reader
{
  public:
    // `analogAllowed` is false in network games: the other machines would not know the speed, gas
    // or turret (the heading is fine, it turns into the control bits the game sends).
    // `blocked`: a menu has the pad, so the game sees it at rest; what is held when it closes is
    // ignored until let go.
    Frame Read(const plat::PadState* pad, Context context, const Settings& settings, bool analogAllowed, bool blocked);

    // Ignore everything held now until it is let go (the layout changed, a menu closed)
    void Latch(const plat::PadState& pad);

  private:
    bool Down(const plat::PadState& pad, int source) const;
    bool Held(const plat::PadState& pad, int source) const;

    int mStick[2] = {0, 0}; // -1, 0, 1 per axis, with hysteresis
    bool mLatched[256] = {};
    bool mHaveContext = false;
    Context mContext = OnFoot;
    bool mWasBlocked = false;
};

// Where the stick sits as a source (kStickLeft..kStickDown), or 0 inside the dead zone. For menus.
int StickDirection(const plat::PadState& pad);
} // namespace padinput
