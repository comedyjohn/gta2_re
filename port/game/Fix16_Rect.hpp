#pragma once

#include "Fix16_Point.hpp"
#include "Function.hpp"
#include "fix16.hpp"

class Sprite;

EXTERN_GLOBAL(Fix16, kSmallWidthEpslion_703450);
EXTERN_GLOBAL(Fix16, kCollisionPrismHalfHeight_6771E4);

// ComputeCollisionPrism_4204D0's half height is a header static each TU of the original has its own copy of
// (9.6f has one, 0x5E4D50). A TU whose copy isn't kCollisionPrismHalfHeight_6771E4 defines
// FIX16_RECT_HALF_HEIGHT to it (and declares it) before its first include.
#ifndef FIX16_RECT_HALF_HEIGHT
    #define FIX16_RECT_HALF_HEIGHT kCollisionPrismHalfHeight_6771E4
#endif

inline bool IntervalsOverlap(const Fix16& ourMin, const Fix16& ourMax, const Fix16& otherMin, const Fix16& otherMax)
{
    if (ourMin < otherMin)
    {
        if (ourMax < otherMin)
        {
            return false;
        }
        return true;
    }
    if (ourMin <= otherMax)
    {
        return true;
    }
    return false;
}

class Fix16Rect
{
  public:
    Fix16Rect() = default;

    void ComputeCollisionPrism(Fix16 x, Fix16 y, Fix16 offset, Fix16 z)
    {
        Fix16 tmp = offset / 2;

        field_0_left = x - tmp;
        field_4_right = x + tmp;
        field_8_top = y - tmp;
        field_C_bottom = y + tmp;

        this->field_10_low_z = z - FIX16_RECT_HALF_HEIGHT;
        this->field_14_high_z = z + FIX16_RECT_HALF_HEIGHT;
    }

    // Unused since sub_5DF270 calls ComputeCollisionPrism_4204D0 with FIX16_RECT_HALF_HEIGHT. Kept because removing
    // an unused inline can move code in other TUs (docs/matching_quirks.md).
    void ComputeShockPrism(Fix16 x, Fix16 y, Fix16 offset, Fix16 z, const Fix16& half_z)
    {
        offset = offset / 2;

        field_0_left = x.Subtract(offset);
        field_4_right = x.Add(offset);
        field_8_top = y.Subtract(offset);
        field_C_bottom = y.Add(offset);

        field_10_low_z = z.Subtract(half_z);
        field_14_high_z = z.Add(half_z);
    }

    void SetRect(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom)
    {
        this->field_0_left = left;
        this->field_4_right = right;
        this->field_8_top = top;
        this->field_C_bottom = bottom;
    }

    // TODO: None inline version of SetRect_41E350 ??
    void SetRectOutOfLine(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom);
    void MakeRect(Fix16 x, Fix16 y, Fix16 w, Fix16 h);

    bool OverlapsZ(Fix16Rect* pOther)
    {
        return IntervalsOverlap(field_10_low_z, field_14_high_z, pOther->field_10_low_z, pOther->field_14_high_z);
    }

    s32 GetMidZ() const
    {
        return (field_10_low_z.Add(field_14_high_z) / 2).ToInt();
    }

    // 9.6f 0x463690, defined in map_0x370.cpp
    inline bool EdgesCrossSegment_463690(Fix16Point& p1, Fix16Point& p2);

    void SetFromPosSize(Fix16 left, Fix16 top, Fix16 width, Fix16 height)
    {
        field_0_left = left;
        field_8_top = top;
        field_4_right = left + width;
        field_C_bottom = top + height;
    }

    void SetHiLowZ(Fix16 lowZ, Fix16 highZ)
    {
        this->field_10_low_z = lowZ;
        this->field_14_high_z = highZ;
    }

    Fix16Rect(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom, Fix16 z)
    {
        SetRect(left, right, top, bottom);
        field_10_low_z = z - kCollisionPrismHalfHeight_6771E4;
        field_14_high_z = z + kCollisionPrismHalfHeight_6771E4;
    }

    // TODO: Get inline addr
    Fix16Rect(Fix16 x, Fix16 y, Fix16 w, Fix16 h)
    {
        field_0_left = x;
        field_8_top = y;
        field_4_right = w + x;
        field_C_bottom = h + y;
    }

    void DoSetCurrentRect();
    bool IntersectsSpriteRenderingRect(Sprite* pSprite);
    char_type CanRectEnterMovementRegion();
    void ExpandToIncludePoint(Fix16 x, Fix16 y);
    bool RectOverlapsZone(u8 zone_type);
    bool EveryTileMatchesArrowType(s32 arrow_type);

    bool IsPointInRect(Fix16Point* a2)
    {
        return Fix16::IsBetween(field_0_left, field_4_right, a2->x) && Fix16::IsBetween(field_8_top, field_C_bottom, a2->y);
    }

    bool AABBIntersects(Fix16Rect* pOther) const
    {
        return IntervalIntersectsRange(field_0_left, this->field_4_right, pOther->field_0_left, pOther->field_4_right) &&
                IntervalIntersectsRange(field_8_top, this->field_C_bottom, pOther->field_8_top, pOther->field_C_bottom) &&
                IntervalIntersectsRange(field_10_low_z, this->field_14_high_z, pOther->field_10_low_z, pOther->field_14_high_z) ?
            true :
            false;
    }

    Fix16 GetTop()
    {
        return field_8_top;
    }

    Fix16 GetLeft()
    {
        return field_0_left;
    }

    Fix16 GetRight()
    {
        return field_4_right;
    }

    Fix16 GetBottom()
    {
        return field_C_bottom;
    }

    Fix16 GetLowZ()
    {
        return field_10_low_z;
    }

    Fix16 field_0_left;
    Fix16 field_4_right;
    Fix16 field_8_top;
    Fix16 field_C_bottom;

    Fix16 field_10_low_z;
    Fix16 field_14_high_z;
};
