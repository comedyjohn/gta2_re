#include "Fix16_Point.hpp"
#include "Function.hpp"

// The out-of-line copy of the inline operator+
// 0x40AC50
Fix16Point Fix16Point::Add_40AC50(const Fix16Point& in)
{
    return Fix16Point(x + in.x, y + in.y);
}

// The out-of-line copy of the inline operator-
// 0x40AC80
Fix16Point Fix16Point::Sub_40AC80(const Fix16Point& rhs) const
{
    return Fix16Point(x - rhs.x, y - rhs.y);
}

// 0x40ACB0
Fix16Point Fix16Point::Negate_40ACB0() const
{
    return Fix16Point(-x, -y);
}

// 0x40ACD0
Ang16 Fix16Point::atan2_40ACD0()
{
    return Fix16::atan2_fixed_405320(y, x);
}

// 0x438FE0
Fix16Point Fix16Point::Multiply_438FE0(const Fix16& in) const
{
    return Fix16Point(x * in, y * in);
}

// 0x442AD0
Fix16Point Fix16Point::NormalizeSafe_442AD0()
{
    Fix16 length = GetLength_41E260();
    if (length == gFix16_6777CC)
    {
        Fix16Point scaled = MultBy_442C80(128);
        length = scaled.GetLength_41E260();
        return scaled / length;
    }
    else
    {
        return *this / length;
    }
}

Fix16Point Fix16Point::operator/(const Fix16& in) const
{
    return Fix16Point(x / in, y / in);
}

// 0x442C80
Fix16Point Fix16Point::MultBy_442C80(const s32& factor)
{
    return Fix16Point(x * factor, y * factor);
}

// 0x442CB0
Fix16Point Fix16Point::Divide_442CB0(const Fix16& in)
{
    return Fix16Point(x / in, y / in);
}

// 0x453590
Fix16 Fix16Point::GetLength_453590()
{
    if (x == gFix16_6777CC)
    {
        return Fix16::Abs(y);
    }
    else if (y == gFix16_6777CC)
    {
        return Fix16::Abs(x);
    }
    else
    {
        return Fix16::SquareRoot(x * x + y * y);
    }
}

// 0x55F9E0
Fix16Point Fix16Point::operator/(const s32& a3)
{
    return Fix16Point(x / a3, y / a3);
}

// 0x5605E0
Fix16Point Fix16Point::Rotate90CCW_5605E0()
{
    // TODO: Mov instruction is encoded wrongly ??
    return Fix16Point(-y, x);
}

// 0x5E40C0
Fix16Point& Fix16Point::AddAssign_5E40C0(const Fix16Point& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

// 0x5E40E0
Fix16Point& Fix16Point::DivAssign_5E40E0(const Fix16& v)
{
    x /= v;
    y /= v;
    return *this;
}

// The larger of |x| and |y|, a cheap stand-in for the length.
// 0x5E4140
Fix16 Fix16Point::MaxAbs_5E4140()
{
    Fix16 ax = Fix16::Abs(x);
    Fix16 ay = Fix16::Abs(y);
    if (ax > ay)
    {
        return ax;
    }
    return ay;
}
