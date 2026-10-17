#pragma once

#include "SpriteGrid_400.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"
#include "core/pool.hpp"

class Sprite;

class GridSpriteLink_8
{
  public:
    void PoolAllocate()
    {
    }

    void PoolDeallocate()
    {
    }

    Sprite* field_0_pSprite;
    GridSpriteLink_8* mpNext;
};

class GridSpriteLink_Pool : public PoolBasic<GridSpriteLink_8, 4096>
{
  public:
    GridSpriteLink_Pool() = default;

    ~GridSpriteLink_Pool()
    {
        this->head_ = nullptr;
    }
};

class GridCell_Pool : public PoolBasic<GridCell_C, 6000>
{
  public:
    GridCell_Pool() = default;

    ~GridCell_Pool()
    {
        this->head_ = nullptr;
    }
};

class CollisionCounters_C
{
  public:
    void ResetCount_478A20();
    CollisionCounters_C();
    ~CollisionCounters_C();
    s32 field_0_test_count;
    s32 field_4_query_id;
    s32 field_8_bUnknown;
};

EXTERN_GLOBAL(CollisionCounters_C*, gCollisionCounters_6791FC);
