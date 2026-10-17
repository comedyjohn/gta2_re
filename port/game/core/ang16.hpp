#pragma once

#include "core/fix16.hpp"
#include "core/function.hpp"

EXTERN_GLOBAL(class Ang16, kAng180);

class Ang16
{
  public:
    //   Ang16  and  Ang16
    // 9.6f 0x40E670/0x41E0F0/0x40F4C0/0x40E690: setcc into a 32-bit register, so these return int, not bool
    s32 operator>(const Ang16& other) const
    {
        return r_value_ > other.r_value_;
    }

    s32 operator<(const Ang16& other) const
    {
        return r_value_ < other.r_value_;
    }

    s32 operator>=(const Ang16& other) const
    {
        return r_value_ >= other.r_value_;
    }

    s32 operator<=(const Ang16& other) const
    {
        return r_value_ <= other.r_value_;
    }

    s32 operator!=(const Ang16& other)
    {
        return r_value_ != other.r_value_;
    }

    Ang16 operator-=(const Ang16& other)
    {
        r_value_ -= other.r_value_;
        Normalize();
        return *this;
    }

    Ang16 operator+=(const Ang16& other)
    {
        r_value_ += other.r_value_;
        Normalize();
        return *this;
    }

    // TODO: this shouldn't exist, try to match Player::DoPedControlInputs_566C80 without it
    /*
    Ang16 operator=(const Ang16& other)
    {
        rValue = other.rValue;
        return *this;
    }
    */

    s32 operator==(const Ang16& other)
    {
        return r_value_ == other.r_value_;
    }

    Ang16 operator*(const Ang16& other)
    {
        return Ang16((Fix16(r_value_) * Fix16(other.r_value_)).ToInt());
    }

    Ang16 operator+(const Ang16& rhs)
    {
        s16 value = r_value_ + rhs.r_value_;
        return Ang16(value, 0);
    }

    Ang16 operator-(const Ang16& other)
    {
        s16 value = r_value_ - other.r_value_;
        return Ang16(value, 0);
    }

    // 9.6f 0x49E240, defined in CarPhysics_B0.cpp
    inline wchar_t* ThetaText();

    Ang16 operator-()
    {
        return Ang16(-r_value_, 0);
    }

    //   Ang16  and  const int

    bool operator>(const s32 other) const
    {
        return r_value_ > other;
    }

    bool operator<(const s32 other) const
    {
        return r_value_ < other;
    }

    bool operator>=(const s32 other) const
    {
        return r_value_ >= other;
    }

    bool operator<=(const s32 other) const
    {
        return r_value_ <= other;
    }

    Ang16 operator-=(const s32 other)
    {
        r_value_ -= other;
        return *this;
    }

    Ang16 operator+=(const s32 other)
    {
        r_value_ += other;
        return *this;
    }

    Ang16 operator<<(const s32 other)
    {
        r_value_ = r_value_ << other;
        return *this;
    }

    Ang16 operator>>(const s32 other)
    {
        r_value_ = r_value_ >> other;
        return *this;
    }

    Ang16 operator*(const s32 other)
    {
        r_value_ *= other;
        return *this;
    }

    Ang16 operator=(const s32 other)
    {
        r_value_ = other;
        return *this;
    }

    Ang16 operator-(const s32 other)
    {
        r_value_ -= other;
        return *this;
    }

    s32 ToFloat()
    {
        return r_value_ << 14;
    }

    s32 FromUnsignedToFloat()
    {
        return static_cast<u16>(r_value_) << 14;
    }

    void ConvertAndMultiply(Ang16* a1, Ang16* a2)
    {
        Fix16 fix_1;
        fix_1.value_ = a2->FromUnsignedToFloat();
        Fix16 fix_2;
        fix_2.value_ = a1->ToFloat();
        Fix16 fix_3;
        fix_3.value_ = fix_2.MultiplyInt64(fix_1);

        r_value_ = fix_3.ToInt();
    }

    void NormalizeOutOfLine();

    Ang16& Normalized()
    {
        NormalizeOutOfLine();
        return *this;
    }

    void Normalize()
    {
        for (; r_value_ < 0; r_value_ += 1440)
        {
            ;
        }
        for (; r_value_ >= 1440; r_value_ -= 1440)
        {
            ;
        }
    }

    static bool IsAngleAhead(Ang16* from, Ang16* to);

    void SnapToAng4();
    Ang16* AssignNormalized(const s16& value, s32 a3);
    Ang16 SubtractNormalized(const Ang16& to_sub);
    static Ang16 Fix16ToAng16OutOfLine(Fix16& value);

    // 10.5 0x409300 (AssignNormalized_409300) is the out-of-line copy of the normalizing constructor
    // Ang16(const s16&, s32) below (9.6f 0x401C60). VC6 calls it instead of inlining in functions that
    // ran out of inline expansions. C++ can't declare the inline and the out-of-line copy of one
    // constructor, so the out-of-line one is reached through these overloads.
    Ang16(Ang16& input, s32 a3)
    {
        AssignNormalized(input.r_value_, a3);
    }

    Ang16(const s16* value, s32 a3)
    {
        AssignNormalized(*value, a3);
    }

    // The value is stored as a dword, as 9.6f's 0x40F540 passes it to 0x401C60; the copy reads its low word
    Ang16(const s32* value, s32 a3)
    {
        AssignNormalized(*reinterpret_cast<const s16*>(value), a3);
    }

    // The out-of-line copy of Ang16(Fix16& value, u8) (9.6f 0x401C40). A real constructor, not a
    // method: when it is called out of line its 0 argument is pushed before the Fix16 argument is
    // evaluated (Particle_8::EmitImpactParticles_53FE40). Also used by miss2_0x11C::SCRCMD_CRANE_5041C0.
    Ang16(Fix16* a2, s32 a3);

    static Fix16 Cosine(const Ang16& angle)
    {
        return gCosTable[angle.r_value_];
    }

    static Fix16 Sine(const Ang16& angle)
    {
        return gSinTable[angle.r_value_];
    }

    // 9.6f 0x41FC20: the table value is the left operand (this) of the multiply
    static void PolarToCartesian(const Ang16& angle, const Fix16& radius, Fix16& ret1, Fix16& ret2)
    {
        ret1 = Ang16::Sine(angle) * radius;
        ret2 = Ang16::Cosine(angle) * radius;
    }

    // Non-inlined version of above function
    static void PolarToCartesianOutOfLine(Ang16& angle, const Fix16& radius, Fix16& ret1, Fix16& ret2);

    static s32 GetAngleFace(const Ang16& angle);

    // inlined from 0x40f580 on v9.6f
    // I was tempted to put this on the Fix16 header, but that will create a circular dependency...
    static Fix16 _stdcall Ang16ToFix16(const Ang16& v)
    {
        s32 value = v.r_value_ * 71;
        return Fix16(value, 0);
    }

    // Reverse of above func
    static Ang16 Fix16ToAng16(const Fix16& a2)
    {
        return Ang16(a2.GetRaw() / 71, 0);
    }

    bool IsAxisAligned() const
    {
        return (r_value_ == 0 || r_value_ == 360 || r_value_ == 720 || r_value_ == 1080) ? true : false;
    }

    bool JIsAxisAligned()
    {
        return IsAxisAligned();
    }

    Ang16(const s16& value, s32 not_used) : r_value_(value)
    {
        this->Normalize();
    }

    Ang16() : r_value_(0)
    {
    }

    Ang16(s16 value, u8 null2) : r_value_(value)
    {
        Normalize();
    }

    Ang16(s32 value) : r_value_(value)
    {
    }

    Ang16(Fix16 value) : r_value_(value.value_ >> 14) // value.ToInt() breaks Map_0x370::SpawnMapObjects_4DFB90
    {
    }

    // 9.6f func: 0x401C40
    Ang16(const Fix16& value, u8 zero)
    {
        r_value_ = value.ToInt();
        Normalize();
    }

    Ang16 MultiplyByFix16(const Fix16& a2)
    {
        return Ang16(Fix16(r_value_) * a2, 0);
    }

    Fix16 DivideBy(const Ang16& scale) const
    {
        return Fix16(r_value_) / scale.r_value_;
    }

    static Fix16 NormalizeAngleDeltaScaled(Ang16& from, Ang16& to, Ang16& divisor);
    u8 GetOctant();
    u8 ToAng4();

    // TODO: move with CarDoorAlignmentSolver_545AF0
    // https://decomp.me/scratch/6VBR0  (matched)
    static void RotateVector(Fix16& xpos, Fix16& ypos, Ang16& rotation)
    {
        Fix16 old_xpos = xpos;
        xpos = xpos * Ang16::Cosine(rotation) + ypos * Ang16::Sine(rotation);
        ypos = (-old_xpos) * Ang16::Sine(rotation) + ypos * Ang16::Cosine(rotation);
    }

    s16 r_value_;
};

inline Ang16 operator-(const Ang16& a, const Ang16& b)
{
    return a.r_value_ - b.r_value_;
}

class Ang8
{
  public:
    //  maybe an Ang8 inline at 0x462EF0 in 9.6f
    Ang16 ConvertToAng16()
    {
        return Ang16((Fix16(r_value_) / 256) * 1440);
    }

    u8 r_value_;
};

inline void RotateAndTranslatePoint(const Fix16& in_x,
                                    const Fix16& in_y,
                                    const Ang16& rot_ang,
                                    Fix16& trans_x,
                                    Fix16& trans_y,
                                    Fix16& rot_trans_x,
                                    Fix16& rot_trans_y)
{
    rot_trans_x = (((in_x - trans_x) * Ang16::Cosine(rot_ang)) + ((in_y - trans_y) * Ang16::Sine(rot_ang)));
    rot_trans_y = ((-(in_x - trans_x) * Ang16::Sine(rot_ang)) + ((in_y - trans_y) * Ang16::Cosine(rot_ang)));
}
