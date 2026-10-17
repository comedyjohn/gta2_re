#pragma once

#include "core/enums.hpp"
#include "core/function.hpp"
#include <cstdio>
#include <windows.h>

struct PaletteBase
{
    u16 field_0_tile;
    u16 field_2_sprite;
    u16 field_4_car_remap;
    u16 field_6_ped_remap;
    u16 field_8_code_obj_remap;
    u16 map_obj_remap;
    u16 user_remap;
    u16 font_remap;
};

struct SpriteBase
{
    u16 field_0_car;
    u16 field_2_ped;
    u16 field_4_code_obj;
    u16 field_6_map_obj;
    u16 field_8_user;
    u16 font;
};

struct FontBase
{
    u16 field_0_font_count;
    u16 field_2_base[1]; // variable � see font_count
};

struct SpriteIndex
{
    void ClearPixelsOfColour(u8 clear_target);
    void CopyPixels(u8* dst_x);

    BYTE* data_;
    u8 width_;
    u8 height_;
    s16 pad_;
};

struct ObjectInfo
{
    u8 field_0_model;
    u8 field_1_sprites;
};

struct PaletteIndex
{
    u16 field_0_phys_palette[16384];
};

struct TileArray
{
    u16 field_0_tile_mapping[1024];
};

struct DeltaEntry
{
    s16 field_0_which_sprite;
    u8 field_2_delta_count;
    u8 field_3_pad;
    s16 field_4_delta[2];
};

struct DoorInfo
{
    char_type rx, ry;
};

// car_info::info_flags bits (from how Car_BC uses them)
namespace car_info_flag
{
enum
{
    kEmergency = 0x2, // police/ambulance/FBI style lights, excluded from respray
    kRoofLight = 0x4,
    kTruckCab = 0x8, // can pull a trailer
    kTrailer = 0x10, // is a trailer (not enterable, has a hitch point)
    kTaxiLight = 0x20,
    kBottomRoofLight = 0x40,
    kTopRoofLights = 0x80,
};
} // namespace car_info_flag

// car_info::info_flags_2 bits
namespace car_info_flag2
{
enum
{
    kHeavy = 0x1, // crushes cars, different crash sound and sprite number
    kPopupHeadlights = 0x2,
};
} // namespace car_info_flag2

class StyleCarInfo
{
  public:
    BYTE model_;
    BYTE sprite_;
    BYTE w;
    BYTE h_;
    BYTE num_remaps_;
    BYTE passengers_;
    BYTE wreck_;
    BYTE rating_;
    s8 front_wheel_offset_;
    s8 rear_wheel_offset_;
    s8 front_window_offset_;
    s8 rear_window_offset_;
    BYTE info_flags_;
    BYTE info_flags_2_;
    BYTE remap_[1]; // [variable � see num_remaps];
    BYTE num_doors_;
    DoorInfo doors_[1]; // [variable � see num_doors];

    bool HasRemaps() const
    {
        return num_remaps_ > 1;
    }

    bool IsTruckCab() const
    {
        return (this->info_flags_ & car_info_flag::kTruckCab) == car_info_flag::kTruckCab;
    }

    // Like IsTruckCab_41FEA0; going through a car_info method keeps the check a plain branch in
    // Car_BC::CanBeLiftedByCrane_447F00 (a direct `(flags & 0x10) == 0x10` there gives a sete)
    bool IsTrailer() const
    {
        return (this->info_flags_ & car_info_flag::kTrailer) == car_info_flag::kTrailer;
    }

    bool IsHeavy() const
    {
        return (this->info_flags_2_ & car_info_flag2::kHeavy) == car_info_flag2::kHeavy;
    }
};

struct CarInfoContainer
{
    CarInfoContainer()
    {
        count_ = 0;
        memset(reinterpret_cast<void*>(car_info_), 0, sizeof(car_info_));
    }
    StyleCarInfo* car_info_[256];
    u8 count_;
    //char_type field_401;// pad
    //char_type field_402;// pad
    //char_type field_403;// pad
};

struct DeltaStoreEntry
{
    u16 field_0_offset;
    u8 field_2_len;
    u8 field_3_data[1]; // variable � see field_2_len
};

struct SpriteDelta
{
    void Delta(u8* array);
    void Delta(u8* array, u32 width);

    DeltaStoreEntry* data_;
    u16 len_;
    u16 pad_;
};

struct SpriteDeltas
{
    u16 field_0_which_sprite;
    u8 field_2_count;
    u8 field_3_pad;
    SpriteDelta field_4_deltas[1];
};

struct DeltaContainer
{
    u16 field_0_count;
    u16 field_2_pad;
    SpriteDeltas* field_4_entries[1];
};

// Bytes taken by a sprite_deltas with count deltas, and by a delta_container with count entries. The
// original spells them for 4-byte pointers; at 64 bits the pointers, and the padding before them, grow.
#if defined(GTA2_PORT_64BIT)
    #define SPRITE_DELTAS_SIZE(count) (offsetof(SpriteDeltas, field_4_deltas) + sizeof(SpriteDelta) * (count))
    #define DELTA_CONTAINER_SIZE(count) (offsetof(DeltaContainer, field_4_entries) + sizeof(SpriteDeltas*) * (count))
#else
    #define SPRITE_DELTAS_SIZE(count) (8 * (count) + 4)
    #define DELTA_CONTAINER_SIZE(count) (4 * (count) + 4)
#endif

namespace tile_spec
{
enum
{
    kNone = 1,
    kGrassDirtFloor = 2,
    kRoadJunctionSpecial = 3,
    kWater = 4,
    kElectrifiedFloor = 5,
    kElectrifiedPlatformFloor = 6,
    kWoodFloor = 7,
    kMetalFloor = 8,
    kMetalWall = 9,
    kGrassDirtWall = 10,
};
//static_assert(sizeof(tile_spec) == 1)
} // namespace tile_spec

class Gtx
{
  public:
    bool IsRemappedWaterTile(u16 spec_idx)
    {
        if (spec_[spec_idx] == 4 && IsTileRemapped(spec_idx))
        {
            return true;
        }
        return false;
    }

    // 9.6f 0x4C03D0 (a ctor of the spec array there)
    void ResetSpecs()
    {
        for (int& i : spec_)
        {
            i = 1;
        }
    }

    bool IsWaterTile(u16 spec_idx)
    {
        return spec_[spec_idx] == 4;
    }

    bool IsRoadTile(u16 spec_idx)
    {
        return spec_[spec_idx] == 3;
    }

    s32 GetSpecWith3AsOne(u16 spec_idx)
    {
        s32 result = spec_[spec_idx];
        if (result == 3)
        {
            result = 1;
        }
        return result;
    }

    bool IsSpec4NotRemapped(u16 spec_idx)
    {
        if (spec_[spec_idx] == 4 && !IsTileRemapped(spec_idx))
        {
            return true;
        }
        return false;
    }

    // inline
    bool HasTiles() const
    {
        return tiles_ != nullptr;
    }

    // inline
    u8* GetTilePixels(u16 tile_num)
    {
        return &tiles_[(((tile_num & ~3) * 64) + (tile_num & 3)) * 64];
    }

    u8 GetSpriteWidth(u16 a2)
    {
        return GetSpriteIndex(a2)->width_;
    }

    u8 GetSpriteHeight(u16 a2)
    {
        return GetSpriteIndex(a2)->height_;
    }

    StyleCarInfo* GetCarInfo(u8 idx);

    BYTE* GetCarRemap(u8 idx);

    SpriteDelta* GetDelta(u16 sprite_idx, u8 delta_idx);

    SpriteIndex* GetSpriteIndex(u16 idx);

    u16 GetSpriteTrueIndex(s32 sprite_type, s16 sprite_pal);

    s16 GetSpriteBaseOfType(s32 sprite_type);

    s16 GetPaletteBaseOfType(s32 palette_type);

    s16 GetTruePalette(s32 palette_type, s16 pal); // maybe "get virtual palette"?

    BYTE* GetPalData(u16 palette_idx);

    u16 GetPhysPal(u16 pal_id);

    u16 GetSpriteIdxFromFont(u16 font_type, s16 offset);

    u16 GetFontWidth(u16* font_type, wchar_t* str);

    u16 GetSpaceCharWidth(u16* font_type);

    s16 GetLineSpacing(u16* font_type);

    bool IsTileRemapped(u16 tile_idx);

    s16 GetTileMapping(u16 tile_idx);

    s16 GetFirstFreeReservedTileIdx();

    void* GetTiles();

    u16 GetPhysicalPalettesLen();

    ObjectInfo* GetMapObjectInfo(u16 idx);

    void SetTileRemap(u16 tile_idx, s16 tile_val);

    void InitTileMapping();

    void BuildCarInfoContainer(u32 chunk_size);

    void LoadDelx(u32 delx_chunk_size);

    void SetDeltaDataPtrs();

    void SetSpriteIndexDataPtrs();

    void BuildDeltaContainer();

    void LoadCarInfo(u32 cari_chunk_size);

    void LoadDeltaIndex(u32 delx_chunk_size);

    void LoadDeltaStore(u32 dels_chunk_size);

    void LoadTiles(u32 tile_chunk_len);

    void SkipOvly(u32 size);

    void SkipPsxt(u32 size);

    void LoadSpriteGraphics(u32 sprg_chunk_len);

    void LoadPhysicalPalettes(u32 ppal_chunk_size);

    void LoadPaletteIndex(u32 palx_chunk_len);

    void LoadMapObjectInfo(u32 obji_chunk_len);

    void LoadSpriteIndex(u32 sprx_chunk_size);

    void InitFontTypes(u16 font_count);

    void LoadFontBase(u32 fonb_chunk_size);

    static u16 SumOffsets(u16* offsets, u32 offsets_count);

    static void ConvertToVirtualOffsets(u16* buffer, u32 len);

    void LoadSpriteBase(u32 sprite_base_chunk_size);

    void LoadPaleteBase(u32 palette_base_chunk_len);

    bool IsCarModelInRecycleList(u8 car_id);

    void LoadCarRecyclingInfo(u32 recy_chunk_size);

    void ReadSpec(u32 read_size2);

    void LoadSpec();

    void LoadChunk(const char_type* str1, u32 chunk_len);

    void SetDataPtrs();

    void LoadSty(const char_type* sty_file_name);

    // 0x5AB820
    Gtx();

    // 0x5AB8A0
    ~Gtx();

    // inlined v9.6f, 0x432850
    u8 GetNumberOfCars() const
    {
        return cari_->count_;
    }

    // inlined v9.6f, 0x420200
    bool DoesCarExist(u8 parm1) const
    {
        return cari_->car_info_[parm1] != nullptr;
    }

    bool IsElectrifiedFloorType(u16 tile_idx)
    {
        return spec_[tile_idx] == SpecSurfaceType::kSpecElectrified;
    }

    s16 total_pal_base_;
    u16 font_base_total_;
    s16 sprite_index_count_;
    u16 map_object_info_len_;
    s16 physical_palettes_len_;
    s16 unused_after_physical_palettes_len_;
    PaletteBase* palette_base2_;
    PaletteBase* palette_base1_;
    SpriteBase* sprite_base2_;
    SpriteBase* sprite_base1_;
    FontBase* font_base_;
    SpriteIndex* sprite_index_;
    ObjectInfo* map_object_info_;
    PaletteIndex* palette_index_;
    void* physical_palettes_;
    void* physical_palettes_size_;
    BYTE* sprite_graphics_;
    void* sprite_graphics_unaligned_;
    u8* tiles_;
    TileArray* tile_;
    void* aligned_tiles_size_;
    DeltaStoreEntry* delta_store_;
    DeltaEntry* delta_index_;
    void* delta_buffer_;
    DeltaContainer* del_;
    StyleCarInfo** car_info_;
    CarInfoContainer* cari_;
    s32 delta_len_;
    u8* car_recycling_info_;
    s16 recy_chunk_size_;
    char_type palettes_converted_;
    char_type unused_after_palettes_converted_;
    s32 spec_[1024];
};

EXTERN_GLOBAL(Gtx*, gGtx);

inline s32 GetSpaceWidth(u16 font_type)
{
    return gGtx->GetSpaceCharWidth(&font_type);
}

EXTERN_GLOBAL(s16, gFontTypeSlot2Of7);
EXTERN_GLOBAL(s16, gFontTypeSlot1Of7);
EXTERN_GLOBAL(s16, gFontTypeSlot5Of7);
EXTERN_GLOBAL(s16, gFontType0);
EXTERN_GLOBAL(s16, gFontType3);
EXTERN_GLOBAL(s16, gFontType1);
EXTERN_GLOBAL(s16, gFontType2);
