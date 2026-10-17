#pragma once

#include "Ped.hpp"
#include "char.hpp"
#include "core/function.hpp"
#include "core/pool.hpp"

class Camera_0xBC;

class Char_B4_Pool
{
  public:
    Char_B4_Pool() = default;

    ~Char_B4_Pool() = default;

    // inline 0x4355C0
    void DeAllocate(Char_B4* pB4)
    {
        field_0_pool.DeAllocate(pB4);
    }

    PoolBasic<Char_B4, 400> field_0_pool;
};

class Char_8
{
  public:
    bool no_char_ped_420EA0()
    {
        return field_0_char_ped == nullptr;
    }

    void PoolAllocate()
    {
    }

    void PoolDeallocate()
    {
    }

    Ped* field_0_char_ped;
    Char_8* mpNext;
};

class Char_8_Pool
{
  public:
    PoolBasic<Char_8, 100> field_0_pool;

    ~Char_8_Pool()
    {
        field_0_pool.head_ = nullptr;
    }

    Char_8* Allocate_445EF0()
    {
        return field_0_pool.Allocate();
    }

    void DeAllocate_445F00(Char_8* pItem)
    {
        field_0_pool.DeAllocate(pItem);
    }

    Char_8_Pool() = default;
};

class PedManager
{
  public:
    void SpawnDummies_46EB60(Camera_0xBC* pCam);
    void PedsService_4703F0();
    PedManager();
    ~PedManager();
    Ped* SpawnPedAt(Fix16 xpos, Fix16 ypos, Fix16 zpos, u8 remap, Ang16 rotation);
    Ped* SpawnDriver_470B00(Car_BC* pCar);
    Ped* SpawnGangDriver_470BA0(Car_BC* pCar, Gang_144* pGang);
    Ped* CreateDummyDriver_470CC0(Car_BC* pCar);
    Ped* SpawnRunAwayGuy_470D60();
    Ped* SpawnTrainLeaver_470E30();
    Ped* AllocatePed_470F30();
    Ped* ClonePed_470F90(Ped* pSrc);
    void DoIanTest_471060(u16 key);
    Ped* PedById(s32 pedId);

    void Dummies_470330();

    s16 field_0_max_dummy_chars;
    u8 field_2_num_dummy_chars;
    char_type field_3_num_peds_updated;
    char_type field_4_num_script_created_peds;
    u8 field_5_fbi_army_count;
    char_type field_6_num_peds_on_screen;
    char_type field_7_make_all_muggers;
    Sprite* field_8;
};

class PedPool
{
  public:
    // inline on the pool ctors: PedManager::PedManager (0x470650) runs out of inline
    // expansions on the Fix16 argument conversions but still expands these
    PedPool() = default;

    ~PedPool();

    Ped* Allocate()
    {
        return field_0_pool.Allocate();
    }

    // Pool::UpdatePool with the next item stored in both branches
    void UpdatePool_445960()
    {
        field_0_pool.field_x_count_ = 0;

        Ped* pPreviousItem = nullptr;
        Ped* pCurrItem = field_0_pool.prev_;

        while (pCurrItem)
        {
            ++field_0_pool.field_x_count_;

            Ped* pNext = pCurrItem->mpNext;

            if (pCurrItem->PoolUpdate())
            {
                pCurrItem->PoolDeallocate();

                if (pPreviousItem && pPreviousItem->mpNext != pCurrItem)
                {
                    pPreviousItem = nullptr;
                }

                if (pPreviousItem)
                {
                    pPreviousItem->mpNext = pCurrItem->mpNext;
                }
                else
                {
                    if (field_0_pool.prev_ == pCurrItem)
                    {
                        field_0_pool.prev_ = pCurrItem->mpNext;
                    }
                    else
                    {
                        pPreviousItem = field_0_pool.prev_;
                        while (pPreviousItem->mpNext != pCurrItem)
                        {
                            pPreviousItem = pPreviousItem->mpNext;
                        }
                        pPreviousItem->mpNext = pCurrItem->mpNext;
                    }
                }

                pCurrItem->mpNext = field_0_pool.start_;
                field_0_pool.start_ = pCurrItem;
                pCurrItem = pNext;
            }
            else
            {
                pPreviousItem = pCurrItem;
                pCurrItem = pNext;
            }
        }
    }

    Ped* GetFirstPed_435530()
    {
        return field_0_pool.prev_;
    }

    Pool<Ped, 200> field_0_pool;
};

EXTERN_GLOBAL(PedManager*, gPedManager_6787BC);

EXTERN_GLOBAL(PedPool*, gPedPool_6787B8);

EXTERN_GLOBAL(Char_B4_Pool*, gChar_B4_Pool_6FDB44);

EXTERN_GLOBAL(Char_8_Pool*, gChar_8_Pool_678b50);

EXTERN_GLOBAL(u8, gNumPolicePedsInRangeScreen_6787EE);

EXTERN_GLOBAL(u8, byte_61A8A2);
