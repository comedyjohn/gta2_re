#include "SpriteRenderer_1C.hpp"
#include "Globals.hpp"
#include "enums.hpp"
#include "error.hpp"
#include "sprite.hpp"
#include <chrono>

// Forward declarations: the functions below are in address order
s32 get_rdtsc_5BEE90();
EXTERN_GLOBAL(u32, dword_705334);

DEFINE_GLOBAL(SpriteRenderer_1C*, gSpriteRenderer_67B580, 0x67B580);
DEFINE_GLOBAL(SpriteTreeNodePool_2EE4*, gSpriteTreeNodePool_705BBC, 0x705BBC);
DEFINE_GLOBAL(SpriteTreeStack_FA4*, gSpriteTreeStack_705BC0, 0x705BC0);
DEFINE_GLOBAL(s32, gDisplayDraw_67B57C, 0x67B57C);
DEFINE_GLOBAL(s32, gDisplayAdd_67B578, 0x67B578);
DEFINE_GLOBAL_INIT(Fix16, kFpOne_67B434, Fix16(1), 0x67B434);
DEFINE_GLOBAL_INIT(Fix16, kFp96_705B80, Fix16(0x180000, 0), 0x705B80);
DEFINE_GLOBAL_INIT(Fix16, kFpZero_705AC4, Fix16(0), 0x705AC4);

// 0x4954f0
void SpriteRenderer_1C::ResetAll_4954F0()
{
    for (auto& layer : field_0_layers)
    {
        layer->Reset_5C5E50();
    }
}

// 0x495510
void SpriteRenderer_1C::DisplayAdd_495510(Sprite* pSprite)
{
    const s32 rdtsc = get_rdtsc_5BEE90();
    if (pSprite->field_1C_zpos >= kFpOne_67B434)
    {
        field_0_layers[pSprite->ComputeZLayer_5A1BD0()]->AddSprite_5C5CF0(pSprite);
    }
    gDisplayAdd_67B578 += get_rdtsc_5BEE90() - rdtsc;
}

// 0x495560
void SpriteRenderer_1C::Draw_495560(s32 layer)
{
    const s32 rdtsc = get_rdtsc_5BEE90();
    field_0_layers[layer - 1]->Draw_5C5DF0();
    gDisplayDraw_67B57C += get_rdtsc_5BEE90() - rdtsc;
}

// 0x4955a0
SpriteRenderer_1C::SpriteRenderer_1C()
{
    for (auto& layer : field_0_layers)
    {
        layer = new SpriteTree_4();
        if (!layer)
        {
            FatalError_4A38C0(Gta2Error::OutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\display.cpp", 121);
        }
    }
}

// 0x495630
SpriteRenderer_1C::~SpriteRenderer_1C()
{
    for (auto& layer : field_0_layers)
    {
        delete layer;
    }
}

// TODO: move
// https://decomp.me/scratch/qe97a
// 0x5BEE90
s32 get_rdtsc_5BEE90()
{
    // The original reads the CPU's cycle counter (rdtsc); nanoseconds stand in for cycles here (only
    // the profiler display reads them)
    const auto t = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<s32>(std::chrono::duration_cast<std::chrono::nanoseconds>(t).count());
}

// Converts a cycle count from get_rdtsc_5BEE90 for the profiler display (dword_705334 is the
// number of cycles per unit)
// 0x5BEED0
s32 sub_5BEED0(s32 cycles)
{
    return static_cast<u32>(cycles) / dword_705334;
}

// https://decomp.me/scratch/qyVgM
// 0x5c5cf0
void SpriteTree_4::AddSprite_5C5CF0(Sprite* pSprite)
{
    SpriteTreeNode_C* pRoot = field_0_pRoot;
    Fix16 node_z;
    Fix16 z_pos;

    if (pSprite->field_28_draw_layer > 9)
    {
        if (pSprite->field_28_draw_layer == 34)
        {
            z_pos = kFp96_705B80;
        }
        else
        {
            z_pos = pSprite->field_1C_zpos;
        }
    }
    else
    {
        z_pos = kFpZero_705AC4;
    }

    SpriteTreeNode_C* pParent = nullptr; // TODO: not initialized before 'for' loop
    for (SpriteTreeNode_C* pIter = pRoot; pIter;)
    {
        Sprite* pSprt = pIter->field_0_pSprite;
        pParent = pIter;
        s32 num = pIter->field_0_pSprite->field_28_draw_layer;
        if (num > 9)
        {
            node_z = pSprt->field_1C_zpos;
        }
        else
        {
            node_z = kFpZero_705AC4;
        }

        if (z_pos < node_z)
        {
            pIter = pIter->field_4_pLeft;
        }
        else
        {
            if (z_pos == node_z)
            {
                if (pSprite->field_28_draw_layer < num)
                {
                    pIter = pIter->field_4_pLeft;
                }
                else
                {
                    if (pSprt == pSprite)
                    {
                        return;
                    }
                    pIter = pIter->field_8_pRight;
                }
            }
            else
            {
                pIter = pIter->field_8_pRight;
            }
        }
    }

    SpriteTreeNode_C* pAllocated = gSpriteTreeNodePool_705BBC->Alloc_4C4B40();
    pAllocated->field_0_pSprite = pSprite;
    pAllocated->field_4_pLeft = nullptr;
    pAllocated->field_8_pRight = nullptr;

    if (!field_0_pRoot)
    {
        field_0_pRoot = pAllocated;
    }
    else
    {
        if (z_pos < node_z || (z_pos == node_z && pSprite->field_28_draw_layer < pParent->field_0_pSprite->field_28_draw_layer))
        {
            pParent->field_4_pLeft = pAllocated;
        }
        else
        {
            pParent->field_8_pRight = pAllocated;
        }
    }
}

// 0x5C5DF0
void SpriteTree_4::Draw_5C5DF0()
{
    SpriteTreeNode_C* pIter = this->field_0_pRoot;
    for (;;)
    {
        while (pIter)
        {
            gSpriteTreeStack_705BC0->Push_4C4B80(pIter);
            pIter = pIter->field_4_pLeft;
        }
        if (gSpriteTreeStack_705BC0->IsEnd_4C4BC0())
        {
            break;
        }

        pIter = gSpriteTreeStack_705BC0->Pop_4C4BA0();
        pIter->field_0_pSprite->Draw_59EFF0();
        pIter = pIter->field_8_pRight;
    }
}

DEFINE_GLOBAL_INIT(u32, dword_705334, 1701493, 0x705334);

// 0x5c5e50
void SpriteTree_4::Reset_5C5E50()
{
    gSpriteTreeNodePool_705BBC->Reset_4C4B70();
    field_0_pRoot = nullptr;
}

// TODO: Doesn't match due to SEH stuff
// 0x5c5e70
SpriteTree_4::SpriteTree_4()
{
    if (!gSpriteTreeNodePool_705BBC)
    {
        gSpriteTreeNodePool_705BBC = new SpriteTreeNodePool_2EE4();
    }

    if (!gSpriteTreeStack_705BC0)
    {
        gSpriteTreeStack_705BC0 = new SpriteTreeStack_FA4();
    }
    Reset_5C5E50();
}

// 0x5c5f10
SpriteTree_4::~SpriteTree_4()
{
    // An explicit dtor call through the global plus operator delete on a saved copy gives the original's
    // test in ecx with esi pushed and loaded inside the if; `delete` keeps the pointer in esi throughout
    if (gSpriteTreeNodePool_705BBC != nullptr)
    {
        SpriteTreeNodePool_2EE4* p = gSpriteTreeNodePool_705BBC;
        gSpriteTreeNodePool_705BBC->~SpriteTreeNodePool_2EE4();
        operator delete(p);
        gSpriteTreeNodePool_705BBC = nullptr;
    }

    if (gSpriteTreeStack_705BC0)
    {
        GTA2_DELETE_AND_NULL(gSpriteTreeStack_705BC0);
    }
}

// 0x5c5f60
SpriteTreeNodePool_2EE4::SpriteTreeNodePool_2EE4()
{
    for (auto& entry : field_0_entries)
    {
        entry.field_0_pSprite = nullptr;
    }
    Reset_4C4B70();
}

// 0x5c5f90
SpriteTreeNodePool_2EE4::~SpriteTreeNodePool_2EE4() = default;