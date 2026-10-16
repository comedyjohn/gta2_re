// The pad layer (port/platform/padinput.h): bindings, the stick, the analog state and the settings file
#include <doctest.h>

#include "padinput.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace padinput;

static plat::PadState Rest()
{
    plat::PadState p;
    memset(&p, 0, sizeof(p));
    p.pov = -1;
    return p;
}

static Settings ButtonsOnly()
{
    Settings s = Defaults();
    s.worldDirections = false;
    return s;
}

TEST_CASE("pad buttons drive the controls of the layout in use")
{
    Reader r;
    const Settings s = ButtonsOnly();
    plat::PadState p = Rest();
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 0);
    p.buttons[2] = 0x80; // X: fire on foot
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == (1u << 4));
    p.buttons[2] = 0;
    p.buttons[5] = 0x80; // RB: next weapon on foot ...
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == (1u << 8));
    p.buttons[5] = 0;
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 0);
    CHECK(r.Read(nullptr, OnFoot, s, true, false).bits == 0);
}

TEST_CASE("the stick is four directions with hysteresis")
{
    Reader r;
    const Settings s = ButtonsOnly();
    plat::PadState p = Rest();
    p.axisY = -32767 * 30 / 100; // under the 40% that turns it on
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 0);
    p.axisY = -32767 * 50 / 100;
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 1u); // Forward
    p.axisY = -32767 * 30 / 100; // still above the 25% that turns it off
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 1u);
    p.axisY = -32767 * 20 / 100;
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 0);
    p.axisY = 0;
    p.axisX = 32767;
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == (1u << 3)); // Right
}

TEST_CASE("on foot with world directions the stick gives a heading and a speed, not direction bits")
{
    Reader r;
    const Settings s = Defaults();
    plat::PadState p = Rest();
    p.axisY = 32767; // down the screen: rotation 0
    Frame f = r.Read(&p, OnFoot, s, true, false);
    CHECK(f.bits == 0);
    CHECK(f.analog.heading == 0);
    CHECK(f.analog.walk == 7);
    p.axisY = 0;
    p.axisX = -32767; // left: a quarter turn the way rotations grow
    f = r.Read(&p, OnFoot, s, true, false);
    CHECK(f.analog.heading == 96);
    p.axisX = 32767;
    CHECK(r.Read(&p, OnFoot, s, true, false).analog.heading == 32);
    p.axisX = 0;
    p.axisY = -32767 * 35 / 100; // a light push walks slowly
    f = r.Read(&p, OnFoot, s, true, false);
    CHECK(f.analog.heading == 64);
    CHECK(f.analog.walk == 1);
    // In a network game the speed is not the stick's: the other machines would not know it
    CHECK(r.Read(&p, OnFoot, s, false, false).analog.walk == 7);
    p.axisY = 0;
    CHECK(r.Read(&p, OnFoot, s, true, false).analog == Analog {});
}

TEST_CASE("in a car the triggers are analog pedals and the right stick turns the turret")
{
    Reader r;
    const Settings s = Defaults();
    plat::PadState p = Rest();
    p.triggerRight = 32767; // RT: Forward
    Frame f = r.Read(&p, InCar, s, true, false);
    CHECK(f.bits == 1u);
    CHECK(f.analog.gas == 15);
    p.triggerRight = 8000; // a light press, under the trigger's 16000 as a button
    f = r.Read(&p, InCar, s, true, false);
    CHECK(f.bits == 1u);
    CHECK(f.analog.gas > 0);
    CHECK(f.analog.gas < 5);
    CHECK(r.Read(&p, InCar, s, false, false).analog.gas == 0);
    p.triggerRight = 0;
    p.rightX = -32767;
    f = r.Read(&p, InCar, s, true, false);
    CHECK(f.bits == 0);
    CHECK(f.analog.turret == 1);
    p.rightX = 32767;
    CHECK(r.Read(&p, InCar, s, true, false).analog.turret == 2);
    CHECK(r.Read(&p, InCar, s, false, false).analog.turret == 0);
}

TEST_CASE("what is held when the layout changes or a menu closes is ignored until let go")
{
    Reader r;
    const Settings s = ButtonsOnly();
    plat::PadState p = Rest();
    p.buttons[3] = 0x80; // Y: enter a car
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == (1u << 5));
    // In the car Y leaves it again: still held, so not yet
    CHECK(r.Read(&p, InCar, s, true, false).bits == 0);
    CHECK(r.Read(&p, InCar, s, true, false).bits == 0);
    p.buttons[3] = 0;
    CHECK(r.Read(&p, InCar, s, true, false).bits == 0);
    p.buttons[3] = 0x80;
    CHECK(r.Read(&p, InCar, s, true, false).bits == (1u << 5));

    // A menu has the pad: the game sees it at rest, and A (pressed to choose) stays out when it closes
    p = Rest();
    p.buttons[0] = 0x80;
    CHECK(r.Read(&p, OnFoot, s, true, true).bits == 0);
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == 0);
    p.buttons[0] = 0;
    r.Read(&p, OnFoot, s, true, false);
    p.buttons[0] = 0x80;
    CHECK(r.Read(&p, OnFoot, s, true, false).bits == (1u << 6));
}

TEST_CASE("analog state packs into 17 bits and back")
{
    for (int heading = -1; heading < 128; heading += 7)
    {
        for (int walk = 0; walk < 8; walk += 3)
        {
            for (int gas = 0; gas < 16; gas += 5)
            {
                for (int turret = 0; turret < 3; turret++)
                {
                    Analog a;
                    a.heading = heading;
                    a.walk = walk;
                    a.gas = gas;
                    a.turret = turret;
                    const uint32_t packed = Pack(a);
                    CHECK(packed < (1u << 17));
                    CHECK(Unpack(packed) == a);
                }
            }
        }
    }
    CHECK(Pack(Analog {}) == Pack(Unpack(Pack(Analog {}))));
}

TEST_CASE("the settings file keeps the layouts and survives damage")
{
    char dir[] = "/tmp/gta2-padinput-XXXXXX";
    REQUIRE(mkdtemp(dir));
    const std::string file = std::string(dir) + "/pad.txt";

    // No file: the defaults
    Settings s = Load(file);
    CHECK(s.layout[OnFoot].source[4] == DefaultLayout(OnFoot).source[4]);
    CHECK(s.worldDirections);
    CHECK(s.escMenu);

    s.layout[InCar].source[4] = 7;
    s.layout[OnFoot].source[0] = kNone;
    s.worldDirections = false;
    s.legend = false;
    s.escMenu = false;
    REQUIRE(Save(file, s));
    const Settings back = Load(file);
    CHECK(back.layout[InCar].source[4] == 7);
    CHECK(back.layout[OnFoot].source[0] == kNone);
    CHECK_FALSE(back.worldDirections);
    CHECK_FALSE(back.legend);
    CHECK_FALSE(back.escMenu);

    // Bad values and junk lines are skipped
    FILE* f = fopen(file.c_str(), "w");
    fputs("garbage\nfoot.4 = 99\ncar.12 = 3\nfoot.5 = 1\n", f);
    fclose(f);
    const Settings damaged = Load(file);
    CHECK(damaged.layout[OnFoot].source[4] == DefaultLayout(OnFoot).source[4]);
    CHECK(damaged.layout[OnFoot].source[5] == 1);
    remove(file.c_str());
    rmdir(dir);
}
