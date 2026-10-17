#include "core/fix16_point.hpp"
#include "core/function.hpp"

// The out-of-line copy of the inline operator+
// 0x40AC50
Fix16Point Fix16Point::Add(const Fix16Point& in)
{
    return Fix16Point(x + in.x, y + in.y);
}

// The out-of-line copy of the inline operator-
// 0x40AC80
Fix16Point Fix16Point::Sub(const Fix16Point& rhs) const
{
    return Fix16Point(x - rhs.x, y - rhs.y);
}

// 0x40ACB0
Fix16Point Fix16Point::Negate() const
{
    return Fix16Point(-x, -y);
}

// 0x40ACD0
Ang16 Fix16Point::Atan2OutOfLine()
{
    return Fix16::Atan2Fixed(y, x);
}

// 0x438FE0
Fix16Point Fix16Point::Multiply(const Fix16& in) const
{
    return Fix16Point(x * in, y * in);
}

// 0x442AD0
Fix16Point Fix16Point::NormalizeSafe()
{
    Fix16 length = GetLength();
    if (length == kFixZero)
    {
        Fix16Point scaled = MultBy(128);
        length = scaled.GetLength();
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
Fix16Point Fix16Point::MultBy(const s32& factor)
{
    return Fix16Point(x * factor, y * factor);
}

// 0x442CB0
Fix16Point Fix16Point::Divide(const Fix16& in)
{
    return Fix16Point(x / in, y / in);
}

// 0x453590
Fix16 Fix16Point::GetLengthOutOfLine()
{
    if (x == kFixZero)
    {
        return Fix16::Abs(y);
    }
    else if (y == kFixZero)
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
Fix16Point Fix16Point::Rotate90CCW()
{
    // TODO: Mov instruction is encoded wrongly ??
    return Fix16Point(-y, x);
}

// 0x5E40C0
Fix16Point& Fix16Point::AddAssign(const Fix16Point& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

// 0x5E40E0
Fix16Point& Fix16Point::DivAssign(const Fix16& v)
{
    x /= v;
    y /= v;
    return *this;
}

// The larger of |x| and |y|, a cheap stand-in for the length.
// 0x5E4140
Fix16 Fix16Point::MaxAbsOutOfLine()
{
    Fix16 ax = Fix16::Abs(x);
    Fix16 ay = Fix16::Abs(y);
    if (ax > ay)
    {
        return ax;
    }
    return ay;
}
