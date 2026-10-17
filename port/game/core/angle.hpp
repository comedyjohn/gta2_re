#pragma once

#include "core/fix16.hpp"
#include "core/types.hpp"

struct Angle
{
    s16 value_;

    Angle() : value_(0)
    {
    }

    Angle(const s16& param_1, s32)
    {
        this->value_ = param_1;
        clamp();
    }

    Angle(const Fix16& param_1, s32)
    {
        value_ = param_1.ToInt();
        clamp();
    }

    void clamp();

    s32 operator!=(const Angle& right) const
    {
        return value_ != right.value_;
    }

    s32 operator>(const Angle& right) const
    {
        return value_ > right.value_;
    }

    s32 operator<(const Angle& right) const
    {
        return value_ < right.value_;
    }

    s32 operator<=(const Angle& right) const
    {
        return value_ <= right.value_;
    }

    Angle operator+(const Angle& right) const
    {
        return Angle(value_ + right.value_, 0);
    }

    Angle operator-(const Angle& right) const
    {
        return Angle(value_ - right.value_, 0);
    }

    Angle operator-()
    {
        return Angle(-value_, 0);
    }

    Angle& operator+=(const Angle& right)
    {
        value_ = value_ + right.value_;
        clamp();
        return *this;
    }

    Angle& operator-=(const Angle& right)
    {
        value_ = value_ - right.value_;
        clamp();
        return *this;
    }
};

//static_assert(sizeof(angle) == 0x2);