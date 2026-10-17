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

        left_ = x - tmp;
        right_ = x + tmp;
        top_ = y - tmp;
        bottom_ = y + tmp;

        this->low_z_ = z - FIX16_RECT_HALF_HEIGHT;
        this->high_z_ = z + FIX16_RECT_HALF_HEIGHT;
    }

    // Unused since sub_5DF270 calls ComputeCollisionPrism_4204D0 with FIX16_RECT_HALF_HEIGHT. Kept because removing
    // an unused inline can move code in other TUs (docs/matching_quirks.md).
    void ComputeShockPrism(Fix16 x, Fix16 y, Fix16 offset, Fix16 z, const Fix16& half_z)
    {
        offset = offset / 2;

        left_ = x.Subtract(offset);
        right_ = x.Add(offset);
        top_ = y.Subtract(offset);
        bottom_ = y.Add(offset);

        low_z_ = z.Subtract(half_z);
        high_z_ = z.Add(half_z);
    }

    void SetRect(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom)
    {
        this->left_ = left;
        this->right_ = right;
        this->top_ = top;
        this->bottom_ = bottom;
    }

    // TODO: None inline version of SetRect_41E350 ??
    void SetRectOutOfLine(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom);
    void MakeRect(Fix16 x, Fix16 y, Fix16 w, Fix16 h);

    bool OverlapsZ(Fix16Rect* pOther)
    {
        return IntervalsOverlap(low_z_, high_z_, pOther->low_z_, pOther->high_z_);
    }

    s32 GetMidZ() const
    {
        return (low_z_.Add(high_z_) / 2).ToInt();
    }

    // 9.6f 0x463690, defined in map_0x370.cpp
    inline bool EdgesCrossSegment_463690(Fix16Point& p1, Fix16Point& p2);

    void SetFromPosSize(Fix16 left, Fix16 top, Fix16 width, Fix16 height)
    {
        left_ = left;
        top_ = top;
        right_ = left + width;
        bottom_ = top + height;
    }

    void SetHiLowZ(Fix16 lowZ, Fix16 highZ)
    {
        this->low_z_ = lowZ;
        this->high_z_ = highZ;
    }

    Fix16Rect(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom, Fix16 z)
    {
        SetRect(left, right, top, bottom);
        low_z_ = z - kCollisionPrismHalfHeight_6771E4;
        high_z_ = z + kCollisionPrismHalfHeight_6771E4;
    }

    // TODO: Get inline addr
    Fix16Rect(Fix16 x, Fix16 y, Fix16 w, Fix16 h)
    {
        left_ = x;
        top_ = y;
        right_ = w + x;
        bottom_ = h + y;
    }

    void DoSetCurrentRect();
    bool IntersectsSpriteRenderingRect(Sprite* pSprite);
    char_type CanRectEnterMovementRegion();
    void ExpandToIncludePoint(Fix16 x, Fix16 y);
    bool RectOverlapsZone(u8 zone_type);
    bool EveryTileMatchesArrowType(s32 arrow_type);

    bool IsPointInRect(Fix16Point* a2)
    {
        return Fix16::IsBetween(left_, right_, a2->x) && Fix16::IsBetween(top_, bottom_, a2->y);
    }

    bool AABBIntersects(Fix16Rect* pOther) const
    {
        return IntervalIntersectsRange(left_, this->right_, pOther->left_, pOther->right_) &&
                IntervalIntersectsRange(top_, this->bottom_, pOther->top_, pOther->bottom_) &&
                IntervalIntersectsRange(low_z_, this->high_z_, pOther->low_z_, pOther->high_z_) ?
            true :
            false;
    }

    Fix16 GetTop()
    {
        return top_;
    }

    Fix16 GetLeft()
    {
        return left_;
    }

    Fix16 GetRight()
    {
        return right_;
    }

    Fix16 GetBottom()
    {
        return bottom_;
    }

    Fix16 GetLowZ()
    {
        return low_z_;
    }

    Fix16 left_;
    Fix16 right_;
    Fix16 top_;
    Fix16 bottom_;

    Fix16 low_z_;
    Fix16 high_z_;
};
