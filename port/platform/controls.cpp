// The pad in the game: reads it once per game update (port_input_read), keeps the analog state a
// replay records, and applies it (port_input_apply, port_gas_scale, port_walk_speed). See
// port/include/port_input.h.
#include "controls.h"

#include "overlay.h"
#include "port_input.h"

#include "Car_BC.hpp"
#include "Frontend.hpp"
#include "Game_0x40.hpp"
#include "Ped.hpp"
#include "Player.hpp"
#include "char.hpp"
#include "winmain.hpp"

#include <dinput.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace
{
padinput::Settings gSettings;
bool gSettingsLoaded = false;
padinput::Reader gReader;
unsigned int gAnalog = padinput::Pack(padinput::Analog {});
int gSpawnRequest = -1;

// GTA2_TEST_PAD=FILE: a made-up pad for tests, one line per change: "READ X Y BUTTONS LT RT RX" (from
// the READth pad read of the game on: left stick, buttons as a hex mask of slots 0..11, triggers,
// right stick X). It stands in for a real pad, so a pad session can be recorded headless.
struct ScriptStep
{
    long read;
    plat::PadState pad;
};
std::vector<ScriptStep> gScript;
bool gScriptLoaded = false;
long gReads = 0; // port_input_read calls since the start

bool ScriptedPad(plat::PadState& pad)
{
    if (!gScriptLoaded)
    {
        gScriptLoaded = true;
        const char* file = getenv("GTA2_TEST_PAD");
        FILE* f = file ? fopen(file, "r") : nullptr;
        char line[256];
        while (f && fgets(line, sizeof(line), f))
        {
            ScriptStep step;
            memset(&step.pad, 0, sizeof(step.pad));
            step.pad.pov = -1;
            unsigned buttons = 0;
            int x = 0, y = 0, lt = 0, rt = 0, rx = 0;
            if (sscanf(line, "%ld %d %d %x %d %d %d", &step.read, &x, &y, &buttons, &lt, &rt, &rx) == 7)
            {
                step.pad.axisX = x;
                step.pad.axisY = y;
                step.pad.triggerLeft = lt;
                step.pad.triggerRight = rt;
                step.pad.rightX = rx;
                for (int slot = 0; slot < 12; slot++)
                {
                    step.pad.buttons[slot] = (buttons >> slot) & 1 ? 0x80 : 0;
                }
                step.pad.buttons[10] = lt > 16000 ? 0x80 : step.pad.buttons[10];
                step.pad.buttons[11] = rt > 16000 ? 0x80 : step.pad.buttons[11];
                gScript.push_back(step);
            }
        }
        if (f)
        {
            fclose(f);
        }
        if (file && gScript.empty())
        {
            fprintf(stderr, "gta2: GTA2_TEST_PAD %s has no steps\n", file);
        }
    }
    if (gScript.empty())
    {
        return false;
    }
    memset(&pad, 0, sizeof(pad));
    pad.pov = -1;
    for (const ScriptStep& step : gScript)
    {
        if (step.read <= gReads)
        {
            pad = step.pad;
        }
    }
    return true;
}

Ped* LocalPed()
{
    Player* player = gGame_0x40_67E008 ? gGame_0x40_67E008->field_38_local_player : nullptr;
    return player ? player->field_2C4_player_ped : nullptr;
}

// Puts the car down in front of the player (or of the car the player is in) if there is open
// ground there, as high as the player stands, else to either side or behind. In the game's own
// fixed point and sine table, so a replay with a spawn plays the same on every system.
void Spawn(int model)
{
    Ped* ped = LocalPed();
    if (!ped || !gCarManager_6C_677930)
    {
        return;
    }
    Sprite* sprite = nullptr;
    Fix16 ahead = Fix16(3); // map blocks
    if (Car_BC* car = ped->get_car_416B60())
    {
        sprite = car->field_50_car_sprite;
        ahead = Fix16(4);
    }
    else if (ped->field_168_game_object)
    {
        sprite = ped->field_168_game_object->field_80_sprite_ptr;
    }
    if (!sprite)
    {
        return;
    }
    Fix16 x = sprite->GetXPos();
    Fix16 y = sprite->GetYPos();
    const Fix16 highest = sprite->field_1C_zpos + Fix16(0x2000, 0); // half a block above the player
    const Ang16 rotation = ped->GetRotation(); // 0 is down the screen
    const s16 kTurns[] = {0, 180, -180, 360, -360, 540, -540, 720}; // eighths of a turn
    Fix16 dx;
    Fix16 dy;
    Ang16::PolarToCartesian(rotation, ahead, dx, dy);
    Fix16 spawnX = x + dx;
    Fix16 spawnY = y + dy;
    bool found = false;
    for (int ring = 0; ring < 2 && !found; ring++)
    {
        const Fix16 dist = ring ? ahead + Fix16(0x6000, 0) : ahead; // and one and a half blocks further
        for (s16 turn : kTurns)
        {
            const s16 value = static_cast<s16>(rotation.r_value_ + turn);
            const Ang16 angle(value, 0);
            Ang16::PolarToCartesian(angle, dist, dx, dy);
            const Fix16 cx = x + dx;
            const Fix16 cy = y + dy;
            if (gMap_0x370_6F6268->FindGroundZForCoord_4E5B60(cx, cy) <= highest)
            {
                spawnX = cx;
                spawnY = cy;
                found = true;
                break;
            }
        }
    }
    gCarManager_6C_677930->SpawnCarAtCorrectZ_426E40(spawnX, spawnY, rotation, model);
}

// Angles in the game's units, 1440 to a turn, into -720..719
int Wrap(int angle)
{
    return ((angle % 1440) + 1440 + 720) % 1440 - 720;
}
} // namespace

namespace controls
{
int FirstPad(plat::PadState& pad)
{
    if (ScriptedPad(pad))
    {
        return 0;
    }
    for (int i = 0; i < 8; i++)
    {
        if (plat::GetPadState(i, pad))
        {
            return i;
        }
    }
    return -1;
}

padinput::Settings& Pad()
{
    if (!gSettingsLoaded)
    {
        gSettingsLoaded = true;
        gSettings = padinput::Load(padinput::SettingsFile());
    }
    return gSettings;
}

void SavePad()
{
    if (!padinput::Save(padinput::SettingsFile(), Pad()))
    {
        fprintf(stderr, "gta2: could not save the pad settings to %s\n", padinput::SettingsFile().c_str());
    }
}

padinput::Context CurrentContext()
{
    Ped* ped = LocalPed();
    return (ped && ped->get_car_416B60()) ? padinput::InCar : padinput::OnFoot;
}

void RequestSpawn(int model)
{
    gSpawnRequest = model;
}

void MenuKeys(uint8_t keys[256])
{
    plat::PadState pad;
    if (!gFrontend_67DC84 || overlay::Captured() || FirstPad(pad) < 0)
    {
        return;
    }
    switch (padinput::StickDirection(pad))
    {
        case padinput::kStickLeft:
            keys[DIK_LEFT] = 0x80;
            break;
        case padinput::kStickRight:
            keys[DIK_RIGHT] = 0x80;
            break;
        case padinput::kStickUp:
            keys[DIK_UP] = 0x80;
            break;
        case padinput::kStickDown:
            keys[DIK_DOWN] = 0x80;
            break;
        default:
            break;
    }
    if (pad.buttons[0] || pad.buttons[7]) // A, Start
    {
        keys[DIK_RETURN] = 0x80;
    }
    if (pad.buttons[1] || pad.buttons[6]) // B, Back
    {
        keys[DIK_ESCAPE] = 0x80;
    }
    if (pad.buttons[2]) // X: delete (a save slot, a typed letter)
    {
        keys[DIK_DELETE] = 0x80;
    }
}
} // namespace controls

PortPadFrame port_input_read()
{
    PortPadFrame out;
    out.captured = overlay::Captured();
    const bool network = bStartNetworkGame_7081F0 != 0;
    out.spawn = network ? -1 : gSpawnRequest;
    gSpawnRequest = -1;
    gReads++;
    plat::PadState pad;
    const bool havePad = controls::FirstPad(pad) >= 0;
    const padinput::Frame frame =
        gReader.Read(havePad ? &pad : nullptr, controls::CurrentContext(), controls::Pad(), !network, out.captured);
    out.bits = frame.bits;
    out.analog = padinput::Pack(frame.analog);
    return out;
}

bool port_game_frozen()
{
    return overlay::FreezesGame();
}

unsigned int port_input_analog()
{
    return gAnalog;
}

void port_input_set_analog(unsigned int packed)
{
    gAnalog = packed & kPortPayload;
}

void port_input_reset()
{
    gAnalog = padinput::Pack(padinput::Analog {});
    gSpawnRequest = -1;
}

void port_input_spawn(int model)
{
    Spawn(model);
}

unsigned int port_input_apply(unsigned int bits)
{
    const padinput::Analog analog = padinput::Unpack(gAnalog);
    Ped* ped = LocalPed();
    if (!ped)
    {
        return bits;
    }
    if (Car_BC* car = ped->get_car_416B60())
    {
        if (analog.turret)
        {
            car->HandleRoofTurretRotation_440D90(analog.turret == 1 ? 1 : 0);
        }
        return bits;
    }
    if (analog.heading < 0 || !ped->field_168_game_object)
    {
        return bits;
    }
    // Turn towards the heading, and stop pressing the turn once the speed it has built up would
    // carry it there. The turn speed is an angle: a speed to the right is just under 1440.
    const int wanted = (analog.heading * 45 + 2) / 4; // 1440 / 128 = 11.25 per step
    const int diff = Wrap(wanted - ped->GetRotation().r_value_);
    int speed = Wrap(gGame_0x40_67E008->field_38_local_player->field_8_turn_speed.r_value_);
    speed = speed < 0 ? -speed : speed;
    const int dead = speed > 12 ? speed : 12;
    unsigned int steer = 0;
    if (diff > dead)
    {
        steer |= 4; // Left
    }
    else if (diff < -dead)
    {
        steer |= 8; // Right
    }
    if (diff < 300 && diff > -300)
    {
        steer |= 1; // Forward
    }
    return (bits & ~0xFU) | steer;
}

int port_gas_scale(const void* driverPed)
{
    const padinput::Analog analog = padinput::Unpack(gAnalog);
    if (!analog.gas || !driverPed || driverPed != LocalPed())
    {
        return 16384;
    }
    return 16384 * analog.gas / 15;
}

int port_walk_speed(const void* ped, int speed)
{
    const padinput::Analog analog = padinput::Unpack(gAnalog);
    if (analog.heading < 0 || analog.walk >= 7 || !ped || ped != LocalPed())
    {
        return speed;
    }
    // A light push walks at a fifth of the speed, all the way at full speed
    return static_cast<int>(static_cast<long long>(speed) * (7 + 4 * analog.walk) / 35);
}
