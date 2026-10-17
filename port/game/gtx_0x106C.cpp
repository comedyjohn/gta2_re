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
void ClearPixelsOfColourInRow(u8* data, int width, u8 clear_target);

DEFINE_GLOBAL(Gtx*, gGtx, 0x703DD4);
DEFINE_GLOBAL(s16, gFontType0, 0x703D9C);
DEFINE_GLOBAL(s16, gFontType2, 0x703D9A);
DEFINE_GLOBAL(s16, gFontType1, 0x703C3E);
DEFINE_GLOBAL(s16, gFontType3, 0x703DA4);
DEFINE_GLOBAL(s16, gFontTypeSlot1Of7, 0x703BAA);
DEFINE_GLOBAL(s16, gFontTypeSlot2Of7, 0x703D98);
DEFINE_GLOBAL(s16, gFontTypeSlot5Of7, 0x703C9C);

// 0x5AA3B0
StyleCarInfo* Gtx::GetCarInfo(u8 idx)
{
    return cari_->car_info_[idx];
}

// 0x5AA3D0
BYTE* Gtx::GetCarRemap(u8 idx)
{
    StyleCarInfo* car_info = cari_->car_info_[idx];
    return &car_info->remap_[car_info->num_remaps_];
}

// 0x5AA3F0
SpriteDelta* Gtx::GetDelta(u16 sprite_idx, u8 delta_idx)
{
    DeltaContainer* del = del_;

    if (sprite_idx >= del->field_0_count)
    {
        return nullptr;
    }

    SpriteDeltas* entry = del->field_4_entries[sprite_idx];

    if (!entry)
    {
        return nullptr;
    }

    if (delta_idx >= entry->field_2_count)
    {
        return nullptr;
    }

    SpriteDelta* delta = &entry->field_4_deltas[delta_idx];

    return delta->len_ != 0 ? delta : nullptr;
}

// note: param type matters
// 0x5AA440
SpriteIndex* Gtx::GetSpriteIndex(u16 idx)
{
    return &sprite_index_[idx];
}

// 0x5AA460
u16 Gtx::GetSpriteTrueIndex(s32 sprite_type, s16 sprite_idx)
{
    u16 result;
    switch (sprite_type)
    {
        case SpriteTypes::kCar:
            result = sprite_idx + sprite_base2_->field_0_car;
            break;
        case SpriteTypes::kPed:
            result = sprite_base2_->field_2_ped + sprite_idx;
            break;
        case SpriteTypes::kCodeObj1:
        case SpriteTypes::kCodeObj2:
            result = sprite_idx + sprite_base2_->field_4_code_obj;
            break;
        case SpriteTypes::kMapObj:
            result = sprite_idx + sprite_base2_->field_6_map_obj;
            break;
        case SpriteTypes::kUser:
            result = sprite_idx + static_cast<s16>(sprite_base2_->field_8_user);
            break;
        case SpriteTypes::kFont:
            result = sprite_idx + sprite_base2_->font;
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
            return sprite_base1_->field_0_car;
        case SpriteTypes::kPed:
            return sprite_base1_->field_2_ped;
        case SpriteTypes::kCodeObj1:
        case SpriteTypes::kCodeObj2:
            return sprite_base1_->field_4_code_obj;
        case SpriteTypes::kMapObj:
            return sprite_base1_->field_6_map_obj;
        case SpriteTypes::kFont:
            return sprite_base1_->font;
        case SpriteTypes::kUser:
            return sprite_base1_->field_8_user;
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
            return palette_base1_->field_0_tile;
        case PaletteTypes::kSprites:
            return palette_base1_->field_2_sprite;
        case PaletteTypes::kCarRemaps:
            return palette_base1_->field_4_car_remap;
        case PaletteTypes::kPedRemaps:
            return palette_base1_->field_6_ped_remap;
        case PaletteTypes::kCodeObjRemaps:
            return palette_base1_->field_8_code_obj_remap;
        case PaletteTypes::kMapObjRemaps:
            return palette_base1_->map_obj_remap;
        case PaletteTypes::kUserRemaps:
            return palette_base1_->user_remap;
        case PaletteTypes::kFontRemaps:
            return palette_base1_->font_remap;
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
            result = pal + palette_base2_->field_0_tile;
            break;
        case PaletteTypes::kSprites:
            result = pal + palette_base2_->field_2_sprite;
            break;
        case PaletteTypes::kCarRemaps:
            result = pal + palette_base2_->field_4_car_remap;
            break;
        case PaletteTypes::kPedRemaps:
            result = pal + palette_base2_->field_6_ped_remap;
            break;
        case PaletteTypes::kCodeObjRemaps:
            result = pal + palette_base2_->field_8_code_obj_remap;
            break;
        case PaletteTypes::kMapObjRemaps:
            result = pal + palette_base2_->map_obj_remap;
            break;
        case PaletteTypes::kUserRemaps:
            result = pal + palette_base2_->user_remap;
            break;
        case PaletteTypes::kFontRemaps:
            result = pal + palette_base2_->font_remap;
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
    return static_cast<BYTE*>(physical_palettes_) + 4 * ((palette_idx & 63) + ((palette_idx & 0xFFFFC0) << 8));
}

// 0x5AA6F0
u16 Gtx::GetPhysPal(u16 pal_id)
{
    return palette_index_->field_0_phys_palette[pal_id];
}

// 0x5AA710
u16 Gtx::GetSpriteIdxFromFont(u16 font_type, s16 offset)
{
    u16 result = font_base_->field_2_base[font_type] + offset;
    if (font_type == font_base_->field_0_font_count - 1)
    {
        if (result >= font_base_total_)
        {
            return font_base_->field_2_base[font_type];
        }
        else
        {
            return result;
        }
    }
    else if (result >= static_cast<u32>(font_base_->field_2_base[font_type + 1]))
    {
        return font_base_->field_2_base[font_type];
    }
    return result;
}

// 0x5AA760
u16 Gtx::GetFontWidth(u16* font_type, wchar_t* str)
{
    if (*font_type >= 101)
    {
        // Standardized width for all letters
        return (*font_type < 201) ? 16 : 32;
    }
    else
    {
        // In this case letters have different widths
        return this->sprite_index_[this->sprite_base2_->font + Gtx::GetSpriteIdxFromFont(*font_type, *str - 33)].width_;
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
    return sprite_index_[sprite_base2_->font + 77 + font_base_->field_2_base[*font_type]].width_;
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
        return this->sprite_index_[this->sprite_base2_->font + 32 + this->font_base_->field_2_base[*font_type]].height_;
    }
}

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define UNIQUE_FUNC printf(__FILE__ LINE_STRING "\n")

// 0x5AA850
bool Gtx::IsTileRemapped(u16 tile_idx)
{
    return tile_->field_0_tile_mapping[tile_idx] != tile_idx;
}

// 0x5AA870
s16 Gtx::GetTileMapping(u16 tile_idx)
{
    return tile_->field_0_tile_mapping[tile_idx];
}

// 0x5AA890
s16 Gtx::GetFirstFreeReservedTileIdx()
{
    // Decreasing order: starts from 1023 down to 992
    u16* i = &tile_->field_0_tile_mapping[1023];
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
    return tiles_;
}

// 0x5AA900
u16 Gtx::GetPhysicalPalettesLen()
{
    return physical_palettes_len_;
}

// 0x5AA910
ObjectInfo* Gtx::GetMapObjectInfo(u16 idx)
{
    if (idx >= map_object_info_len_)
    {
        return nullptr;
    }
    return &map_object_info_[idx];
}

// 0x5AA930
void Gtx::SetTileRemap(u16 tile_idx, s16 tile_val)
{
    tile_->field_0_tile_mapping[tile_idx] = tile_val;
}

// 0x5AA950
void Gtx::InitTileMapping()
{
    tile_ = new TileArray();

    for (u16 tile_num = 0; tile_num < 992; tile_num++)
    {
        tile_->field_0_tile_mapping[tile_num] = tile_num;
    }

    u32 tile_num_2 = 992;
    for (u32 i = 32; i > 0; i--)
    {
        tile_->field_0_tile_mapping[tile_num_2++] = 0;
    }
}

// Better score:
// Current WIP (lower score, but more runtime correct?):
// https://decomp.me/scratch/CqJJm
// 0x5AA9A0
void Gtx::BuildCarInfoContainer(u32 chunk_size)
{
    auto* car_info_iter = reinterpret_cast<StyleCarInfo*>(car_info_);
    u32 total_len = 0;
    u32 idx = 0;
    u8 total_sprite = 0;
    u8 last_car_sprite = 0;

    cari_ = new CarInfoContainer();

    if (cari_ == nullptr)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 821);
    }

    while (total_len < chunk_size)
    {
        if (idx >= 256)
        {
            FatalError_4A38C0(Gta2Error::kFileOverflow, "C:\\Splitting\\Gta2\\Source\\style.cpp", 825);
        }

        if (car_info_iter->w > 0x80U || car_info_iter->h_ > 0x80U || car_info_iter->num_remaps_ > 0x40U)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 826, car_info_iter->model_);
        }

        if (car_info_iter->sprite_ != 0 && car_info_iter->sprite_ != 1)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 827, car_info_iter->model_);
        }

        cari_->car_info_[car_info_iter->model_] = car_info_iter;

        if (car_info_iter->sprite_)
        {
            total_sprite += last_car_sprite;
            last_car_sprite = car_info_iter->sprite_;
        }

        car_info_iter->sprite_ = total_sprite;

        // Offset of num_doors. The door part (2 * num_doors + 1) is its own local, added to the offset at
        // both uses: one expression folds it into a single lea, the original keeps lea / add / add.
        u32 off = car_info_iter->num_remaps_ + 0xE;
        if ((reinterpret_cast<u8*>(car_info_iter))[off] > 5U)
        {
            FatalError_4A38C0(Gta2Error::kInvalidCarModelStyleData, "C:\\Splitting\\Gta2\\Source\\style.cpp", 842, car_info_iter->model_);
        }
        u32 door_len = (reinterpret_cast<u8*>(car_info_iter))[off] * sizeof(DoorInfo) + 1;
        total_len += off + door_len;
        car_info_iter = reinterpret_cast<StyleCarInfo*>(reinterpret_cast<u8*>(car_info_iter) + off + door_len);
        idx++;
    }
    cari_->count_ = idx;
}

// 0x5AAB30
void Gtx::LoadDelx(u32 delx_chunk_size)
{
    DeltaEntry* iter = delta_index_;
    u32 off = 0;
    u32 total = 0;

    for (; off < delx_chunk_size;)
    {
        u32 size = 2 * iter->field_2_delta_count + 4;
        delta_len_ += SPRITE_DELTAS_SIZE(iter->field_2_delta_count);
        iter = reinterpret_cast<DeltaEntry*>(reinterpret_cast<u8*>(iter) + size);
        off += size;
    }

    delta_buffer_ = static_cast<SpriteDeltas*>(Memory::Malloc(delta_len_));
    memset(delta_buffer_, 0, delta_len_); // the entries below leave padding bytes unset (valgrind)
    DeltaEntry* src = delta_index_;
    u32 off2 = 0;
    auto* dst = static_cast<SpriteDeltas*>(delta_buffer_);

    for (; off2 < delx_chunk_size;)
    {
        dst->field_0_which_sprite = src->field_0_which_sprite;
        dst->field_2_count = src->field_2_delta_count;
        dst->field_3_pad = 0;

        for (u32 i = 0; i < src->field_2_delta_count; i++)
        {
            dst->field_4_deltas[i].len_ = src->field_4_delta[i];
            dst->field_4_deltas[i].data_ =
                reinterpret_cast<DeltaStoreEntry*>(static_cast<uptr>(total)); // an offset until the deltas are loaded
            total += dst->field_4_deltas[i].len_;
        }

        u32 size = 2 * src->field_2_delta_count + 4;
        dst = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(dst) + SPRITE_DELTAS_SIZE(dst->field_2_count));
        off2 += size;
        src = reinterpret_cast<DeltaEntry*>(reinterpret_cast<u8*>(src) + size);
    }
}

// https://decomp.me/scratch/vwSG1 TODO: fix this hack
// 0x5AABF0
void Gtx::SetDeltaDataPtrs()
{
#if defined(GTA2_PORT_64BIT)
    // The walk below at sprite_deltas' real layout. It adds the store's address to each offset
    // load_delx_5AAB30 left in field_0_pData, which the original does with 32-bit adds.
    for (u32 offset = 0; offset < static_cast<u32>(delta_len_);)
    {
        auto* deltas = reinterpret_cast<SpriteDeltas*>(static_cast<u8*>(delta_buffer_) + offset);
        for (u32 delta_idx = 0; delta_idx < deltas->field_2_count; delta_idx++)
        {
            SpriteDelta& delta = deltas->field_4_deltas[delta_idx];
            delta.data_ = reinterpret_cast<DeltaStoreEntry*>(reinterpret_cast<u8*>(delta_store_) + reinterpret_cast<sptr>(delta.data_));
        }
        offset += SPRITE_DELTAS_SIZE(deltas->field_2_count);
    }
#else
    s32 size;
    DeltaEntry* i = reinterpret_cast<DeltaEntry*>(delta_buffer_);

    for (u32 offset = 0; offset < static_cast<u32>(delta_len_); offset += size)
    {
        for (u32 delta_idx = 0; delta_idx < i->field_2_delta_count; delta_idx++)
        {
            *reinterpret_cast<u32*>(&i->field_4_delta[4 * delta_idx]) += *reinterpret_cast<u32*>(&delta_store_); // TODO: fix me
        }
        size = 8 * static_cast<u8>(i->field_2_delta_count) + 4;
        i = reinterpret_cast<DeltaEntry*>(reinterpret_cast<char_type*>(i) + size);
    }
#endif
}

// 0x5AAC40
void Gtx::SetSpriteIndexDataPtrs()
{
    u8* field_0_p_data;
    SpriteIndex* sprite_index;

    for (u32 i = 0; i < static_cast<u16>(this->sprite_index_count_); i++)
    {
        field_0_p_data = this->sprite_index_[i].data_;
        sprite_index = &this->sprite_index_[i];
        sprite_index->data_ = &this->sprite_graphics_[static_cast<u32>(reinterpret_cast<uintptr_t>(field_0_p_data))];
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
    auto* iter = static_cast<SpriteDeltas*>(delta_buffer_);
    u16 max_sprite = 0;

    for (; off < static_cast<u32>(delta_len_);)
    {
        if (iter->field_0_which_sprite >= max_sprite)
        {
            max_sprite = iter->field_0_which_sprite;
        }

        u32 size = SPRITE_DELTAS_SIZE(iter->field_2_count);
        off += size;
        iter = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(iter) + size);
    }

    del_ = static_cast<DeltaContainer*>(Memory::Malloc(DELTA_CONTAINER_SIZE(max_sprite + 1)));
    del_->field_0_count = max_sprite + 1;

    for (u32 i = 0; i < del_->field_0_count; i++)
    {
        del_->field_4_entries[i] = nullptr;
    }

    iter = static_cast<SpriteDeltas*>(delta_buffer_);

    for (u32 off2 = 0; off2 < static_cast<u32>(delta_len_);)
    {
        if (del_->field_4_entries[iter->field_0_which_sprite])
        {
            FatalError_4A38C0(Gta2Error::kMultipleSpriteDeltas, "C:\\Splitting\\Gta2\\Source\\style.cpp", 996, iter->field_0_which_sprite);
        }

        del_->field_4_entries[iter->field_0_which_sprite] = iter;

        if (iter->field_0_which_sprite >= max_sprite)
        {
            max_sprite = iter->field_0_which_sprite;
        }

        u32 size = SPRITE_DELTAS_SIZE(iter->field_2_count);
        iter = reinterpret_cast<SpriteDeltas*>(reinterpret_cast<u8*>(iter) + size);
        off2 += size;
    }
}

// 0x5AAD50
void Gtx::LoadCarInfo(u32 cari_chunk_size)
{
    car_info_ = static_cast<StyleCarInfo**>(Memory::Malloc(cari_chunk_size));
    File::GlobalRead(reinterpret_cast<void*>(car_info_), cari_chunk_size);

    BuildCarInfoContainer(cari_chunk_size);
}

// 0x5AAD80
void Gtx::LoadDeltaIndex(u32 delx_chunk_size)
{
    this->delta_index_ = static_cast<DeltaEntry*>(Memory::Malloc(delx_chunk_size));
    File::GlobalRead(delta_index_, delx_chunk_size);

    LoadDelx(delx_chunk_size);
    BuildDeltaContainer();

    ::free(this->delta_index_);
    this->delta_index_ = nullptr;
}

// 0x5AADD0
void Gtx::LoadDeltaStore(u32 dels_chunk_size)
{
    delta_store_ = static_cast<DeltaStoreEntry*>(Memory::Malloc(dels_chunk_size)); // todo: or new?
    File::GlobalRead(delta_store_, dels_chunk_size);
}

// 0x5AADF0
void Gtx::LoadTiles(u32 tile_chunk_len)
{
    tiles_ = static_cast<u8*>(Memory::AlignedMalloc(tile_chunk_len, &aligned_tiles_size_));
    File::GlobalRead(tiles_, tile_chunk_len);
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
    sprite_graphics_ = reinterpret_cast<BYTE*>(Memory::AlignedMalloc(sprg_chunk_len, &sprite_graphics_unaligned_));
    File::GlobalRead(sprite_graphics_, sprg_chunk_len);
}

// 0x5AAE70
void Gtx::LoadPhysicalPalettes(u32 ppal_chunk_size)
{
    physical_palettes_ = Memory::AlignedMalloc(ppal_chunk_size, &physical_palettes_size_);
    File::GlobalRead(physical_palettes_, ppal_chunk_size);
    physical_palettes_len_ = ppal_chunk_size >> 10;
}

// 0x5AAEA0
void Gtx::LoadPaletteIndex(u32 palx_chunk_len)
{
    if (palx_chunk_len != sizeof(PaletteIndex))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1157, palx_chunk_len);
    }

    palette_index_ = new PaletteIndex();
    if (!palette_index_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1159);
    }

    File::GlobalRead(palette_index_, palx_chunk_len);
}

// 0x5AAF00
void Gtx::LoadMapObjectInfo(u32 obji_chunk_len)
{
    map_object_info_ = static_cast<ObjectInfo*>(Memory::Malloc(obji_chunk_len));
    File::GlobalRead(map_object_info_, obji_chunk_len);
    if (obji_chunk_len / 2 >= 0x10000)
    {
        FatalError_4A38C0(Gta2Error::kInvalidNewTileNumber, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1177, obji_chunk_len / 2);
    }

    map_object_info_len_ = obji_chunk_len / 2;
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
    auto* file_entries = static_cast<SpriteIndexFile*>(Memory::Malloc(sprx_chunk_size));
    File::GlobalRead(file_entries, sprx_chunk_size);
    sprite_index_ = static_cast<SpriteIndex*>(Memory::Malloc(entry_count * sizeof(SpriteIndex)));
    memset(sprite_index_, 0, entry_count * sizeof(SpriteIndex));
    for (u32 i = 0; i < entry_count - 1; i++)
    {
        sprite_index_[i].data_ = reinterpret_cast<BYTE*>(static_cast<sptr>(file_entries[i].field_0_offset));
        sprite_index_[i].width_ = file_entries[i].field_4_width;
        sprite_index_[i].height_ = file_entries[i].field_5_height;
        sprite_index_[i].pad_ = file_entries[i].field_6_pad;
    }
    ::free(file_entries);
#else
    sprite_index_ = reinterpret_cast<SpriteIndex*>(Memory::Malloc(sprx_chunk_size + 8));
    File::GlobalRead(sprite_index_, sprx_chunk_size);
#endif
    if ((sprx_chunk_size & ~7U) > 524280)
    {
        FatalError_4A38C0(Gta2Error::kTooManySprites, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1198, sprx_chunk_size >> 3);
    }
    sprite_index_count_ = (sprx_chunk_size >> 3) + 1;
}

// 0x5AAFE0
void Gtx::InitFontTypes(u16 font_count)
{
    if (font_count == 14)
    {
        gFontType0 = 0;
        gFontType1 = 1;
        gFontType2 = 2;
        gFontType3 = 3;
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
        gFontTypeSlot1Of7 = 99;
        gFontTypeSlot2Of7 = 99;
        gFontTypeSlot5Of7 = 99;
    }
    else if (font_count == 7)
    {
        gFontType0 = 0;
        gFontTypeSlot1Of7 = 1;
        gFontTypeSlot2Of7 = 2;
        gFontType1 = 3;
        gFontType2 = 4;
        gFontTypeSlot5Of7 = 5;
        gFontType3 = 6;
    }
    else
    {
        FatalError_4A38C0(Gta2Error::kStyleFileContainsUnexpectedNumberOfFonts, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1251, font_count);
    }
}

// 0x5AB0F0
void Gtx::LoadFontBase(u32 fonb_chunk_size)
{
    if (fonb_chunk_size < 2)
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1268, fonb_chunk_size);
    }

    font_base_ = static_cast<FontBase*>(Memory::Malloc(fonb_chunk_size));
    File::GlobalRead(font_base_, fonb_chunk_size);

    u16 fount_count = font_base_->field_0_font_count;
    if (fonb_chunk_size != (fount_count * 2U) + 2)
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1271, fonb_chunk_size);
    }

    font_base_total_ = SumOffsets(font_base_->field_2_base, font_base_->field_0_font_count);

    ConvertToVirtualOffsets(font_base_->field_2_base, font_base_->field_0_font_count);

    InitFontTypes(font_base_->field_0_font_count);
}

// 0x5AB1A0
u16 Gtx::SumOffsets(u16* offsets, u32 offsets_count)
{
    u16 total = 0;
    while (offsets_count > 0) // jbe vs jz due to missing > 0 and signed vs unsigned
    {
        // note: mov si, [ecx] also did xor esi, esi because
        // total was 32bits not 16bits, then an additional bitmask of 0xFFFF was done
        // because the return value was 32bits rather than 16bits.
        total += *offsets;
        offsets++;
        offsets_count--;
    }
    return total;
}

// 0x5AB1C0
void Gtx::ConvertToVirtualOffsets(u16* buffer, u32 len)
{
    for (s32 i = len - 1; i > 0; i--)
    {
        buffer[i] = buffer[i - 1];
    }
    if (len != 0)
    {
        buffer[0] = 0;
    }
    s32 j = 1;
    for (s32 k = len; k > 1; k--, j++)
    {
        buffer[j] += buffer[j - 1];
    }
}

// 0x5AB210
void Gtx::LoadSpriteBase(u32 sprite_base_chunk_size)
{
    if (sprite_base_chunk_size != sizeof(SpriteBase))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1289, sprite_base_chunk_size);
    }

    sprite_base1_ = new SpriteBase();
    if (!sprite_base1_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1291);
    }

    sprite_base2_ = new SpriteBase();
    if (!sprite_base2_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1293);
    }

    File::GlobalRead(sprite_base1_, sprite_base_chunk_size);

    *sprite_base2_ = *sprite_base1_;

    ConvertToVirtualOffsets(&sprite_base2_->field_0_car, 6);
}

// 0x5AB2C0
void Gtx::LoadPaleteBase(u32 palette_base_chunk_len)
{
    if (palette_base_chunk_len != sizeof(PaletteBase))
    {
        FatalError_4A38C0(Gta2Error::kInvalidChunkSize, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1311, palette_base_chunk_len);
    }

    palette_base1_ = new PaletteBase();
    if (!palette_base1_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1313);
    }

    palette_base2_ = new PaletteBase();
    if (!palette_base2_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\style.cpp", 1315);
    }

    File::GlobalRead(palette_base1_, palette_base_chunk_len);

    *palette_base2_ = *palette_base1_;

    total_pal_base_ = SumOffsets(&palette_base2_->field_0_tile, 8);
    ConvertToVirtualOffsets(&palette_base2_->field_0_tile, 8);
}

// 0x5AB380
bool Gtx::IsCarModelInRecycleList(u8 car_id)
{
    // Certain car models are in the recycle list only for certain stages/districts
    s32 i = 0;
    if (!car_recycling_info_)
    {
        return true;
    }

    // This while only ends if it finds 'car_id' or if it reaches the
    // sentinel value 255, which is surely greater than 'car_id'
    while (car_recycling_info_[i] < car_id)
    {
        i++;
    }
    return car_recycling_info_[i] == car_id;
}

// 0x5AB3C0
void Gtx::LoadCarRecyclingInfo(u32 recy_chunk_size)
{
    car_recycling_info_ = static_cast<u8*>(Memory::Malloc(recy_chunk_size));
    recy_chunk_size_ = recy_chunk_size;
    File::GlobalRead(car_recycling_info_, recy_chunk_size);
}

// 0x5AB3F0
void Gtx::ReadSpec(u32 value_to_set)
{
    u16 index_to_set;
    u32 len = 2;
    File::GlobalRead(&index_to_set, len);
    while (index_to_set)
    {
        this->spec_[index_to_set] = value_to_set;
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
void Gtx::LoadChunk(const char_type* str1, u32 chunk_len)
{
    if (!strncmp(str1, "PALB", 4U))
    {
        LoadPaleteBase(chunk_len);
    }
    else if (!strncmp(str1, "SPRB", 4U))
    {
        LoadSpriteBase(chunk_len);
    }
    else if (!strncmp(str1, "FONB", 4U))
    {
        LoadFontBase(chunk_len);
    }
    else if (!strncmp(str1, "SPRX", 4U))
    {
        LoadSpriteIndex(chunk_len);
    }
    else if (!strncmp(str1, "OBJI", 4U))
    {
        LoadMapObjectInfo(chunk_len);
    }
    else if (!strncmp(str1, "PALX", 4U))
    {
        LoadPaletteIndex(chunk_len);
    }
    else if (!strncmp(str1, "PPAL", 4U))
    {
        LoadPhysicalPalettes(chunk_len);
    }
    else if (!strncmp(str1, "SPRG", 4U))
    {
        LoadSpriteGraphics(chunk_len);
    }
    else if (!strncmp(str1, "TILE", 4U))
    {
        LoadTiles(chunk_len);
    }
    else if (!strncmp(str1, "DELS", 4U))
    {
        LoadDeltaStore(chunk_len);
    }
    else if (!strncmp(str1, "DELX", 4U))
    {
        LoadDeltaIndex(chunk_len);
    }
    else if (!strncmp(str1, "CARI", 4U))
    {
        LoadCarInfo(chunk_len);
    }
    else if (!strncmp(str1, "PSXT", 4U))
    {
        SkipPsxt(chunk_len);
    }
    else if (!strncmp(str1, "OVLY", 4U))
    {
        SkipOvly(chunk_len);
    }
    else if (!strncmp(str1, "SPEC", 4U))
    {
        LoadSpec();
    }
    else if (!strncmp(str1, "RECY", 4U))
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
    if (delta_buffer_ && delta_store_)
    {
        SetDeltaDataPtrs();
    }

    if (sprite_index_ && sprite_graphics_)
    {
        SetSpriteIndexDataPtrs();
    }
}

// 0x5AB750
void Gtx::LoadSty(const char_type* sty_file_name)
{
    File::GlobalOpen(sty_file_name);

    FileHeader sty_header;
    u32 len = sizeof(FileHeader);
    File::GlobalRead(sty_header.type_, len);

    sty_header.VerifyType("GBST");
    sty_header.VerifyVersion(700);

    ChunkHeader chunk_header;
    for (len = sizeof(ChunkHeader); File::GlobalRead(&chunk_header, &len); len = sizeof(ChunkHeader))
    {
        if (chunk_header.size_ != 0)
        {
            LoadChunk(chunk_header.type_, chunk_header.size_);
        }
    }

    File::GlobalClose();

    SetDataPtrs();
}

// 0x5AB820
Gtx::Gtx()
{
    ResetSpecs();

    car_recycling_info_ = nullptr;
    recy_chunk_size_ = 0;
    sprite_base2_ = nullptr;
    palette_base2_ = nullptr;
    font_base_ = nullptr;
    sprite_index_ = nullptr;
    map_object_info_ = nullptr;
    map_object_info_len_ = 0;
    palette_index_ = nullptr;
    physical_palettes_ = nullptr;
    physical_palettes_size_ = nullptr;
    tiles_ = nullptr;
    aligned_tiles_size_ = nullptr;
    sprite_graphics_ = nullptr;
    sprite_graphics_unaligned_ = nullptr;
    delta_store_ = nullptr;
    delta_buffer_ = nullptr;
    car_info_ = nullptr;
    cari_ = nullptr;
    delta_index_ = nullptr;
    delta_len_ = 0;
    del_ = nullptr;
    sprite_index_count_ = 0;
    total_pal_base_ = 0;
    font_base_total_ = 0;
    tile_ = nullptr;
    physical_palettes_len_ = 0;
    palettes_converted_ = 0;
    palette_base1_ = nullptr;
    sprite_base1_ = nullptr;
}

// 0x5AB8A0
Gtx::~Gtx()
{
    SpriteBase* local_field_14_sprite_base2; // eax
    SpriteBase* local_field_18_sprite_base1; // ecx
    PaletteBase* local_field_c_palette_base2; // edx
    PaletteBase* local_field_10_palette_base1; // eax
    FontBase* local_field_1_c_font_base; // eax
    SpriteIndex* local_field_20_sprite_index; // eax
    ObjectInfo* local_field_24_map_object_info; // eax
    PaletteIndex* local_field_28_palette_index; // ecx
    void* local_field_30_physical_palettes_size; // eax
    void* local_field_44_aligned_tiles_size; // eax
    void* unaligned; // eax
    DeltaStoreEntry* local_field_48_delta_store; // eax
    void* local_field_50_delta_buffer; // eax
    void* local_field_58_car_info; // eax
    CarInfoContainer* local_field_5_c_cari; // edx
    DeltaEntry* local_field_4_c_delta_index; // eax
    void* local_field_54_del; // eax
    TileArray* local_field_40_tile; // eax

    if (car_recycling_info_)
    {
        ::free(car_recycling_info_);
    }

    local_field_14_sprite_base2 = sprite_base2_;
    car_recycling_info_ = nullptr;
    delete local_field_14_sprite_base2;

    local_field_18_sprite_base1 = sprite_base1_;
    sprite_base2_ = nullptr;
    delete (local_field_18_sprite_base1);

    local_field_c_palette_base2 = palette_base2_;
    sprite_base1_ = nullptr;
    delete (local_field_c_palette_base2);
    local_field_10_palette_base1 = palette_base1_;
    palette_base2_ = nullptr;
    delete (local_field_10_palette_base1);
    local_field_1_c_font_base = font_base_;
    palette_base1_ = nullptr;
    if (local_field_1_c_font_base)
    {
        ::free(local_field_1_c_font_base);
    }

    local_field_20_sprite_index = sprite_index_;
    font_base_ = nullptr;
    if (local_field_20_sprite_index)
    {
        ::free(local_field_20_sprite_index);
    }
    local_field_24_map_object_info = map_object_info_;
    sprite_index_ = nullptr;
    if (local_field_24_map_object_info)
    {
        ::free(local_field_24_map_object_info);
    }
    local_field_28_palette_index = palette_index_;
    map_object_info_ = nullptr;
    delete (local_field_28_palette_index);
    local_field_30_physical_palettes_size = physical_palettes_size_;
    palette_index_ = nullptr;
    if (local_field_30_physical_palettes_size)
    {
        ::free(local_field_30_physical_palettes_size);
    }
    local_field_44_aligned_tiles_size = aligned_tiles_size_;
    physical_palettes_ = nullptr;
    physical_palettes_size_ = nullptr;
    if (local_field_44_aligned_tiles_size)
    {
        ::free(local_field_44_aligned_tiles_size);
    }
    unaligned = sprite_graphics_unaligned_;
    tiles_ = nullptr;
    aligned_tiles_size_ = nullptr;
    if (unaligned)
    {
        ::free(unaligned);
    }
    local_field_48_delta_store = delta_store_;
    sprite_graphics_ = nullptr;
    sprite_graphics_unaligned_ = nullptr;
    if (local_field_48_delta_store)
    {
        ::free(local_field_48_delta_store);
    }
    local_field_50_delta_buffer = delta_buffer_;
    delta_store_ = nullptr;
    if (local_field_50_delta_buffer)
    {
        ::free(local_field_50_delta_buffer);
    }
    local_field_58_car_info = reinterpret_cast<void*>(car_info_);
    delta_buffer_ = nullptr;
    if (local_field_58_car_info)
    {
        ::free(local_field_58_car_info);
    }
    local_field_5_c_cari = cari_;
    car_info_ = nullptr;
    delete (local_field_5_c_cari);
    local_field_4_c_delta_index = delta_index_;
    cari_ = nullptr;
    if (local_field_4_c_delta_index)
    {
        ::free(local_field_4_c_delta_index);
    }
    local_field_54_del = static_cast<void*>(del_);
    delta_index_ = nullptr;
    if (local_field_54_del)
    {
        ::free(local_field_54_del);
    }
    local_field_40_tile = tile_;
    del_ = nullptr;
    delete (local_field_40_tile);
    tile_ = nullptr;
}

// 0x5ABA00
void SpriteDelta::Delta(u8* array)
{
    // Inline asm in the original (lodsw, rep movsb). Each entry is a u16 offset from the end
    // of the previous run, a u8 length and that many bytes.
    // The asm below, in C++ for compilers without MSVC inline asm
    u8* src = reinterpret_cast<u8*>(data_);
    u8* end = src + len_;
    u8* dst = array;
    do
    {
        u16 offset;
        memcpy(&offset, src, sizeof(offset));
        src += 2;
        u8 len = *src++;
        dst += offset;
        memcpy(dst, src, len);
        dst += len;
        src += len;
    } while (src != end);
}

// https://decomp.me/scratch/Cc0Dx Not fully working
// 0x5ABA40
void SpriteDelta::Delta(u8* array, u32 width)
{
    // Inline asm in the original (lodsw / loop)
    // Each entry: u16 offset, u8 len, then len bytes written backwards from the end of the row
    // The asm below, in C++ for compilers without MSVC inline asm
    u8* src = reinterpret_cast<u8*>(data_);
    u8* end = src + len_;
    u8* dst = array - 1 + width;
    do
    {
        u16 offset;
        memcpy(&offset, src, sizeof(offset));
        src += 2;
        u32 len = *src;
        dst += offset;
        u32 mirrored = static_cast<u32>(offset & 0xFF) << 1;
        dst -= mirrored;
        if (mirrored >= 0x100)
        {
            dst += 0x200;
        }
        src++;
        do // like the loop instruction, a zero length wraps
        {
            *dst-- = *src++;
        } while (--len);
    } while (src != end);
}

// 0x5abaa0
void SpriteIndex::ClearPixelsOfColour(u8 clear_target)
{
    for (s32 i = 0; i < height_; i++)
    {
        ClearPixelsOfColourInRow(&this->data_[i * 256], this->width_, clear_target);
    }
}

// 0x5ABAE0
void ClearPixelsOfColourInRow(u8* data, int width, u8 clear_target)
{
    u8* data_iter = data;
    for (s32 i = 0; i < width; i++)
    {
        if (*data_iter == clear_target)
        {
            *data_iter = 0;
        }
        data_iter++;
    }
}

// 0x5abb00
void SpriteIndex::CopyPixels(u8* src)
{
    for (s32 ypos = 0; ypos < height_; ypos++)
    {
        s32 idx = ypos << 8;
        memcpy(&src[idx], &data_[idx], width_);
    }
}