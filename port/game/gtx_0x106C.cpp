#include "gtx_0x106C.hpp"
#include "Frontend.hpp" // TODO: for some globals, prob should be put some where else
#include "chunk.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "core/memory.hpp"
#include "error.hpp"
#include "file.hpp"
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// Forward declarations: the functions below are in address order
void ClearPixelsOfColourInRow(u8* pData, int width, u8 clear_target);

DEFINE_GLOBAL(Gtx*, gGtx_0x106C_703DD4, 0x703DD4);
DEFINE_GLOBAL(s16, word_703D9C, 0x703D9C);
DEFINE_GLOBAL(s16, word_703D9A, 0x703D9A);
DEFINE_GLOBAL(s16, word_703C3E, 0x703C3E);
DEFINE_GLOBAL(s16, word_703DA4, 0x703DA4);
DEFINE_GLOBAL(s16, word_703BAA, 0x703BAA);
DEFINE_GLOBAL(s16, word_703D98, 0x703D98);
DEFINE_GLOBAL(s16, word_703C9C, 0x703C9C);

// 0x5AA3B0
StyleCarInfo* Gtx::GetCarInfo(u8 idx)
{
    return field_5C_cari->field_0_car_info[idx];
}

// 0x5AA3D0
BYTE* Gtx::GetCarRemap(u8 idx)
{
    StyleCarInfo* pCarInfo = field_5C_cari->field_0_car_info[idx];
    return &pCarInfo->remap[pCarInfo->num_remaps];
}

// 0x5AA3F0
SpriteDelta* Gtx::GetDelta(u16 sprite_idx, u8 delta_idx)
{
    DeltaContainer* pDel = field_54_del;

    if (sprite_idx >= pDel->field_0_count)
    {
        return nullptr;
    }

    SpriteDeltas* pEntry = pDel->field_4_entries[sprite_idx];

    if (!pEntry)
    {
        return nullptr;
    }

    if (delta_idx >= pEntry->field_2_count)
    {
        return nullptr;
    }

    SpriteDelta* pDelta = &pEntry->field_4_deltas[delta_idx];

    return pDelta->field_4_len != 0 ? pDelta : nullptr;
}

// note: param type matters
// 0x5AA440
SpriteIndex* Gtx::GetSpriteIndex(u16 idx)
{
    return &field_20_sprite_index[idx];
}

// 0x5AA460
u16 Gtx::GetSpriteTrueIndex(s32 sprite_type, s16 sprite_idx)
{
    u16 result;
    switch (sprite_type)
    {
        case SpriteTypes::kCar:
            result = sprite_idx + field_14_sprite_base2->field_0_car;
            break;
        case SpriteTypes::kPed:
            result = field_14_sprite_base2->field_2_ped + sprite_idx;
            break;
        case SpriteTypes::kCodeObj1:
        case SpriteTypes::kCodeObj2:
            result = sprite_idx + field_14_sprite_base2->field_4_code_obj;
            break;
        case SpriteTypes::kMapObj:
            result = sprite_idx + field_14_sprite_base2->field_6_map_obj;
            break;
        case SpriteTypes::kUser:
            result = sprite_idx + static_cast<s16>(field_14_sprite_base2->field_8_user);
            break;
        case SpriteTypes::kFont:
            result = sprite_idx + field_14_sprite_base2->field_A_font;
            break;
        default:
            result = sprite_idx;
            break;
    }
    return result;
}

// 0x5AA4F0
s16 Gtx::GetSpriteBaseOfType(s32 sprite_type)
{
    switch (sprite_type)
    {
        case SpriteTypes::kCar:
            return field_18_sprite_base1->field_0_car;
        case SpriteTypes::kPed:
            return field_18_sprite_base1->field_2_ped;
        case SpriteTypes::kCodeObj1:
        case SpriteTypes::kCodeObj2:
            return field_18_sprite_base1->field_4_code_obj;
        case SpriteTypes::kMapObj:
            return field_18_sprite_base1->field_6_map_obj;
        case SpriteTypes::kFont:
            return field_18_sprite_base1->field_A_font;
        case SpriteTypes::kUser:
            return field_18_sprite_base1->field_8_user;
        default:
            return 0;
    }
}

// 0x5AA560
s16 Gtx::GetPaletteBaseOfType(s32 palette_type)
{
    switch (palette_type)
    {
        case PaletteTypes::kTiles:
            return field_10_palette_base1->field_0_tile;
        case PaletteTypes::kSprites:
            return field_10_palette_base1->field_2_sprite;
        case PaletteTypes::kCarRemaps:
            return field_10_palette_base1->field_4_car_remap;
        case PaletteTypes::kPedRemaps:
            return field_10_palette_base1->field_6_ped_remap;
        case PaletteTypes::kCodeObjRemaps:
            return field_10_palette_base1->field_8_code_obj_remap;
        case PaletteTypes::kMapObjRemaps:
            return field_10_palette_base1->field_A_map_obj_remap;
        case PaletteTypes::kUserRemaps:
            return field_10_palette_base1->field_C_user_remap;
        case PaletteTypes::kFontRemaps:
            return field_10_palette_base1->field_E_font_remap;
        default:
            return 0;
    }
}

// 0x5AA5F0
s16 Gtx::GetTruePalette(s32 palette_type, s16 pal)
{
    s16 result; // ax

    switch (palette_type)
    {
        case PaletteTypes::kTiles:
            result = pal + field_C_palette_base2->field_0_tile;
            break;
        case PaletteTypes::kSprites:
            result = pal + field_C_palette_base2->field_2_sprite;
            break;
        case PaletteTypes::kCarRemaps:
            result = pal + field_C_palette_base2->field_4_car_remap;
            break;
        case PaletteTypes::kPedRemaps:
            result = pal + field_C_palette_base2->field_6_ped_remap;
            break;
        case PaletteTypes::kCodeObjRemaps:
            result = pal + field_C_palette_base2->field_8_code_obj_remap;
            break;
        case PaletteTypes::kMapObjRemaps:
            result = pal + field_C_palette_base2->field_A_map_obj_remap;
            break;
        case PaletteTypes::kUserRemaps:
            result = pal + field_C_palette_base2->field_C_user_remap;
            break;
        case PaletteTypes::kFontRemaps:
            result = pal + field_C_palette_base2->field_E_font_remap;
            break;
        default:
            result = pal;
            break;
    }
    return result;
}

// 0x5AA6A0
BYTE* Gtx::GetPalData(u16 palette_idx)
{
    return static_cast<BYTE*>(field_2C_physical_palettes) + 4 * ((palette_idx & 63) + ((palette_idx & 0xFFFFC0) << 8));
}

// 0x5AA6F0
u16 Gtx::GetPhysPal(u16 palId)
{
    return field_28_palette_index->field_0_phys_palette[palId];
}

// 0x5AA710
u16 Gtx::GetSpriteIdxFromFont(u16 font_type, s16 offset)
{
    u16 result = field_1C_font_base->field_2_base[font_type] + offset;
    if (font_type == field_1C_font_base->field_0_font_count - 1)
    {
        if (result >= field_2_font_base_total)
        {
            return field_1C_font_base->field_2_base[font_type];
        }
        else
        {
            return result;
        }
    }
    else if (result >= static_cast<u32>(field_1C_font_base->field_2_base[font_type + 1]))
    {
        return field_1C_font_base->field_2_base[font_type];
    }
    return result;
}

// 0x5AA760
u16 Gtx::GetFontWidth(u16* font_type, wchar_t* pStr)
{
    if (*font_type >= 101)
    {
        // Standardized width for all letters
        return (*font_type < 201) ? 16 : 32;
    }
    else
    {
        // In this case letters have different widths
        return this->field_20_sprite_index[this->field_14_sprite_base2->field_A_font + Gtx::GetSpriteIdxFromFont(*font_type, *pStr - 33)]
            .field_4_width;
    }
}

// 0x5AA7B0
u16 Gtx::GetSpaceCharWidth(u16* font_type)
{
    if (*font_type >= 101)
    {
        // Standardized width for all space characters
        return (*font_type < 201) ? 16 : 32;
    }

    // In this case space characters have different widths for different font types below 101
    return field_20_sprite_index[field_14_sprite_base2->field_A_font + 77 + field_1C_font_base->field_2_base[*font_type]].field_4_width;
}

// 0x5AA800
s16 Gtx::GetLineSpacing(u16* font_type)
{
    if (*font_type >= 0x65U)
    {
        return (*font_type < 0xC9U) ? 0x11 : 0x22;
    }
    else
    {
        return this
            ->field_20_sprite_index[this->field_14_sprite_base2->field_A_font + 32 + this->field_1C_font_base->field_2_base[*font_type]]
            .field_5_height;
    }
}

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define UNIQUE_FUNC printf(__FILE__ LINE_STRING "\n")

// 0x5AA850
bool Gtx::IsTileRemapped(u16 tile_idx)
{
    return field_40_tile->field_0_tile_mapping[tile_idx] != tile_idx;
}

// 0x5AA870
s16 Gtx::GetTileMapping(u16 tile_idx)
{
    return field_40_tile->field_0_tile_mapping[tile_idx];
}

// 0x5AA890
s16 Gtx::GetFirstFreeReservedTileIdx()
{
    // Decreasing order: starts from 1023 down to 992
    u16* i = &field_40_tile->field_0_tile_mapping[1023];
    for (u16 j = 1023; j >= 992; j--, i--)
    {
        if (*i == 0)
        {
            return j;
        }
    }

    return 0;
}

// 0x5AA8C0
void* Gtx::GetTiles()
{
    return field_3C_tiles;
}

// 0x5AA900
u16 Gtx::GetPhysicalPalettesLen()
{
    return field_8_physical_palettes_len;
}

// 0x5AA910
ObjectInfo* Gtx::GetMapObjectInfo(u16 idx)
{
    if (idx >= field_6_map_object_info_len)
    {
        return nullptr;
    }
    return &field_24_map_object_info[idx];
}

// 0x5AA930
void Gtx::SetTileRemap(u16 tile_idx, s16 tile_val)
{
    field_40_tile->field_0_tile_mapping[tile_idx] = tile_val;
}

// 0x5AA950
void Gtx::InitTileMapping()
{
    field_40_tile = new TileArray();

    for (u16 tile_num = 0; tile_num < 992; tile_num++)
    {
        field_40_tile->field_0_tile_mapping[tile_num] = tile_num;
    }

    u32 tile_num_2 = 992;
    for (u32 i = 32; i > 0; i--)
    {
        field_40_tile->field_0_tile_mapping[tile_num_2++] = 0;
    }
}

// Better score:
// Current WIP (lower score, but more runtime correct?):
// https://decomp.me/scratch/CqJJm
// 0x5AA9A0
void Gtx::BuildCarInfoContainer(u32 chunk_size)
{
    auto* pCarInfoIter = reinterpret_cast<StyleCarInfo*>(field_58_car_info);
    u32 total_len = 0;
    u32 idx = 0;
    u8 total_sprite = 0;
    u8 last_car_sprite = 0;

    field_5C_cari = new CarInfoContainer();

    if (field_5C_cari == nullptr)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 821);
    }

    while (total_len < chunk_size)
    {
        if (idx >= 256)
        {
            FatalError_4A38C0(Gta2Error::kFileOverflow, "C:\\Splitting\\Gta2\\Source\\style.cpp", 825);
        }

        if (pCarInfoIter->w > 0x80U || pCarInfoIter->h > 0x80U || pCarInfoIter->num_remaps > 0x40U)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 826, pCarInfoIter->model);
        }

        if (pCarInfoIter->sprite != 0 && pCarInfoIter->sprite != 1)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 827, pCarInfoIter->model);
        }

        field_5C_cari->field_0_car_info[pCarInfoIter->model] = pCarInfoIter;

        if (pCarInfoIter->sprite)
        {
            total_sprite += last_car_sprite;
            last_car_sprite = pCarInfoIter->sprite;
        }

        pCarInfoIter->sprite = total_sprite;

        // Offset of num_doors. The door part (2 * num_doors + 1) is its own local, added to the offset at
        // both uses: one expression folds it into a single lea, the original keeps lea / add / add.
        u32 off = pCarInfoIter->num_remaps + 0xE;
        if ((reinterpret_cast<u8*>(pCarInfoIter))[off] > 5U)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 842, pCarInfoIter->model);
        }
        u32 door_len = (reinterpret_cast<u8*>(pCarInfoIter))[off] * sizeof(DoorInfo) + 1;
        total_len += off + door_len;
        pCarInfoIter = reinterpret_cast<StyleCarInfo*>(reinterpret_cast<u8*>(pCarInfoIter) + off + door_len);
        idx++;
    }
    field_5C_cari->field_400_count = idx;
}

// 0x5AAB30
void Gtx::LoadDelx(u32 delx_chunk_size)
{
    DeltaEntry* pIter = field_4C_delta_index;
    u32 off = 0;
    u32 total = 0;

    for (; off < delx_chunk_size;)
    {
        u32 size = 2 * pIter->field_2_delta_count + 4;
        field_60_delta_len += SPRITE_DELTAS_SIZE(pIter->field_2_delta_count);
        pIter = reinterpret_cast<DeltaEntry*>(reinterpret_cast<u8*>(pIter) + size);
        off += size;
    }

    field_50_delta_buffer = static_cast<SpriteDeltas*>(Memory::Malloc(field_60_delta_len));
    memset(field_50_delta_buffer, 0, field_60_delta_len); // the entries below leave padding bytes unset (valgrind)
    DeltaEntry* pSrc = field_4C_delta_index;
    u32 off2 = 0;
    auto* pDst = static_cast<SpriteDeltas*>(field_50_delta_buffer);

    for (; off2 < delx_chunk_size;)
    {
        pDst->field_0_which_sprite = pSrc->field_0_which_sprite;
        pDst->field_2_count = pSrc->field_2_delta_count;
        pDst->field_3_pad = 0;

        for (u32 i = 0; i < pSrc->field_2_delta_count; i++)
        {
            pDst->field_4_deltas[i].field_4_len = pSrc->field_4_delta[i];
            pDst->field_4_deltas[i].field_0_pData =
                reinterpret_cast<DeltaStoreEntry*>(static_cast<uptr>(total)); // an offset until the deltas are loaded
            total += pDst->field_4_deltas[i].field_4_len;
        }

        u32 size = 2 * pSrc->field_2_delta_count + 4;
        pDst = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(pDst) + SPRITE_DELTAS_SIZE(pDst->field_2_count));
        off2 += size;
        pSrc = reinterpret_cast<DeltaEntry*>(reinterpret_cast<u8*>(pSrc) + size);
    }
}

// https://decomp.me/scratch/vwSG1 TODO: fix this hack
// 0x5AABF0
void Gtx::SetDeltaDataPtrs()
{
#if defined(GTA2_PORT_64BIT)
    // The walk below at sprite_deltas' real layout. It adds the store's address to each offset
    // load_delx_5AAB30 left in field_0_pData, which the original does with 32-bit adds.
    for (u32 offset = 0; offset < static_cast<u32>(field_60_delta_len);)
    {
        auto* pDeltas = reinterpret_cast<SpriteDeltas*>(static_cast<u8*>(field_50_delta_buffer) + offset);
        for (u32 delta_idx = 0; delta_idx < pDeltas->field_2_count; delta_idx++)
        {
            SpriteDelta& delta = pDeltas->field_4_deltas[delta_idx];
            delta.field_0_pData = reinterpret_cast<DeltaStoreEntry*>(reinterpret_cast<u8*>(field_48_delta_store) +
                                                                     reinterpret_cast<sptr>(delta.field_0_pData));
        }
        offset += SPRITE_DELTAS_SIZE(pDeltas->field_2_count);
    }
#else
    s32 size;
    DeltaEntry* i = reinterpret_cast<DeltaEntry*>(field_50_delta_buffer);

    for (u32 offset = 0; offset < static_cast<u32>(field_60_delta_len); offset += size)
    {
        for (u32 delta_idx = 0; delta_idx < i->field_2_delta_count; delta_idx++)
        {
            *reinterpret_cast<u32*>(&i->field_4_delta[4 * delta_idx]) += *reinterpret_cast<u32*>(&field_48_delta_store); // TODO: fix me
        }
        size = 8 * static_cast<u8>(i->field_2_delta_count) + 4;
        i = reinterpret_cast<DeltaEntry*>(reinterpret_cast<char_type*>(i) + size);
    }
#endif
}

// 0x5AAC40
void Gtx::SetSpriteIndexDataPtrs()
{
    u8* field_0_pData;
    SpriteIndex* pSpriteIndex;

    for (u32 i = 0; i < static_cast<u16>(this->field_4_sprite_index_count); i++)
    {
        field_0_pData = this->field_20_sprite_index[i].field_0_pData;
        pSpriteIndex = &this->field_20_sprite_index[i];
        pSpriteIndex->field_0_pData = &this->field_34_sprite_graphics[static_cast<u32>(reinterpret_cast<uintptr_t>(field_0_pData))];
    }
}

/*
 //gtx_0x106C *this_; // edi
    u32 idx; // ebx
    car_info *pCarInfoIter; // esi
    u32 total_len; // ebp

   // BYTE new_total_sprite; // dl
    s32 num_remaps; // eax
    s32 next_item_len; // eax
    BYTE total_sprite; // [esp+12h] [ebp-6h]
   // BYTE car_sprite; // [esp+13h] [ebp-5h]

    //this_ = this;
    idx = 0;
    total_len = 0;
    total_sprite = 0;
    BYTE last_car_sprite = 0;
    pCarInfoIter = (car_info *)field_58_car_info;

    car_info_container* pInfo = new car_info_container();
    field_5C_cari = pInfo;                  // 257 "dynamic" array ??
    if (!field_5C_cari)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 821);
    }

    if (chunk_size > 0)
    {
        for (; total_len < chunk_size; idx++)
            //while (1)
        {
            if (idx >= 256)
            {
                FatalError_4A38C0(34, "C:\\Splitting\\Gta2\\Source\\style.cpp", 825);
            }

            if (pCarInfoIter->w > 0x80u || pCarInfoIter->h > 0x80u || pCarInfoIter->num_remaps > 0x40u)
            {
                FatalError_4A38C0(Gta2Error::InvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 826, pCarInfoIter->model);
            }

            BYTE sprite = pCarInfoIter->sprite;
            if (sprite && sprite != 1)
            {
                FatalError_4A38C0(Gta2Error::InvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 827, pCarInfoIter->model);
            }

            pInfo->field_0_car_info[pCarInfoIter->model] = pCarInfoIter;

            if (pCarInfoIter->sprite)
            {
                total_sprite = last_car_sprite + total_sprite;
                last_car_sprite = pCarInfoIter->sprite;
            }

            num_remaps = pCarInfoIter->num_remaps;
            pCarInfoIter->sprite = total_sprite;

            BYTE* pRemaps = pCarInfoIter->remap;
            BYTE* t = pRemaps + num_remaps;
            if (*t > 5u)// num_doors
            {
                FatalError_4A38C0(Gta2Error::InvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 842, pCarInfoIter->model);
            }

            // 0xE = remap
            next_item_len = pCarInfoIter->remap[num_remaps] + sizeof(door_info) * pCarInfoIter->remap[num_remaps] + 1;

            total_len += next_item_len;

            pCarInfoIter = (car_info *)((char_type *)pCarInfoIter + next_item_len);

            //++idx;

            //if (total_len >= chunk_size)
            //{
            //    break;
            //}
            //this_ = this;
        } // loop end

        pInfo->field_400_count = idx;

    }
    else
    {

        field_5C_cari->field_400_count = 0;
    }
*/

// 0x5AAC70
void Gtx::BuildDeltaContainer()
{
    u32 off = 0;
    auto* pIter = static_cast<SpriteDeltas*>(field_50_delta_buffer);
    u16 maxSprite = 0;

    for (; off < static_cast<u32>(field_60_delta_len);)
    {
        if (pIter->field_0_which_sprite >= maxSprite)
        {
            maxSprite = pIter->field_0_which_sprite;
        }

        u32 size = SPRITE_DELTAS_SIZE(pIter->field_2_count);
        off += size;
        pIter = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(pIter) + size);
    }

    field_54_del = static_cast<DeltaContainer*>(Memory::Malloc(DELTA_CONTAINER_SIZE(maxSprite + 1)));
    field_54_del->field_0_count = maxSprite + 1;

    for (u32 i = 0; i < field_54_del->field_0_count; i++)
    {
        field_54_del->field_4_entries[i] = nullptr;
    }

    pIter = static_cast<SpriteDeltas*>(field_50_delta_buffer);

    for (u32 off2 = 0; off2 < static_cast<u32>(field_60_delta_len);)
    {
        if (field_54_del->field_4_entries[pIter->field_0_which_sprite])
        {
            FatalError_4A38C0(Gta2Error::kMultipleSpriteDeltas, "C:\\Splitting\\Gta2\\Source\\style.cpp", 996, pIter->field_0_which_sprite);
        }

        field_54_del->field_4_entries[pIter->field_0_which_sprite] = pIter;

        if (pIter->field_0_which_sprite >= maxSprite)
        {
            maxSprite = pIter->field_0_which_sprite;
        }

        u32 size = SPRITE_DELTAS_SIZE(pIter->field_2_count);
        pIter = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(pIter) + size);
        off2 += size;
    }
}

// 0x5AAD50
void Gtx::LoadCarInfo(u32 cari_chunk_size)
{
    field_58_car_info = static_cast<StyleCarInfo**>(Memory::Malloc(cari_chunk_size));
    File::GlobalRead(reinterpret_cast<void*>(field_58_car_info), cari_chunk_size);

    BuildCarInfoContainer(cari_chunk_size);
}

// 0x5AAD80
void Gtx::LoadDeltaIndex(u32 delx_chunk_size)
{
    this->field_4C_delta_index = static_cast<DeltaEntry*>(Memory::Malloc(delx_chunk_size));
    File::GlobalRead(field_4C_delta_index, delx_chunk_size);

    LoadDelx(delx_chunk_size);
    BuildDeltaContainer();

    ::free(this->field_4C_delta_index);
    this->field_4C_delta_index = nullptr;
}

// 0x5AADD0
void Gtx::LoadDeltaStore(u32 dels_chunk_size)
{
    field_48_delta_store = static_cast<DeltaStoreEntry*>(Memory::Malloc(dels_chunk_size)); // todo: or new?
    File::GlobalRead(field_48_delta_store, dels_chunk_size);
}

// 0x5AADF0
void Gtx::LoadTiles(u32 tile_chunk_len)
{
    field_3C_tiles = static_cast<u8*>(Memory::AlignedMalloc(tile_chunk_len, &field_44_aligned_tiles_size));
    File::GlobalRead(field_3C_tiles, tile_chunk_len);
    InitTileMapping();
}

// 0x5AAE20
void Gtx::SkipOvly(u32 size)
{
    File::GlobalSeek(&size);
}

// 0x5AAE30
void Gtx::SkipPsxt(u32 size)
{
    File::GlobalSeek(&size);
}

// 0x5AAE40
void Gtx::LoadSpriteGraphics(u32 sprg_chunk_len)
{
    field_34_sprite_graphics = reinterpret_cast<BYTE*>(Memory::AlignedMalloc(sprg_chunk_len, &field_38_sprite_graphics_unaligned));
    File::GlobalRead(field_34_sprite_graphics, sprg_chunk_len);
}

// 0x5AAE70
void Gtx::LoadPhysicalPalettes(u32 ppal_chunk_size)
{
    field_2C_physical_palettes = Memory::AlignedMalloc(ppal_chunk_size, &field_30_physical_palettes_size);
    File::GlobalRead(field_2C_physical_palettes, ppal_chunk_size);
    field_8_physical_palettes_len = ppal_chunk_size >> 10;
}

// 0x5AAEA0
void Gtx::LoadPaletteIndex(u32 palx_chunk_len)
{
    if (palx_chunk_len != sizeof(PaletteIndex))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1157, palx_chunk_len);
    }

    field_28_palette_index = new PaletteIndex();
    if (!field_28_palette_index)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1159);
    }

    File::GlobalRead(field_28_palette_index, palx_chunk_len);
}

// 0x5AAF00
void Gtx::LoadMapObjectInfo(u32 obji_chunk_len)
{
    field_24_map_object_info = static_cast<ObjectInfo*>(Memory::Malloc(obji_chunk_len));
    File::GlobalRead(field_24_map_object_info, obji_chunk_len);
    if (obji_chunk_len / 2 >= 0x10000)
    {
        FatalError_4A38C0(Gta2Error::kInvalidNewTileNumber, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1177, obji_chunk_len / 2);
    }

    field_6_map_object_info_len = obji_chunk_len / 2;
    if (((obji_chunk_len / 2) & 0xffff) * 2 != obji_chunk_len)
    {
        FatalError_4A38C0(Gta2Error::kInvalidLoadedObjectInfoData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1179);
    }
}

// 0x5AAF80
void Gtx::LoadSpriteIndex(u32 sprx_chunk_size)
{
#if defined(GTA2_PORT_64BIT)
    // The file's 8-byte entries hold a 4-byte offset where sprite_index has a pointer. Read them as
    // they are and widen them; SetSpriteIndexDataPtrs_5AAC40 turns the offsets into pointers later.
    struct SpriteIndexFile
    {
        u32 field_0_offset;
        u8 field_4_width;
        u8 field_5_height;
        s16 field_6_pad;
    };
    const u32 entry_count = (sprx_chunk_size >> 3) + 1; // plus the spare last entry, as below
    auto* pFileEntries = static_cast<SpriteIndexFile*>(Memory::Malloc(sprx_chunk_size));
    File::GlobalRead(pFileEntries, sprx_chunk_size);
    field_20_sprite_index = static_cast<SpriteIndex*>(Memory::Malloc(entry_count * sizeof(SpriteIndex)));
    memset(field_20_sprite_index, 0, entry_count * sizeof(SpriteIndex));
    for (u32 i = 0; i < entry_count - 1; i++)
    {
        field_20_sprite_index[i].field_0_pData = reinterpret_cast<BYTE*>(static_cast<sptr>(pFileEntries[i].field_0_offset));
        field_20_sprite_index[i].field_4_width = pFileEntries[i].field_4_width;
        field_20_sprite_index[i].field_5_height = pFileEntries[i].field_5_height;
        field_20_sprite_index[i].field_6_pad = pFileEntries[i].field_6_pad;
    }
    ::free(pFileEntries);
#else
    field_20_sprite_index = reinterpret_cast<SpriteIndex*>(Memory::Malloc(sprx_chunk_size + 8));
    File::GlobalRead(field_20_sprite_index, sprx_chunk_size);
#endif
    if ((sprx_chunk_size & ~7U) > 524280)
    {
        FatalError_4A38C0(Gta2Error::kTooManySprites, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1198, sprx_chunk_size >> 3);
    }
    field_4_sprite_index_count = (sprx_chunk_size >> 3) + 1;
}

// 0x5AAFE0
void Gtx::InitFontTypes(u16 fontCount)
{
    if (fontCount == 14)
    {
        word_703D9C = 0;
        word_703C3E = 1;
        word_703D9A = 2;
        word_703DA4 = 3;
        word_703D0C = 4;
        word_703C16 = 5;
        word_703C8C = 6;
        font_type_703C14 = 7;
        word_703C3C = 8;
        word_703C8A = 9;
        word_703BE2 = 10;
        word_703B88 = 11;
        word_703DAC = 12;
        word_703B9C = 13;
        word_703BAA = 99;
        word_703D98 = 99;
        word_703C9C = 99;
    }
    else if (fontCount == 7)
    {
        word_703D9C = 0;
        word_703BAA = 1;
        word_703D98 = 2;
        word_703C3E = 3;
        word_703D9A = 4;
        word_703C9C = 5;
        word_703DA4 = 6;
    }
    else
    {
        FatalError_4A38C0(Gta2Error::kStyleFileContainsUnexpectedNumberOfFonts, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1251, fontCount);
    }
}

// 0x5AB0F0
void Gtx::LoadFontBase(u32 fonb_chunk_size)
{
    if (fonb_chunk_size < 2)
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1268, fonb_chunk_size);
    }

    field_1C_font_base = static_cast<FontBase*>(Memory::Malloc(fonb_chunk_size));
    File::GlobalRead(field_1C_font_base, fonb_chunk_size);

    u16 fountCount = field_1C_font_base->field_0_font_count;
    if (fonb_chunk_size != (fountCount * 2U) + 2)
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1271, fonb_chunk_size);
    }

    field_2_font_base_total = SumOffsets(field_1C_font_base->field_2_base, field_1C_font_base->field_0_font_count);

    ConvertToVirtualOffsets(field_1C_font_base->field_2_base, field_1C_font_base->field_0_font_count);

    InitFontTypes(field_1C_font_base->field_0_font_count);
}

// 0x5AB1A0
u16 Gtx::SumOffsets(u16* pOffsets, u32 offsetsCount)
{
    u16 total = 0;
    while (offsetsCount > 0) // jbe vs jz due to missing > 0 and signed vs unsigned
    {
        // note: mov si, [ecx] also did xor esi, esi because
        // total was 32bits not 16bits, then an additional bitmask of 0xFFFF was done
        // because the return value was 32bits rather than 16bits.
        total += *pOffsets;
        pOffsets++;
        offsetsCount--;
    }
    return total;
}

// 0x5AB1C0
void Gtx::ConvertToVirtualOffsets(u16* pBuffer, u32 len)
{
    for (s32 i = len - 1; i > 0; i--)
    {
        pBuffer[i] = pBuffer[i - 1];
    }
    if (len != 0)
    {
        pBuffer[0] = 0;
    }
    s32 j = 1;
    for (s32 k = len; k > 1; k--, j++)
    {
        pBuffer[j] += pBuffer[j - 1];
    }
}

// 0x5AB210
void Gtx::LoadSpriteBase(u32 sprite_base_chunk_size)
{
    if (sprite_base_chunk_size != sizeof(SpriteBase))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1289, sprite_base_chunk_size);
    }

    field_18_sprite_base1 = new SpriteBase();
    if (!field_18_sprite_base1)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1291);
    }

    field_14_sprite_base2 = new SpriteBase();
    if (!field_14_sprite_base2)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1293);
    }

    File::GlobalRead(field_18_sprite_base1, sprite_base_chunk_size);

    *field_14_sprite_base2 = *field_18_sprite_base1;

    ConvertToVirtualOffsets(&field_14_sprite_base2->field_0_car, 6);
}

// 0x5AB2C0
void Gtx::LoadPaleteBase(u32 palette_base_chunk_len)
{
    if (palette_base_chunk_len != sizeof(PaletteBase))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1311, palette_base_chunk_len);
    }

    field_10_palette_base1 = new PaletteBase();
    if (!field_10_palette_base1)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1313);
    }

    field_C_palette_base2 = new PaletteBase();
    if (!field_C_palette_base2)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1315);
    }

    File::GlobalRead(field_10_palette_base1, palette_base_chunk_len);

    *field_C_palette_base2 = *field_10_palette_base1;

    field_0_totalPalBase = SumOffsets(&field_C_palette_base2->field_0_tile, 8);
    ConvertToVirtualOffsets(&field_C_palette_base2->field_0_tile, 8);
}

// 0x5AB380
bool Gtx::IsCarModelInRecycleList(u8 car_id)
{
    // Certain car models are in the recycle list only for certain stages/districts
    s32 i = 0;
    if (!field_64_car_recycling_info)
    {
        return true;
    }

    // This while only ends if it finds 'car_id' or if it reaches the
    // sentinel value 255, which is surely greater than 'car_id'
    while (field_64_car_recycling_info[i] < car_id)
    {
        i++;
    }
    return field_64_car_recycling_info[i] == car_id;
}

// 0x5AB3C0
void Gtx::LoadCarRecyclingInfo(u32 recy_chunk_size)
{
    field_64_car_recycling_info = static_cast<u8*>(Memory::Malloc(recy_chunk_size));
    field_68_recy_chunk_size = recy_chunk_size;
    File::GlobalRead(field_64_car_recycling_info, recy_chunk_size);
}

// 0x5AB3F0
void Gtx::ReadSpec(u32 value_to_set)
{
    u16 index_to_set;
    u32 len = 2;
    File::GlobalRead(&index_to_set, len);
    while (index_to_set)
    {
        this->field_6C_spec[index_to_set] = value_to_set;
        s32 len2 = 2;
        File::GlobalRead(&index_to_set, len2);
    }
}

// 0x5AB450
void Gtx::LoadSpec()
{
    ReadSpec(2);
    ReadSpec(3);
    ReadSpec(4);
    ReadSpec(5);
    ReadSpec(6);
    ReadSpec(7);
    ReadSpec(8);
    ReadSpec(9);
    ReadSpec(10);
}

// 0x5AB4B0
void Gtx::LoadChunk(const char_type* Str1, u32 chunk_len)
{
    if (!strncmp(Str1, "PALB", 4U))
    {
        LoadPaleteBase(chunk_len);
    }
    else if (!strncmp(Str1, "SPRB", 4U))
    {
        LoadSpriteBase(chunk_len);
    }
    else if (!strncmp(Str1, "FONB", 4U))
    {
        LoadFontBase(chunk_len);
    }
    else if (!strncmp(Str1, "SPRX", 4U))
    {
        LoadSpriteIndex(chunk_len);
    }
    else if (!strncmp(Str1, "OBJI", 4U))
    {
        LoadMapObjectInfo(chunk_len);
    }
    else if (!strncmp(Str1, "PALX", 4U))
    {
        LoadPaletteIndex(chunk_len);
    }
    else if (!strncmp(Str1, "PPAL", 4U))
    {
        LoadPhysicalPalettes(chunk_len);
    }
    else if (!strncmp(Str1, "SPRG", 4U))
    {
        LoadSpriteGraphics(chunk_len);
    }
    else if (!strncmp(Str1, "TILE", 4U))
    {
        LoadTiles(chunk_len);
    }
    else if (!strncmp(Str1, "DELS", 4U))
    {
        LoadDeltaStore(chunk_len);
    }
    else if (!strncmp(Str1, "DELX", 4U))
    {
        LoadDeltaIndex(chunk_len);
    }
    else if (!strncmp(Str1, "CARI", 4U))
    {
        LoadCarInfo(chunk_len);
    }
    else if (!strncmp(Str1, "PSXT", 4U))
    {
        SkipPsxt(chunk_len);
    }
    else if (!strncmp(Str1, "OVLY", 4U))
    {
        SkipOvly(chunk_len);
    }
    else if (!strncmp(Str1, "SPEC", 4U))
    {
        LoadSpec();
    }
    else if (!strncmp(Str1, "RECY", 4U))
    {
        LoadCarRecyclingInfo(chunk_len);
    }
    else
    {
        File::GlobalSeek(&chunk_len);
    }
}

// 0x5AB720
void Gtx::SetDataPtrs()
{
    if (field_50_delta_buffer && field_48_delta_store)
    {
        SetDeltaDataPtrs();
    }

    if (field_20_sprite_index && field_34_sprite_graphics)
    {
        SetSpriteIndexDataPtrs();
    }
}

// 0x5AB750
void Gtx::LoadSty(const char_type* pStyFileName)
{
    File::GlobalOpen(pStyFileName);

    FileHeader styHeader;
    u32 len = sizeof(FileHeader);
    File::GlobalRead(styHeader.field_0_type, len);

    styHeader.VerifyType("GBST");
    styHeader.VerifyVersion(700);

    ChunkHeader chunkHeader;
    for (len = sizeof(ChunkHeader); File::GlobalRead(&chunkHeader, &len); len = sizeof(ChunkHeader))
    {
        if (chunkHeader.field_4_size != 0)
        {
            LoadChunk(chunkHeader.field_0_type, chunkHeader.field_4_size);
        }
    }

    File::GlobalClose();

    SetDataPtrs();
}

// 0x5AB820
Gtx::Gtx()
{
    ResetSpecs();

    field_64_car_recycling_info = nullptr;
    field_68_recy_chunk_size = 0;
    field_14_sprite_base2 = nullptr;
    field_C_palette_base2 = nullptr;
    field_1C_font_base = nullptr;
    field_20_sprite_index = nullptr;
    field_24_map_object_info = nullptr;
    field_6_map_object_info_len = 0;
    field_28_palette_index = nullptr;
    field_2C_physical_palettes = nullptr;
    field_30_physical_palettes_size = nullptr;
    field_3C_tiles = nullptr;
    field_44_aligned_tiles_size = nullptr;
    field_34_sprite_graphics = nullptr;
    field_38_sprite_graphics_unaligned = nullptr;
    field_48_delta_store = nullptr;
    field_50_delta_buffer = nullptr;
    field_58_car_info = nullptr;
    field_5C_cari = nullptr;
    field_4C_delta_index = nullptr;
    field_60_delta_len = 0;
    field_54_del = nullptr;
    field_4_sprite_index_count = 0;
    field_0_totalPalBase = 0;
    field_2_font_base_total = 0;
    field_40_tile = nullptr;
    field_8_physical_palettes_len = 0;
    field_6A_palettes_converted = 0;
    field_10_palette_base1 = nullptr;
    field_18_sprite_base1 = nullptr;
}

// 0x5AB8A0
Gtx::~Gtx()
{
    SpriteBase* local_field_14_sprite_base2; // eax
    SpriteBase* local_field_18_sprite_base1; // ecx
    PaletteBase* local_field_C_palette_base2; // edx
    PaletteBase* local_field_10_palette_base1; // eax
    FontBase* local_field_1C_font_base; // eax
    SpriteIndex* local_field_20_sprite_index; // eax
    ObjectInfo* local_field_24_map_object_info; // eax
    PaletteIndex* local_field_28_palette_index; // ecx
    void* local_field_30_physical_palettes_size; // eax
    void* local_field_44_aligned_tiles_size; // eax
    void* pUnaligned; // eax
    DeltaStoreEntry* local_field_48_delta_store; // eax
    void* local_field_50_delta_buffer; // eax
    void* local_field_58_car_info; // eax
    CarInfoContainer* local_field_5C_cari; // edx
    DeltaEntry* local_field_4C_delta_index; // eax
    void* local_field_54_del; // eax
    TileArray* local_field_40_tile; // eax

    if (field_64_car_recycling_info)
    {
        ::free(field_64_car_recycling_info);
    }

    local_field_14_sprite_base2 = field_14_sprite_base2;
    field_64_car_recycling_info = nullptr;
    delete local_field_14_sprite_base2;

    local_field_18_sprite_base1 = field_18_sprite_base1;
    field_14_sprite_base2 = nullptr;
    delete (local_field_18_sprite_base1);

    local_field_C_palette_base2 = field_C_palette_base2;
    field_18_sprite_base1 = nullptr;
    delete (local_field_C_palette_base2);
    local_field_10_palette_base1 = field_10_palette_base1;
    field_C_palette_base2 = nullptr;
    delete (local_field_10_palette_base1);
    local_field_1C_font_base = field_1C_font_base;
    field_10_palette_base1 = nullptr;
    if (local_field_1C_font_base)
    {
        ::free(local_field_1C_font_base);
    }

    local_field_20_sprite_index = field_20_sprite_index;
    field_1C_font_base = nullptr;
    if (local_field_20_sprite_index)
    {
        ::free(local_field_20_sprite_index);
    }
    local_field_24_map_object_info = field_24_map_object_info;
    field_20_sprite_index = nullptr;
    if (local_field_24_map_object_info)
    {
        ::free(local_field_24_map_object_info);
    }
    local_field_28_palette_index = field_28_palette_index;
    field_24_map_object_info = nullptr;
    delete (local_field_28_palette_index);
    local_field_30_physical_palettes_size = field_30_physical_palettes_size;
    field_28_palette_index = nullptr;
    if (local_field_30_physical_palettes_size)
    {
        ::free(local_field_30_physical_palettes_size);
    }
    local_field_44_aligned_tiles_size = field_44_aligned_tiles_size;
    field_2C_physical_palettes = nullptr;
    field_30_physical_palettes_size = nullptr;
    if (local_field_44_aligned_tiles_size)
    {
        ::free(local_field_44_aligned_tiles_size);
    }
    pUnaligned = field_38_sprite_graphics_unaligned;
    field_3C_tiles = nullptr;
    field_44_aligned_tiles_size = nullptr;
    if (pUnaligned)
    {
        ::free(pUnaligned);
    }
    local_field_48_delta_store = field_48_delta_store;
    field_34_sprite_graphics = nullptr;
    field_38_sprite_graphics_unaligned = nullptr;
    if (local_field_48_delta_store)
    {
        ::free(local_field_48_delta_store);
    }
    local_field_50_delta_buffer = field_50_delta_buffer;
    field_48_delta_store = nullptr;
    if (local_field_50_delta_buffer)
    {
        ::free(local_field_50_delta_buffer);
    }
    local_field_58_car_info = reinterpret_cast<void*>(field_58_car_info);
    field_50_delta_buffer = nullptr;
    if (local_field_58_car_info)
    {
        ::free(local_field_58_car_info);
    }
    local_field_5C_cari = field_5C_cari;
    field_58_car_info = nullptr;
    delete (local_field_5C_cari);
    local_field_4C_delta_index = field_4C_delta_index;
    field_5C_cari = nullptr;
    if (local_field_4C_delta_index)
    {
        ::free(local_field_4C_delta_index);
    }
    local_field_54_del = static_cast<void*>(field_54_del);
    field_4C_delta_index = nullptr;
    if (local_field_54_del)
    {
        ::free(local_field_54_del);
    }
    local_field_40_tile = field_40_tile;
    field_54_del = nullptr;
    delete (local_field_40_tile);
    field_40_tile = nullptr;
}

// 0x5ABA00
void SpriteDelta::Delta(u8* pArray)
{
    // Inline asm in the original (lodsw, rep movsb). Each entry is a u16 offset from the end
    // of the previous run, a u8 length and that many bytes.
    // The asm below, in C++ for compilers without MSVC inline asm
    u8* pSrc = reinterpret_cast<u8*>(field_0_pData);
    u8* pEnd = pSrc + field_4_len;
    u8* pDst = pArray;
    do
    {
        u16 offset;
        memcpy(&offset, pSrc, sizeof(offset));
        pSrc += 2;
        u8 len = *pSrc++;
        pDst += offset;
        memcpy(pDst, pSrc, len);
        pDst += len;
        pSrc += len;
    } while (pSrc != pEnd);
}

// https://decomp.me/scratch/Cc0Dx Not fully working
// 0x5ABA40
void SpriteDelta::Delta(u8* pArray, u32 width)
{
    // Inline asm in the original (lodsw / loop)
    // Each entry: u16 offset, u8 len, then len bytes written backwards from the end of the row
    // The asm below, in C++ for compilers without MSVC inline asm
    u8* pSrc = reinterpret_cast<u8*>(field_0_pData);
    u8* pEnd = pSrc + field_4_len;
    u8* pDst = pArray - 1 + width;
    do
    {
        u16 offset;
        memcpy(&offset, pSrc, sizeof(offset));
        pSrc += 2;
        u32 len = *pSrc;
        pDst += offset;
        u32 mirrored = static_cast<u32>(offset & 0xFF) << 1;
        pDst -= mirrored;
        if (mirrored >= 0x100)
        {
            pDst += 0x200;
        }
        pSrc++;
        do // like the loop instruction, a zero length wraps
        {
            *pDst-- = *pSrc++;
        } while (--len);
    } while (pSrc != pEnd);
}

// 0x5abaa0
void SpriteIndex::ClearPixelsOfColour(u8 clear_target)
{
    for (s32 i = 0; i < field_5_height; i++)
    {
        ClearPixelsOfColourInRow(&this->field_0_pData[i * 256], this->field_4_width, clear_target);
    }
}

// 0x5ABAE0
void ClearPixelsOfColourInRow(u8* pData, int width, u8 clear_target)
{
    u8* pDataIter = pData;
    for (s32 i = 0; i < width; i++)
    {
        if (*pDataIter == clear_target)
        {
            *pDataIter = 0;
        }
        pDataIter++;
    }
}

// 0x5abb00
void SpriteIndex::CopyPixels(u8* src)
{
    for (s32 ypos = 0; ypos < field_5_height; ypos++)
    {
        s32 idx = ypos << 8;
        memcpy(&src[idx], &field_0_pData[idx], field_4_width);
    }
}