#include "ang16.hpp"
#include "Function.hpp"
#include <cmath>

DEFINE_GLOBAL_INIT(Ang16, kAng45_6F677C, Ang16(0xB4), 0x6F677C);
DEFINE_GLOBAL_INIT(Ang16, kAng315_6F680C, Ang16(0x4EC), 0x6F680C);
DEFINE_GLOBAL_INIT(Ang16, kAng135_6F6800, Ang16(0x21C), 0x6F6800);
DEFINE_GLOBAL_INIT(Ang16, kAng225_6F67DC, Ang16(0x384), 0x6F67DC);

EXTERN_GLOBAL(Ang16, kAng180_669156);

// 0x405640
void Ang16::SnapToAng4()
{
    if (rValue < 180 || rValue > 1260)
    {
        rValue = 0;
    }
    else if (rValue < 540)
    {
        rValue = 360;
    }
    else
    {
        rValue = rValue < 900 ? 720 : 1080;
    }
}

// 0x405680
u8 Ang16::ToAng4()
{
    return rValue / 360;
}

// 0x4056A0
u8 Ang16::GetOctant()
{
    return rValue / 180;
}

// 0x405B60
Fix16 Ang16::NormalizeAngleDeltaScaled(Ang16& from, Ang16& to, Ang16& divisor)
{
    Ang16 d = to - from;
    if (d > kAng180_669156)
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
    if (*pTo - *pFrom <= kAng180_669156)
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
    rValue = value;
    Normalize();
    return this;
}

// 0x409340
Ang16 Ang16::SubtractNormalized(const Ang16& toSub)
{
    return Ang16(rValue - toSub.rValue, static_cast<u8>(0));
}

// 0x4516B0
Ang16::Ang16(Fix16* a2, s32 a3)
{
    rValue = a2->ToInt();
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
    return Ang16(value.mValue / 71, 0);
}

// 0x4F78F0
s32 Ang16::GetAngleFace(const Ang16& angle)
{
    if (angle <= kAng45_6F677C || angle > kAng315_6F680C) //  45° and 315°
    {
        return 2; //  North
    }
    if (angle < kAng135_6F6800) // kAng135_6F6800 = 135°
    {
        return 3; //  East
    }
    return angle < kAng225_6F67DC ? 1 : 4; // kAng225_6F67DC = 225°  ,  1 = South, 4 = West
}
