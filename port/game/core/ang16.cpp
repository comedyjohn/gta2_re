#include "core/ang16.hpp"
#include "core/function.hpp"
#include <cmath>

DEFINE_GLOBAL_INIT(Ang16, kAng45, Ang16(0xB4), 0x6F677C);
DEFINE_GLOBAL_INIT(Ang16, kAng315, Ang16(0x4EC), 0x6F680C);
DEFINE_GLOBAL_INIT(Ang16, kAng135, Ang16(0x21C), 0x6F6800);
DEFINE_GLOBAL_INIT(Ang16, kAng225, Ang16(0x384), 0x6F67DC);

EXTERN_GLOBAL(Ang16, kAng180);

// 0x405640
void Ang16::SnapToAng4()
{
    if (r_value_ < 180 || r_value_ > 1260)
    {
        r_value_ = 0;
    }
    else if (r_value_ < 540)
    {
        r_value_ = 360;
    }
    else
    {
        r_value_ = r_value_ < 900 ? 720 : 1080;
    }
}

// 0x405680
u8 Ang16::ToAng4()
{
    return r_value_ / 360;
}

// 0x4056A0
u8 Ang16::GetOctant()
{
    return r_value_ / 180;
}

// 0x405B60
Fix16 Ang16::NormalizeAngleDeltaScaled(Ang16& from, Ang16& to, Ang16& divisor)
{
    Ang16 d = to - from;
    if (d > kAng180)
    {
        d = -d;
        return d.DivideBy(divisor);
    }
    else
    {
        return d.DivideBy(divisor);
    }
}

// 0x405C60
bool Ang16::IsAngleAhead(Ang16* pFrom, Ang16* pTo)
{
    if (*pTo - *pFrom <= kAng180)
    {
        return true;
    }
    return false;
}

// 0x406C20
void Ang16::NormalizeOutOfLine()
{
    Normalize();
}

// 0x409300
Ang16* Ang16::AssignNormalized(const s16& value, s32 a3)
{
    r_value_ = value;
    Normalize();
    return this;
}

// 0x409340
Ang16 Ang16::SubtractNormalized(const Ang16& toSub)
{
    return Ang16(r_value_ - toSub.r_value_, static_cast<u8>(0));
}

// 0x4516B0
Ang16::Ang16(Fix16* a2, s32 a3)
{
    r_value_ = a2->ToInt();
    Normalize();
}

// TODO: move
// 0x451730
void Ang16::PolarToCartesianOutOfLine(Ang16& angle, const Fix16& radius, Fix16& ret1, Fix16& ret2)
{
    ret1 = radius * Ang16::Sine(angle);
    ret2 = radius * Ang16::Cosine(angle);
}

// 0x482740
Ang16 Ang16::Fix16ToAng16OutOfLine(Fix16& value)
{
    return Ang16(value.value_ / 71, 0);
}

// 0x4F78F0
s32 Ang16::GetAngleFace(const Ang16& angle)
{
    if (angle <= kAng45 || angle > kAng315) //  45° and 315°
    {
        return 2; //  North
    }
    if (angle < kAng135) // kAng135_6F6800 = 135°
    {
        return 3; //  East
    }
    return angle < kAng225 ? 1 : 4; // kAng225_6F67DC = 225°  ,  1 = South, 4 = West
}
