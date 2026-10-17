#pragma once

#include "core/ang16.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"
#include "data/gtx.hpp"
#include <windows.h>

class Gang_144;
class Sprite;
class Fix16Rect;

struct gmp_col_info
{
    u8 field_0_height;
    u8 field_1_offset;
    s16 field_2_pad;
    s32 field_4_blockd[2];
};

// gmp_block_info::field_A_arrows bits: the low nibble is the green arrows, the high nibble the red ones
namespace block_arrow
{
enum
{
    green_left_1 = 0x1,
    green_right_2 = 0x2,
    green_up_4 = 0x4,
    green_down_8 = 0x8,
    red_left_10 = 0x10,
    red_right_20 = 0x20,
    red_up_40 = 0x40,
    red_down_80 = 0x80,
    green_mask = 0xF,
    red_mask = 0xF0,
    left_mask = 0x11, // green or red left
    right_mask = 0x22,
    horizontal_mask = 0x33, // left or right
    vertical_mask = 0xCC, // up or down
};
} // namespace block_arrow

struct gmp_block_info
{
    void init_44C840()
    {
        field_0_left = 0;
        field_2_right = 0;
        field_4_top = 0;
        field_6_bottom = 0;
        field_8_lid = 0;
        field_A_arrows = 0;
        field_B_slope_type = 0;
    }

    u16 field_0_left;
    u16 field_2_right;
    u16 field_4_top;
    u16 field_6_bottom;
    u16 field_8_lid;
    u8 field_A_arrows;
    u8 field_B_slope_type;
};

enum gmp_slope_type
{
    AIR = 0,
    ROAD = 1,
    PAVEMENT = 2,
    FIELD = 3,
};

enum MapSlopes
{
    DIAGONAL_WALL_UP_LEFT = 0xB4,
    DIAGONAL_WALL_UP_RIGHT = 0xB8,
    DIAGONAL_WALL_DOWN_LEFT = 0xBC,
    DIAGONAL_WALL_DOWN_RIGHT = 0xC0,
    TRIANGULAR_SIDES_DIAGONAL_UP_LEFT = 0xC4,
    TRIANGULAR_SIDES_DIAGONAL_UP_RIGHT = 0xC8,
    TRIANGULAR_SIDES_DIAGONAL_DOWN_LEFT = 0xCC,
    TRIANGULAR_SIDES_DIAGONAL_DOWN_RIGHT = 0xD0,
    PARTIAL_BLOCK_LEFT = 0xD4,
    PARTIAL_BLOCK_RIGHT = 0xD8,
    PARTIAL_BLOCK_TOP = 0xDC,
    PARTIAL_BLOCK_BOTTOM = 0xE0,
    PARTIAL_TOP_LEFT_CORNER = 0xE4,
    PARTIAL_TOP_RIGHT_CORNER = 0xE8,
    PARTIAL_BOTTOM_RIGHT_CORNER = 0xEC,
    PARTIAL_BOTTOM_LEFT_CORNER = 0xF0,
    PARTIAL_CENTRE_BLOCK = 0xF4,
};

struct gmp_header
{
    char_type field_0_file_type[4];
    s16 field_4_version;
};

struct DmapInfoList;

// One 4-byte word of a .gmp file's column data. The original types the column array as u16**, which
// has the right element size at 32 bits only; at 64 bits the words would be read 8 bytes apart.
#if defined(GTA2_PORT_64BIT)
using gmp_column_word = u32;
#else
using gmp_column_word = u16*;
#endif

struct gmp_compressed_map_32
{
    void ApplyDmapInfos_4E80A0(DmapInfoList* pMapSub);

    gmp_compressed_map_32()
    {
        field_40008_pColumn = nullptr;
        field_4000C_block = nullptr;
        field_40004_num_blocks = 0;
        field_40000_column_words = 0;
    }

    u32 field_0_base[256][256];
    u32 field_40000_column_words;
    u32 field_40004_num_blocks;
    gmp_column_word* field_40008_pColumn;
    gmp_block_info* field_4000C_block;

    gmp_block_info* get_block(u32 block)
    {
        return &field_4000C_block[block];
    }

    u32* get_base_42A830(s32 y, s32 x)
    {
        return &field_0_base[y][x];
    }
};

struct gmp_dmap_info
{
    void set_data_463080(u8 x, u8 y, u32 column_idx)
    {
        field_4_x = x;
        field_5_y = y;
        field_0_column_idx = column_idx;
    }

    u32 field_0_column_idx;
    u8 field_4_x;
    u8 field_5_y;
    u16 field_6_unused;
};

struct DmapInfoList
{
    DmapInfoList()
    {
        field_320_max_idx = 0;
    }

    void AddOrUpdateDmapInfo_4E80E0(u8 x, u8 y, u32 column_idx);

    gmp_dmap_info field_0_dmap_infos[100];
    u32 field_320_max_idx;
};

class gmp_map_zone
{
  public:
    u8 field_0_zone_type;
    u8 field_1_x;
    u8 field_2_y;
    u8 field_3_w;
    u8 field_4_h;
    u8 field_5_name_length;
    char_type field_6_name[]; // field_5_name_length chars, as stored in the map file

    s8 IsZoneVisibleToAnyPlayer_4DEF40();
    wchar_t* get_zone_str_4DEF00();

    bool ContainsPoint_463020(u8 x, u8 y)
    {
        return x >= field_1_x && y >= field_2_y && x < field_1_x + field_3_w && y < field_2_y + field_4_h;
    }
};

struct gmp_zone_info
{
    u16 field_0_car_density;
    s16 field_2_goodcar_ratio;
    s16 field_4_badcar_ratio;
    s16 field_6_policecar_ratio;
    s16 field_8_gangcar_ratio;
    s16 field_A_ped_density;
    s16 field_C_mugger_ratio;
    s16 field_E_carthief_ratio;
    s16 field_10_elvis_ratio;
    s16 field_12_gangchar_ratio;
    s16 field_14_policeped_ratio;
};

struct gmp_map_object
{
    u16 field_0_x;
    u16 field_2_y;
    Ang8 field_4_rotation;
    u8 field_5_object_type;
};

struct gmp_map_light
{
    s32 field_0_argb;
    s16 field_4_x;
    s16 field_6_y;
    s16 field_8_z;
    s16 field_A_radius;
    u8 field_C_intensity;
    u8 field_D_shape;
    u8 field_E_on_time;
    u8 field_F_off_time;
};

struct gmp_tile_animation
{
    u16 field_0_base; // The base tile of the animation. This marks the tile as an animating tile.
    u8 field_2_frame_rate; // The number of game frames that each frame of the animation is displayed for.
    u8 field_3_repeat; // The number of times the animation will be played. 0 means play forever.
    u8 field_4_anim_length; // The number of tiles in the animation
    u8 field_5_unused;
    // NOTE: Size set to 1 to shutup a warning
    u16 field_6_tiles[1]; // Size given by field_4_anim_length
};

struct gmp_map_slope
{
    gmp_map_slope(u8 gradient_direction, u8 gradient_size, u8 gradient_level, Fix16 zpos_lower, Fix16 zpos_higher);

    gmp_map_slope() = default;

    u8 field_0_gradient_direction; // the direction the slope level increases. E.g. "direction north" means it increases the level from south to north
    u8 field_1_gradient_size; // 1 = 45 degree; 2 = 26 degree; 8 = 7 degree slope
    u8 field_2_gradient_level; // 0 = highest slope of the gradient, it increases as going lower
    u8 field_3_padding;
    Fix16 field_4_zpos_lower; // relative z coordinate from the lowest point of the slope
    Fix16 field_8_zpos_higher; // relative z coordinate from the highest point of the slope
};

enum gmp_gradient_slope_direction // direction: low to high
{
    NO_GRADIENT_SLOPE_0 = 0,
    NORTH_1 = 1,
    SOUTH_2 = 2,
    WEST_3 = 3,
    EAST_4 = 4,
};

enum gmp_gradient_slope_size
{
    SIZE_1 = 1,
    SIZE_2 = 2,
    SIZE_8 = 8,
};

enum gmp_arrow_direction
{
    UP_1 = 1,
    DOWN_2 = 2,
    LEFT_3 = 3,
    RIGHT_4 = 4,
};

namespace path_direction
{
enum
{
    up_1 = 1,
    down_2 = 2,
    right_3 = 3, // notice that right and left are swapped in comparison with gmp_arrow_direction
    left_4 = 4, // notice that right and left are swapped in comparison with gmp_arrow_direction
};
} // namespace path_direction

namespace road_direction
{
enum
{
    up_1 = 1,
    down_2 = 2,
    right_3 = 3, // notice that right and left are swapped in comparison with gmp_arrow_direction
    left_4 = 4, // notice that right and left are swapped in comparison with gmp_arrow_direction
};
} // namespace road_direction

enum gmp_arrow_type
{
    green_1 = 1,
    red_2 = 2,
    green_or_red_3 = 3,
};

EXTERN_GLOBAL_ARRAY(gmp_map_slope, gGmpSlopes_6F5BA8, 64);

EXTERN_GLOBAL(Fix16, kGradientScaleSize1_6F5ED8);

EXTERN_GLOBAL(Fix16, kGradientScaleSize2_6F5FA8);

EXTERN_GLOBAL(Fix16, kGradientScaleSize8_6F6214);

extern Fix16 kFpOne_6F6110;
EXTERN_GLOBAL(Ang16, word_6F603E);
Ang16 ReturnAngleFromRoadDirection_4F7940(s32* road_direction);

struct gmp_zone_list
{
    u16 field_0_num_zones;
    u16 field_2_pad;
    gmp_map_zone* field_4_zone_array[2];
};

// Bytes taken by a gmp_zone_list of count zones. The original spells it for 4-byte pointers.
#if defined(GTA2_PORT_64BIT)
    #define GMP_ZONE_LIST_SIZE(count) (offsetof(gmp_zone_list, field_4_zone_array) + sizeof(gmp_map_zone*) * (count))
#else
    #define GMP_ZONE_LIST_SIZE(count) (4 * (count) + 4)
#endif

class Map_0x370
{
  public:
    gmp_block_info* get_block_452980(u8 x_coord, u8 y_coord, u8 z_coord);

    gmp_map_zone* zone_by_name_4DEFD0(const char_type* pZoneName);

    s32 zone_idx_by_name_4DF050(const char_type* pZoneName, BYTE zone_name_len);

    gmp_map_zone* zone_by_type_bounded_4DF0F0(u8 zone_type);

    gmp_map_zone* first_zone_by_type_4DF1D0(u8 zone_type);

    gmp_map_zone* GetNearestZoneOfType_4DF240(u8 xpos, u8 ypos, u8 zone_type);
    gmp_map_zone* sub_4DF3E0(u8 xpos, u8 ypos, u8 zone_type);

    gmp_map_zone* zone_by_pos_and_type_4DF4D0(u8 zone_x, u8 zone_y, u8 zone_type);
    gmp_map_zone* nav_zone_by_pos_4DF5C0(u8 zone_x, u8 zone_y);

    gmp_map_zone* first_zone_by_pos_4DF6A0(u8 zone_x, u8 zone_y);

    gmp_map_zone* next_zone_4DF770();

    gmp_zone_info* cur_zone_4DF840();

    gmp_zone_info* GetZoneInfoAt_4DF890(s32 zone_x, s32 zone_y);

    void SetZoneInfo_4DF8C0(s32 zone_idx,
                            s16 car_density,
                            s16 goodcar_ratio,
                            s16 badcar_ratio,
                            s16 policecar_ratio,
                            s16 gangcar_ratio,
                            s16 ped_density,
                            s16 mugger_ratio,
                            s16 carthief_ratio,
                            s16 elvis_ratio,
                            s16 gangchar_ratio,
                            s16 policeped_ratio);

    gmp_zone_info* GetZoneInfo_4DF9A0(u16 zone_idx);

    void SetZoneCarDensity_4DF9D0(u16 zone_idx, s16 value);

    void SetZoneGoodCarRatio_4DF9F0(u16 zone_idx, s16 value);

    void SetZoneBadCarRatio_4DFA10(u16 zone_idx, s16 value);

    void SetZonePoliceCarRatio_4DFA30(u16 zone_idx, s16 value);

    void SetZoneGangCarRatio_4DFA50(u16 zone_idx, s16 value);

    void SetZonePedDensity_4DFA70(u16 zone_idx, s16 value);

    void SetZoneMuggerRatio_4DFA90(u16 zone_idx, s16 value);

    void SetZoneCarThiefRatio_4DFAB0(u16 zone_idx, s16 value);

    void SetZoneElvisRatio_4DFAD0(u16 zone_idx, s16 value);

    void SetZoneGangCharRatio_4DFAF0(u16 zone_idx, s16 value);

    void SetZonePolicePedRatio_4DFB10(u16 zone_idx, s16 value);

    gmp_map_zone* get_zone_4DFB30(u16 zone_idx);

    Gang_144* GetGangAtCoords_4DFB50(Fix16 x, Fix16 y);

    void SpawnMapObjects_4DFB90();
    void alloc_zones_4DFCA0();
    void update_lights_4DFCD0();

    gmp_block_info* get_block_4DFE10(s32 x_coord, s32 y_coord, s32 z_coord);

    gmp_block_info* GetEffectiveBlock_4DFE60(s32 x, s32 y, s32 z);
    gmp_block_info* GetBlockClamped_4DFEE0(s32 x_coord, s32 y_coord, s32 z_coord);

    DWORD GetLeftEdgeSpec_4DFF60(Fix16 x_coord, Fix16 y_coord, Fix16 z_coord);

    s32 GetTopEdgeSpec_4E0000(Fix16 x_pos, Fix16 y_pos, Fix16 z_pos);
    s32 GetBlockSpec_4E00A0(Fix16 x, Fix16 y, Fix16 z);

    // 9.6f 0x49EBE0, inlined in 10.5; defined in CarPhysics_B0.cpp (ProcessGroundCollisionAndSurfaceType_55B970)
    inline u8 GetBlockSurfaceType_49EBE0(s32 x, s32 y, s32 z, u8* pGradientSize, u8* pGradientLevel);
    char_type sub_4E0110();
    char_type sub_4E0120();
    bool CanMoveOntoSlopeTile_4E0130(s32 x, s32 y, s32 z, s32 path_direction, u8* pSlopeZDelta, char_type bReportStepUp);
    char_type RectHitsDiagonalWall_4E11E0(Fix16Rect* pRect);
    bool SpriteHitsDiagonalWall_4E1520(s32 z_pos);
    bool HasWallInArea_4E18A0(s32 x_min, s32 x_max, s32 y_min, s32 y_max, s32 z);
    bool sub_4E1A30(s32 tileX_min, s32 tileX_max, s32 tileY_min, s32 tileY_max, s32 zLevel);
    char_type CanSpriteEnterTile_4E1E00(s32 regionLeft, s32 regionRight, s32 regionTop, s32 regionBottom, s32 tileX, s32 tileY, s32 zLevel);
    char_type CanSpriteEnterMovementRegion_4E4460(s32 tile_x, s32 tile_y, s32 tile_z, Sprite* pSprite, s16 collision_mask);
    char_type sub_4E4630(Fix16 zpos_f16);
    bool sub_4E4770(Fix16 z_pos);
    char_type sub_4E4820(Fix16Rect* pRect, u8 slope_type);
    void FindNearbyBlockOfType_4E4930(u8* pX, u8* pY, u8* pZ, char_type block_type);
    static bool IsSearchBlockOfType_4E4AC0(char_type block_type);
    bool CheckGreenArrowDirection_4E4B40(s32 direction, gmp_block_info* pBlock);
    gmp_block_info* FindPavementBlockForCoord_4E4BB0(s32 x, s32 y, s32& z);
    gmp_block_info* FindHighestBlockForCoord_4E4C30(s32 x, s32 y, s32* found_z);
    gmp_block_info* FindNonAirBlockAtOrBelowZ_4E4CB0(s32 x, s32 y, s32& z);
    Fix16 FindGroundZBelowCoord_4E4D40(Fix16 x_pos, Fix16 y_pos, Fix16 z_pos);
    Fix16 sub_4E4E50(Fix16 x_pos, Fix16 y_pos, Fix16 z_pos);
    Fix16 GetGroundZBelowCoord_4E4F40(Fix16 x, Fix16 y, Fix16 z);
    Fix16 sub_4E5050(Fix16 x, Fix16 y, Fix16 z, bool& bFound);
    bool IsZOnGround_4E5170(Fix16 x, Fix16 y, Fix16 z);
    char_type sub_4E52A0(Fix16 x, Fix16 y, Fix16 z);
    bool CheckZCollisionAtCoord_4E5300(Fix16 x, Fix16 y, Fix16 z, Fix16 second_z);
    bool CanPlaceOilOrMine_4E5480(Fix16 x, Fix16 y, Fix16 z, Fix16 unk_z_coord, Fix16* found_z);
    char_type sub_4E5640(Fix16 width, Fix16 height, Fix16 depth, Fix16 x_1, Fix16 y_1, Fix16 z_1, Fix16 x_2, Fix16 y_2, Fix16 z_2);
    Fix16 FindGroundZForCoord_4E5B60(Fix16 x_pos, Fix16 y_pos);
    u8 UpdateZFromSlopeAtCoord_4E5BF0(Fix16 x_pos, Fix16 y_pos, Fix16& z_pos);
    bool RoundedZIsAboveSlope(Fix16 x_pos, Fix16 y_pos, Fix16 z_pos, Fix16& new_z);
    char_type HasGreenArrowForPathDirection_4E5E90(gmp_block_info* pBlock, s32 direction, char_type bDefault);
    s32 GetArrowDirectionFromBlock_4E5FC0(gmp_block_info* pBlock, char_type bDontFlip);
    s16 sub_4E6190(Fix16 x, Fix16 y, Fix16 z, s32 arrow_dir, char_type bFlag);
    Fix16 FloorPlusOne_4E62B0(Fix16 value);
    gmp_block_info* FindRailwayAtCoord_4E62D0(s32 x, s32 y, s32& found_z);
    gmp_block_info* FindRailwayBelowZAtCoord_4E6360(s32 x, s32 y, s32& z);
    Fix16 GetRailwayZBelowCoord_4E6400(Fix16 x_pos, Fix16 y_pos, Fix16 z_pos);
    Fix16 GetRailwayZCoordAtXY_4E6510(Fix16 x, Fix16 y);
    void sub_4E65A0(Fix16 x, Fix16 y, Fix16* z_pos, char_type arrow_param, char_type bFlag);
    void sub_4E5D10(Fix16* pX, Fix16* pY, Fix16 dist, s32 direction);
    Fix16 sub_4E5D70(Fix16 x, Fix16 y, Ang16 angle);
    Fix16 sub_4E5E00(Fix16 x, Fix16 y, Ang16 angle);

    // The block under (x, y, z), or the one at z when the one under is a lid-less "air" block
    gmp_block_info* GetRoadBlockAt_4E6660(Fix16& x, Fix16& y, Fix16& z)
    {
        gmp_block_info* pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), (z - kFpOne_6F6110).ToInt());
        if ((pBlock->field_B_slope_type & 0xFC) == 0xFC)
        {
            pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), z.ToInt());
        }
        return pBlock;
    }

    // As GetRoadBlockAt_4E6660, writing straight into pBlock (the original keeps pBlock in a register
    // through both lookups, which a returned temporary doesn't give)
    void SetRoadBlockAt_4E6660(gmp_block_info*& pBlock, Fix16& x, Fix16& y, Fix16& z)
    {
        pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), (z - kFpOne_6F6110).ToInt());
        if ((pBlock->field_B_slope_type & 0xFC) == 0xFC)
        {
            pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), z.ToInt());
        }
    }

    // As SetRoadBlockAt_4E6660, but copes with no block under (x, y, z)
    void SetRoadBlockAtOrNull_4E7190(gmp_block_info*& pBlock, Fix16& x, Fix16& y, Fix16& z)
    {
        pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), (z - kFpOne_6F6110).ToInt());
        if (pBlock && (pBlock->field_B_slope_type & 0xFC) == 0xFC)
        {
            pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), z.ToInt());
        }
    }

    // As GetRoadBlockAt_4E6660, but copes with no block under (x, y, z)
    gmp_block_info* GetRoadBlockAtOrNull_4E7190(Fix16& x, Fix16& y, Fix16& z)
    {
        gmp_block_info* pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), (z - kFpOne_6F6110).ToInt());
        if (pBlock && (pBlock->field_B_slope_type & 0xFC) == 0xFC)
        {
            pBlock = get_block_4DFE10(x.ToInt(), y.ToInt(), z.ToInt());
        }
        return pBlock;
    }

    // Move up to one block along direction, using up dist
    void StepOneBlock_4E6660(Fix16* pX, Fix16* pY, Fix16& dist, s32 direction)
    {
        if (dist >= kFpOne_6F6110)
        {
            sub_4E5D10(pX, pY, kFpOne_6F6110, direction);
            dist -= kFpOne_6F6110;
        }
        else
        {
            sub_4E5D10(pX, pY, dist, direction);
            dist = kFixZero;
        }
    }

    s32 sub_4E6660(Fix16* pX, Fix16* pY, Fix16* pZ, Fix16 dist);
    s32 sub_4E7190(Fix16* pX, Fix16* pY, Fix16* pZ, Fix16 dist);
    char_type FindFirstPavementCoord_4E7E90(u8* pX, char_type* pY);
    char_type CheckColumnHasSolidAbove_4E7FC0(Fix16 x, Fix16 y, Fix16 z);
    s32 AddBlock_4E8140(gmp_block_info* pBlockInfo);
    s32 CloneOriginalBlock_4E8180(u32 read_block_idx);
    s32 CloneOriginalColumn_4E81D0(u32 column_idx);
    s32 CloneColumnExtendedToZ_4E8220(u32 column_idx, s32 z);
    s32 sub_4E8370(u32 column_idx, s32 z, char_type do_drop);
    void ChangeBlock_4E8620(s32 x, s32 y, s32 z, s32 info_type_to_set, u16 info_value);
    void AddNewBlock_4E87C0(s32 x, s32 y, s32 z, gmp_block_info* pBlockData);
    void RemoveBlock_4E8940(s32 x_pos, s32 y_pos, s32 offset, char_type do_drop);
    void LowerColumn_4E8A10(s32 x_pos, s32 y_pos);
    void LowerLevel_4E8B70(s32 x_min, s32 x_max, s32 y_min, s32 y_max);
    void OnModifiedMapDataLoaded_4E8C00(u32 column_bytes, u32 block_bytes, u32 obj_bytes);
    void GetModifiedMapData_4E8CF0(gmp_column_word** outColumnPtr,
                                   u32* outColumnBytes,
                                   gmp_block_info** outBlockPtr,
                                   u32* outBlockBytes,
                                   DmapInfoList** outSubObj,
                                   s32* outSubObjBytes);

    void do_process_loaded_zone_data_4E8E30();

    void build_zone_list_4E90E0(u32 chunk_size);

    void AddTileAnimations_4E9160(s32 size);

    void load_mobj_4E91A0(size_t len);

    void load_lght_4E9200(size_t size);

    void load_zone_4E9250(size_t size);

    void load_anim_4E9280(size_t size);

    void load_dmap_4E92B0(u32 len);

    void load_rgen_4E94A0();

    void load_chunk_4E94B0(const char_type* pChunkType, u32 chunkLen);

    void process_loaded_zone_data_4E95A0();

    void LoadMap_4E95B0(const char_type* pGmpFileName);

    Map_0x370();

    ~Map_0x370();

    bool IsWaterBlockAt_4B9F40(s32 x, s32 y, s32 z)
    {
        gmp_block_info* pBlock = get_block_4DFE10(x, y, z);
        return pBlock && gGtx->IsRemappedWaterTile(pBlock->field_8_lid & 0x3FF);
    }

    // 9.6f inline 0x4634E0
    Fix16 get_grad_scale_from_size_4634E0(Fix16& scale, u8 gradient_size)
    {
        switch (gradient_size)
        {
            case SIZE_1:
                scale = kGradientScaleSize1_6F5ED8; //  16384
                break;
            case SIZE_2:
                scale = kGradientScaleSize2_6F5FA8; //  8168
                break;
            case SIZE_8:
                scale = kGradientScaleSize8_6F6214; //  2084
                break;
            default:
                scale = kFixZero; //  0
                break;
        }
        return scale;
    }

    gmp_block_info* get_block_42A850(u8 x, u8 y, u8 z)
    {
        u32 col_idx = *field_0_pDmap->get_base_42A830(y, x);
        auto* pCol = reinterpret_cast<gmp_col_info*>(&field_0_pDmap->field_40008_pColumn[col_idx]);
        if (z >= pCol->field_0_height || z < pCol->field_1_offset)
        {
            return nullptr;
        }
        return &field_0_pDmap->field_4000C_block[pCol->field_4_blockd[z - pCol->field_1_offset]];
    }

    bool IsBlockPavementType_42A8C0(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x, y, z);
        if (pBlock && (pBlock->field_B_slope_type & 3) == PAVEMENT)
        {
            return true;
        }
        return false;
    }

    bool IsBlockPavementTypeInlined_433530(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_42A850(x, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == PAVEMENT;
        }
        return false;
    }

    // Like IsBlockPavementTypeInlined_433530 but via get_block_452980 (TrafficLight_20::Init_5C1D00)
    bool IsBlockPavementTypeAt_452980(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == PAVEMENT;
        }
        return false;
    }

    bool IsBlockRoadType_42A8C0(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    // 9.6f out of line 0x433470 (North), 0x4334A0 (East), 0x4334D0 (South), 0x433500 (West).
    bool IsBlockRoadTypeInlined_433470(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_42A850(x, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    // 9.6f inline 0x420420
    u8 GetBlockTypeAtCoord_420420(s32 a2, s32 a3, s32 a4)
    {
        gmp_block_info* pBlock = get_block_4DFE10(a2, a3, a4);
        if (pBlock)
        {
            return pBlock->field_B_slope_type & 3;
        }
        return 0;
    }

    bool IsSlopeB4ToD0At_433430(s32 xpos, s32 ypos, s32 zpos)
    {
        gmp_block_info* pBlock = get_block_4DFE10(xpos, ypos, zpos);
        if (pBlock)
        {
            u32 slope_type = pBlock->field_B_slope_type & 0xFC;
            if (slope_type >= 0xB4 && slope_type <= 0xD0)
            {
                return true;
            }
        }
        return false;
    }

    bool IsGradientSlopeAt_466CF0(s32 xpos, s32 ypos, s32 zpos)
    {
        gmp_block_info* block_4DFE10 = get_block_4DFE10(xpos, ypos, zpos);
        if (block_4DFE10 && (block_4DFE10->field_B_slope_type & 0xFC) > 0 &&
            (block_4DFE10->field_B_slope_type & 0xFC) < 0xB4 //is_gradient_slope(block_4DFE10->field_B_slope_type)
            && (block_4DFE10->field_B_slope_type & 3) != 0) // !is_air_type(block_4DFE10->field_B_slope_type)
        {
            return true;
        }
        return false;
    }

    bool HasBlockAnyArrows_492140(s32 xpos, s32 ypos, s32 zpos)
    {
        gmp_block_info* pBlock = get_block_4DFE10(xpos, ypos, zpos);
        return pBlock && pBlock->field_A_arrows;
    }

    void Clear_F36E_492130()
    {
        field_36E_bBlockedByTerrain = 0;
    }

    bool IsNorthOrSouthGradSlope_4634B0(gmp_block_info* pBlock)
    {
        gmp_map_slope* gradient_slope = &gGmpSlopes_6F5BA8[pBlock->field_B_slope_type >> 2];
        if (gradient_slope->field_0_gradient_direction == NORTH_1 || gradient_slope->field_0_gradient_direction == SOUTH_2)
        {
            return true;
        }
        return false;
    }

    // 9.6f 0x463480 (the name's 0x4634B0 is the north/south one)
    bool IsWestOrEastGradSlope_4634B0(gmp_block_info* pBlock)
    {
        gmp_map_slope* gradient_slope = &gGmpSlopes_6F5BA8[pBlock->field_B_slope_type >> 2];
        if (gradient_slope->field_0_gradient_direction == WEST_3 || gradient_slope->field_0_gradient_direction == EAST_4)
        {
            return true;
        }
        return false;
    }

    bool IsNorthBlockRoadType_433470(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x, y - 1, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    bool IsEastBlockRoadType_4334A0(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x + 1, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    bool IsSouthBlockRoadType_4334D0(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_42A850(x, y + 1, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    bool IsWestBlockRoadType_433500(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_42A850(x - 1, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) == ROAD;
        }
        return false;
    }

    bool IsBlockNonAirType_48A350(u8 x, u8 y, u8 z)
    {
        gmp_block_info* pBlock = get_block_452980(x, y, z);
        if (pBlock)
        {
            return (pBlock->field_B_slope_type & 3) != AIR;
        }
        return false;
    }

    gmp_compressed_map_32* field_0_pDmap;
    DmapInfoList field_4_obj;
    gmp_map_zone* field_328_pZoneData;
    gmp_zone_list* field_32C_pZones;
    BYTE* field_330_pZoneArray;
    gmp_zone_info* field_334_pUnknownZoneData;
    gmp_map_object* field_338_pMapObjects;
    gmp_map_light* field_33C_pLightData;
    gmp_tile_animation* field_340_pTileAnimData;
    u32 field_344_map_object_count;
    u32 field_348_num_lights;
    u32 field_34C_num_blocks;
    s32 field_350_num_blocks_extra;
    u32 field_354_num_blocks; // u32: OnModifiedMapDataLoaded_4E8C00 matches only with these two unsigned
    u32 field_358_column_words;
    s32 field_35C_column_word_extra;
    u32 field_360_column_words;
    u16 field_364_cur_zone_idx;
    s16 field_366_unused;
    u8 field_368_zone_type;
    u8 field_369_unused;
    u8 field_36A_zone_x;
    u8 field_36B_zone_y;
    char_type field_36C_bMatchPos;
    char_type field_36D_unused;
    char_type field_36E_bBlockedByTerrain;
    char_type field_36F_bLowerBlockHasArrows;
};

EXTERN_GLOBAL(Map_0x370*, gMap_0x370_6F6268);

EXTERN_GLOBAL(s32, gSpriteGrid_left_6F5FD4);

EXTERN_GLOBAL(s32, gSpriteGrid_right_6F5B80);

void Init_gmp_slopes_array();

static inline u8 get_slope_bits(u8& slope_byte) // slope_byte but with its first 2 bits cleared
{
    return slope_byte & 0xFCU;
}

static inline u8 get_slope_idx(u8& slope_byte) // 0 to 63
{
    return slope_byte >> 2;
}

static inline bool is_air_type(u8& slope_byte)
{
    return (slope_byte & 3) == 0;
}

static inline bool is_gradient_slope(u8& slope_byte)
{
    u8 slope = get_slope_bits(slope_byte);
    return slope > 0 && slope < 0xB4U; // slope idx in range 1 to 45
}

static inline u8 get_block_type(u8& slope_byte)
{
    return slope_byte & 3;
}

static inline bool is_partial_block(s32& slope)
{
    return slope >= 0xD4 && slope <= 0xF4;
}

static inline bool is_tridiagonal_block(s32& slope)
{
    return slope >= 0xC4 && slope <= 0xD0;
}

static inline gmp_map_slope* get_slope_struct(u8& slope_byte)
{
    return &gGmpSlopes_6F5BA8[get_slope_idx(slope_byte)];
}
