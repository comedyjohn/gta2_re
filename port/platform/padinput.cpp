#include "padinput.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

namespace padinput
{
namespace
{
// Walking: the stick and d-pad move, the face buttons act (slots as plat::PadButtonName names them)
const Layout kFootDefault = {{kStickUp, kStickDown, kStickLeft, kStickRight, 2 /* X: fire */, 3 /* Y: enter car */,
                              0 /* A: jump */, 4 /* LB: previous weapon */, 5 /* RB: next weapon */, 1 /* B: special 1 */,
                              10 /* LT */, 11 /* RT */}};
// Driving: the triggers are the pedals (analog), the stick steers, RB shoots (a tank's gun too)
const Layout kCarDefault = {{11 /* RT: gas */, 10 /* LT: brake, reverse */, kStickLeft, kStickRight, 5 /* RB: fire */,
                             3 /* Y: leave the car */, 0 /* A: handbrake */, 4 /* LB: previous weapon */, kNone,
                             1 /* B: horn */, 2 /* X: special 2 */, kNone}};

const char* const kContextKeys[2] = {"foot", "car"};

const int32_t kStickOn = 32767 * 40 / 100; // a stick direction counts as pressed past 40% of its travel
const int32_t kStickOff = 32767 * 25 / 100; // ... and until it is back under 25%
const double kSteerDead = 0.25; // the stick walks once pushed this far
const double kSteerFull = 0.90; // ... and at full speed from here
const int32_t kTriggerDead = 2600; // of 32767: a pedal trigger acts past this
const double kTurretDead = 0.35; // the right stick turns a turret past this

bool ValidSource(int source)
{
    return (source >= 0 && source < kButtonSlots) || (source >= kStickLeft && source <= kStickDown) || source == kNone;
}

bool IsTrigger(int source)
{
    return source == 10 || source == 11;
}

int32_t TriggerTravel(const plat::PadState& pad, int source)
{
    return source == 11 ? pad.triggerRight : pad.triggerLeft;
}

// -1, 0 or 1 for an axis, with the hysteresis that keeps a stick near the threshold from flickering
int Hysteresis(int previous, int32_t value)
{
    const int sign = value < 0 ? -1 : 1;
    const int32_t size = value < 0 ? -value : value;
    if (previous != 0 && (previous != sign || size < kStickOff))
    {
        previous = 0;
    }
    if (previous == 0 && size > kStickOn)
    {
        previous = sign;
    }
    return previous;
}
} // namespace

Settings Defaults()
{
    Settings s;
    s.layout[OnFoot] = kFootDefault;
    s.layout[InCar] = kCarDefault;
    return s;
}

const Layout& DefaultLayout(Context context)
{
    return context == InCar ? kCarDefault : kFootDefault;
}

std::string SettingsFile()
{
    const char* base = getenv("XDG_CONFIG_HOME");
    std::string dir = base && *base ? base : std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.config";
    dir += "/gta2-native";
    mkdir(dir.c_str(), 0755);
    return dir + "/pad.txt";
}

Settings Load(const std::string& file)
{
    Settings s = Defaults();
    FILE* f = fopen(file.c_str(), "r");
    if (!f)
    {
        return s;
    }
    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        char key[64];
        int value = 0;
        if (line[0] == '#' || sscanf(line, " %63[^= ] = %d", key, &value) != 2)
        {
            continue;
        }
        if (!strcmp(key, "world_directions"))
        {
            s.worldDirections = value != 0;
        }
        else if (!strcmp(key, "legend"))
        {
            s.legend = value != 0;
        }
        else
        {
            for (int context = 0; context < 2; context++)
            {
                const size_t n = strlen(kContextKeys[context]);
                int action = -1;
                if (!strncmp(key, kContextKeys[context], n) && key[n] == '.' && sscanf(key + n + 1, "%d", &action) == 1 &&
                    action >= 0 && action < kActions && ValidSource(value))
                {
                    s.layout[context].source[action] = value;
                }
            }
        }
    }
    fclose(f);
    return s;
}

bool Save(const std::string& file, const Settings& settings)
{
    const std::string temp = file + ".new";
    FILE* f = fopen(temp.c_str(), "w");
    if (!f)
    {
        return false;
    }
    fprintf(f, "# GTA2 pad settings. Sources: 0..11 the buttons A B X Y LB RB Back Start L3 R3 LT RT,\n"
               "# 224..227 stick or d-pad left, right, up, down, 255 none. Actions: 0 forward, 1 backward,\n"
               "# 2 left, 3 right, 4 fire, 5 enter/exit car, 6 jump/handbrake, 7 previous weapon,\n"
               "# 8 next weapon, 9..11 specials.\n");
    fprintf(f, "world_directions = %d\nlegend = %d\n", settings.worldDirections ? 1 : 0, settings.legend ? 1 : 0);
    for (int context = 0; context < 2; context++)
    {
        for (int action = 0; action < kActions; action++)
        {
            fprintf(f, "%s.%d = %d\n", kContextKeys[context], action, settings.layout[context].source[action]);
        }
    }
    const bool ok = fflush(f) == 0 && !ferror(f);
    fclose(f);
    if (!ok || rename(temp.c_str(), file.c_str()) != 0)
    {
        remove(temp.c_str());
        return false;
    }
    return true;
}

uint32_t Pack(const Analog& a)
{
    return static_cast<uint32_t>(a.heading + 1) | (static_cast<uint32_t>(a.walk) << 8) | (static_cast<uint32_t>(a.gas) << 11) |
        (static_cast<uint32_t>(a.turret) << 15);
}

Analog Unpack(uint32_t bits)
{
    Analog a;
    a.heading = static_cast<int>(bits & 0xFF) - 1;
    a.walk = static_cast<int>((bits >> 8) & 7);
    a.gas = static_cast<int>((bits >> 11) & 15);
    a.turret = static_cast<int>((bits >> 15) & 3);
    if (a.heading > 127)
    {
        a.heading = -1;
    }
    if (a.turret == 3)
    {
        a.turret = 0;
    }
    return a;
}

int StickDirection(const plat::PadState& pad)
{
    if (pad.axisX < -kStickOn)
    {
        return kStickLeft;
    }
    if (pad.axisX > kStickOn)
    {
        return kStickRight;
    }
    if (pad.axisY < -kStickOn)
    {
        return kStickUp;
    }
    if (pad.axisY > kStickOn)
    {
        return kStickDown;
    }
    return 0;
}

bool Reader::Held(const plat::PadState& pad, int source) const
{
    switch (source)
    {
        case kStickLeft:
            return mStick[0] < 0;
        case kStickRight:
            return mStick[0] > 0;
        case kStickUp:
            return mStick[1] < 0;
        case kStickDown:
            return mStick[1] > 0;
        default:
            return source >= 0 && source < kButtonSlots && pad.buttons[source] != 0;
    }
}

bool Reader::Down(const plat::PadState& pad, int source) const
{
    return source != kNone && !mLatched[source & 0xFF] && Held(pad, source);
}

void Reader::Latch(const plat::PadState& pad)
{
    mStick[0] = Hysteresis(mStick[0], pad.axisX);
    mStick[1] = Hysteresis(mStick[1], pad.axisY);
    for (int source = 0; source < 256; source++)
    {
        mLatched[source] = Held(pad, source);
    }
    // A trigger pressed even lightly, or the stick off centre, stays out until let go
    mLatched[10] = mLatched[10] || pad.triggerLeft > kTriggerDead;
    mLatched[11] = mLatched[11] || pad.triggerRight > kTriggerDead;
    const bool stickOff = pad.axisX * static_cast<double>(pad.axisX) + pad.axisY * static_cast<double>(pad.axisY) >
        kSteerDead * kSteerDead * 32767.0 * 32767.0;
    if (stickOff)
    {
        mLatched[kStickLeft] = mLatched[kStickRight] = mLatched[kStickUp] = mLatched[kStickDown] = true;
    }
}

Frame Reader::Read(const plat::PadState* pad, Context context, const Settings& settings, bool analogAllowed, bool blocked)
{
    Frame frame;
    if (!pad)
    {
        mStick[0] = mStick[1] = 0;
        memset(mLatched, 0, sizeof(mLatched));
        mHaveContext = false;
        mWasBlocked = blocked;
        return frame;
    }
    if (blocked)
    {
        mWasBlocked = true;
        return frame;
    }
    // A layout change or a closed menu: what is held now meant something else
    if (mWasBlocked || (mHaveContext && context != mContext))
    {
        Latch(*pad);
    }
    mWasBlocked = false;
    mHaveContext = true;
    mContext = context;

    mStick[0] = Hysteresis(mStick[0], pad->axisX);
    mStick[1] = Hysteresis(mStick[1], pad->axisY);
    const double x = pad->axisX / 32767.0;
    const double y = pad->axisY / 32767.0;
    const double size = sqrt(x * x + y * y);

    // Let go of: no longer ignored
    for (int source = 0; source < 256; source++)
    {
        if (mLatched[source] && !Held(*pad, source))
        {
            const bool trigger = IsTrigger(source) && TriggerTravel(*pad, source) > kTriggerDead;
            const bool stick = source >= kStickLeft && source <= kStickDown && size > kSteerDead;
            mLatched[source] = trigger || stick;
        }
    }

    const Layout& layout = settings.layout[context];
    for (int action = 0; action < kActions; action++)
    {
        if (Down(*pad, layout.source[action]))
        {
            frame.bits |= 1u << action;
        }
    }

    const bool stickLatched = mLatched[kStickLeft] || mLatched[kStickRight] || mLatched[kStickUp] || mLatched[kStickDown];
    if (context == OnFoot)
    {
        if (settings.worldDirections && size > kSteerDead && !stickLatched)
        {
            // Rotation 0 faces down the screen and grows to the left: atan2(x, y) is that angle
            int heading = static_cast<int>(lround(atan2(x, y) * 128.0 / (2.0 * M_PI)));
            frame.analog.heading = ((heading % 128) + 128) % 128;
            double push = (size - kSteerDead) / (kSteerFull - kSteerDead);
            push = push > 1.0 ? 1.0 : (push < 0.0 ? 0.0 : push);
            frame.analog.walk = analogAllowed ? static_cast<int>(lround(push * 7.0)) : 7;
            // The game's Forward, Backward, Left and Right come from the heading now
            frame.bits &= ~0xFu;
        }
        return frame;
    }

    // In a car: triggers bound to Forward or Backward are pedals, acting from a light press
    int32_t travelled = 0;
    for (int action = 0; action < 2; action++)
    {
        const int source = layout.source[action];
        if (IsTrigger(source) && !mLatched[source])
        {
            const int32_t travel = TriggerTravel(*pad, source);
            frame.bits &= ~(1u << action);
            if (travel > kTriggerDead)
            {
                frame.bits |= 1u << action;
                travelled = travel > travelled ? travel : travelled;
            }
        }
    }
    if (analogAllowed && travelled)
    {
        const int level = static_cast<int>(ceil((travelled - kTriggerDead) * 15.0 / (32767 - kTriggerDead)));
        frame.analog.gas = level < 1 ? 1 : (level > 15 ? 15 : level);
    }
    const double rx = pad->rightX / 32767.0;
    if (analogAllowed && (rx > kTurretDead || rx < -kTurretDead))
    {
        frame.analog.turret = rx < 0 ? 1 : 2;
    }
    return frame;
}
} // namespace padinput
