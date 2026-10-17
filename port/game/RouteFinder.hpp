#pragma once

#include "Function.hpp"

struct gmp_block_info;

class Link_2
{
  public:
    // 9.6f 0x40CE90
    u16 GetIndex_0040CE90()
    {
        return field_0 & 0x1ff;
    }

    // 9.6f 0x40CED0
    u8 IsEnabled()
    {
        return field_0 >> 15;
    }

    // 9.6f 0x40CEA0
    u16 GetLength()
    {
        return (field_0 >> 9) & 0x3F;
    }

    void Disable_40CEC0()
    {
        field_0 &= ~0x8000u;
    }

    void Enable_40CEB0()
    {
        field_0 |= 0x8000u;
    }

    u16 field_0;
};

class Junction_10
{
  public:
    char_type sub_588580(s32 arrow_type);
    u16 GetDirectionToJunction_5885C0(u16 junction_idx);

    // 9.6f 0x40CEE0
    bool ContainsPoint(s16 x, s16 y)
    {
        return x >= field_C_min_x && x <= field_E_max_x && y >= field_D_min_y && y <= field_F_max_y;
    }

    Link_2 field_0_n;
    Link_2 field_2_s;
    Link_2 field_4_e;
    Link_2 field_6_w;
    s32 field_8_type;
    u8 field_C_min_x;
    u8 field_D_min_y;
    u8 field_E_max_x;
    u8 field_F_max_y;
};

struct JunctionSegment_0x8
{
    bool ContainsPoint_40CF20(s16 x, s16 y)
    {
        return x >= field_4_min_x && x <= field_6_max_x && y >= field_5_min_y && y <= field_7_min_y;
    }
    u16 field_0_junction_num1;
    u16 field_2_junction_num2;
    u8 field_4_min_x;
    u8 field_5_min_y;
    u8 field_6_max_x;
    u8 field_7_min_y;
};

class RouteFinder_10
{
  public:
    RouteFinder_10();
    u16 field_0_idx;
    u16 field_2_cost;
    s16 field_4_expanded;
    s16 field_6_unused;
    RouteFinder_10* field_8_pParent;
    RouteFinder_10* field_C_pNext;
};

class RouteFinder_200
{
  public:
    u16 field_0_junctions[256];
};

class RouteFinder
{
  public:
    void ShowJunctionIds_588620();
    void RoadOff_588810(u8 x, u8 y, u8 z);
    void RoadOn_588950(u8 x, u8 y, u8 z);
    u16 IsPointInJunctionBounds_588AA0(u8 x, u8 y, u16 junc_idx1, u16 junc_idx2); // ret _BOOL2
    void Load_RGEN_588B30();
    void Reset_588C60();
    bool HasBlockDesiredArrow_588CA0(gmp_block_info* block, s32 arrow_type, u8 direction);
    char_type sub_588DE0(gmp_block_info* pBlock, s32 arrow_type, s32 road_direction);
    u16 FindHorzSegmentJunction_588E60(u8 x, u8 y, u8 z, char_type bIsDest, s32 arrow_type);
    u16 FindVertSegmentJunction_588F30(u8 x, u8 y, u8 z, char_type bIsDest, s32 arrow_type);
    u16 FindSegmentJunction_589000(u8 x_coord, u8 y_coord, u8 z_coord, char_type bIsDest, s32 arrow_type);
    void FindArrowBlockInJunction_5890D0(u16 junction_idx, s32 direction, u8* xpos, u8* ypos);
    s32 NoRefs_589210(u8 x, u8 y, s32 a4, u8 direction, s32 a6, u16 junction_idx);
    RouteFinder_10* NewChildNode_5892F0(RouteFinder_10* pRoute, u16 idx, s16 step_cost);
    RouteFinder_10* NewStartNode_589390(u16 junction_idx);
    void InsertIntoOpenList_589420(RouteFinder_10* pRoute);
    char_type InitSearch_589480(u8 start_x, u8 start_y, u8 start_z, u8 dest_x, u8 dest_y, u8 dest_z, s32 arrow_type);
    char_type sub_5895C0(u8 x, u8 y, u8 z, s32 arrow_type, s32 direction);
    void CancelRoute_589930(s16 idx);
    s16 GetFreeRouteIdx_589960();
    u16 AddChildNode_589990(RouteFinder_10* pRoute, u16 junction_idx, s16 step_cost);
    bool sub_5899C0(RouteFinder_10* pNode, s32 arrow_type);
    char_type sub_589BB0(RouteFinder_10* pRoute, s32 arrow_type);
    RouteFinder_10* GetFirstUnexpandedNode_589E00();
    char_type sub_589E20(s32 arrow_type);
    char_type sub_589E70(s32 arrow_type);
    s16 sub_589EB0();
    s16 sub_589F70();
    void DebugPrintRoute_58A020(char_type junc_idx);
    Junction_10* GetJunction_58A0B0(u16 jIdx);
    s16 DoStartRoute_58A0D0(u8 x1, u8 y1, u8 z1, u8 x2, u8 y2, u8 z2, s32 arrow_type);
    s16 sub_58A130(u8 a1, s16 a2, u8 a3, u8* pRouteLength, s32 a5, s32 a6);
    u16 StartRoute_58A190(u8 x1, u8 y1, u8 z1, u8 x2, u8 y2, u8 z2, s32 arrow_type);
    RouteFinder();

    u16 field_0_route_count;
    u8 field_2;
    char_type field_3_unused;
    s16 field_4;
    s16 field_6_unused;
    Junction_10 field_8_junctions[545];
    RouteFinder_200 field_2218_routes[50];
    u16 field_8618_idx;
    u16 field_861A_dest_idx;
    RouteFinder_10 field_861C_nodes[545];
    RouteFinder_10* field_A82C_open_list;
    JunctionSegment_0x8 field_A830_horz_segments[545];
    JunctionSegment_0x8 field_B938_vert_segments[545];
    char_type field_CA40_visited[545];
    u16 field_CC62_horz_segment_count;
    u16 field_CC64_vert_segment_count;
    u16 field_CC66_545_count;
};

EXTERN_GLOBAL(RouteFinder*, gRouteFinder_6FFDC8);
