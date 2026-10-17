#pragma once

#include "fix16.hpp"
#include "types.hpp"

struct Angle
{
    s16 field_0_value;

    Angle() : field_0_value(0)
    {
    }

    Angle(const s16& param_1, s32)
    {
        this->field_0_value = param_1;
        clamp();
    }

    Angle(const Fix16& param_1, s32)
    {
        field_0_value = param_1.ToInt();
        clamp();
    }

    void clamp();

    s32 operator!=(const Angle& right) const
    {
        return field_0_value != right.field_0_value;
    }

    s32 operator>(const Angle& right) const
    {
        return field_0_value > right.field_0_value;
    }

    s32 operator<(const Angle& right) const
    {
        return field_0_value < right.field_0_value;
    }

    s32 operator<=(const Angle& right) const
    {
        return field_0_value <= right.field_0_value;
    }

    Angle operator+(const Angle& right) const
    {
        return Angle(field_0_value + right.field_0_value, 0);
    }

    Angle operator-(const Angle& right) const
    {
        return Angle(field_0_value - right.field_0_value, 0);
    }

    Angle operator-()
    {
        return Angle(-field_0_value, 0);
    }

    Angle& operator+=(const Angle& right)
    {
        field_0_value = field_0_value + right.field_0_value;
        clamp();
        return *this;
    }

    Angle& operator-=(const Angle& right)
    {
        field_0_value = field_0_value - right.field_0_value;
        clamp();
        return *this;
    }
};

//static_assert(sizeof(angle) == 0x2);