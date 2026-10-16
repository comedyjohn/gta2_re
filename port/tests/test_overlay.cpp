#include <doctest.h>

#include "overlay.h"

#include <dinput.h>
#include <string.h>

static plat::PadState Rest()
{
    plat::PadState p;
    memset(&p, 0, sizeof(p));
    p.pov = -1;
    return p;
}

TEST_CASE("overlay names keys and bindings")
{
    CHECK(overlay::KeyName(DIK_LCONTROL) == "Ctrl");
    CHECK(overlay::KeyName(DIK_Z) == "Z");
    CHECK(overlay::KeyName(DIK_0) == "0");
    CHECK(overlay::KeyName(DIK_1) == "1");
    CHECK(overlay::KeyName(DIK_F5) == "F5");
    CHECK(overlay::KeyName(0x7F) == "Key 0x7F");
    CHECK(overlay::BindingLabel(0, DIK_SPACE) == "Space");
    CHECK(overlay::BindingLabel(1, 224) == "D-pad Left");
    CHECK(overlay::BindingLabel(1, 227) == "D-pad Down");
    CHECK(overlay::BindingLabel(1, 20) == "Button 21");
    CHECK(overlay::BindingLabel(1, 255) == "None");
    CHECK(std::string(overlay::ActionName(4)) == "Fire");
}

TEST_CASE("overlay captures the pad input that just went down")
{
    const plat::PadState rest = Rest();
    int binding = -1;
    CHECK_FALSE(overlay::CapturePadBinding(rest, rest, binding));

    plat::PadState a = Rest();
    a.buttons[3] = 0x80;
    CHECK(overlay::CapturePadBinding(rest, a, binding));
    CHECK(binding == 3);
    // Still held on the next frame: not a new press
    CHECK_FALSE(overlay::CapturePadBinding(a, a, binding));

    plat::PadState stick = Rest();
    stick.axisY = -32767;
    CHECK(overlay::CapturePadBinding(rest, stick, binding));
    CHECK(binding == 226);
    stick.axisY = 0;
    stick.axisX = 32767;
    CHECK(overlay::CapturePadBinding(rest, stick, binding));
    CHECK(binding == 225);
    // A small deflection is not a press
    stick.axisX = 3000;
    CHECK_FALSE(overlay::CapturePadBinding(rest, stick, binding));
}
