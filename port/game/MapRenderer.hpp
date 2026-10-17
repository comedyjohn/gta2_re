#pragma once

#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp" // Vert
#include "Function.hpp"
#include "Game_0x40.hpp"
#include "fix16.hpp"
#include "map_0x370.hpp"

class Fix16Point;

class Nanobotz_8 // Maybe Fix16_Point
{
  public:
    s32 IsEqual_46BB60(Nanobotz_8* other)
    {
        return field_0_x == other->field_0_x && field_4_y == other->field_4_y;
    }
    s32 field_0_x; // x?
    s32 field_4_y; // y?
};

EXTERN_GLOBAL(Fix16, gXCoord_6F63AC);

EXTERN_GLOBAL(Fix16, gYCoord_6F63B8);

EXTERN_GLOBAL(u32, gZCoord_6F63E0);

EXTERN_GLOBAL(gmp_map_slope, gCurrentSlope_6F646C);

EXTERN_GLOBAL(u32, gGradientSize_6F6480);

EXTERN_GLOBAL(u32, gGradientLevel_6F647C);

class MapRenderer
{
  public:
    MapRenderer()
    {
        field_0_ambient = kInitialAmbient_67DCCC;
        field_4_target_ambient = kInitialAmbient_67DCCC;
        field_8_ambient_step.mValue = 0;
        field_2F00_drawn_tile_count = 0;
        field_2EFC_curr_draw_layer_size = 0;
        set_shading_lev_4E9DB0(15U);
    }

    void SetAmbientLevel_4E9D50(s32& target_level, u16& cycles);
    void set_shading_lev_4E9DB0(u8 shading_lev);
    void draw_4E9EE0(u16& word_side, const bool& bUnk, u8 colour);
    void ambient_light_tick_4E9EA0();
    void draw_4EA190(u16& rotation_and_flip);
    void DrawLeftSide_4EA390(u16& left_word);
    void ProjectVertTop_4EAE00(const Fix16& xpos, const Fix16& ypos, Vert* pVert);
    void ProjectVertBottom_4EAEA0(const Fix16& xCoord, const Fix16& yCoord, Vert* pVert);
    void DrawRightSide_4EAF40(u16& right_word);
    void ProjectVert_4EB940(const Fix16& xpos, const Fix16& ypos, const Fix16& zpos, Vert* pVert);
    void DrawTopSide_4EBA60(u16& top_word);
    void DrawDiagonalUpLeftFace_4EC450(u16& left_word);
    void DrawDiagonalUpRightFace_4EC7A0(u16& right_word);
    void DrawDiagonalDownLeftFace_4ECAF0(u16& left_word);
    void DrawDiagonalDownRightFace_4ECE40(u16& right_word);
    void draw_bottom_4ED290(u16& bottom_word);
    void draw_lid_4EE130();

    // Note: These are func chunks
    void DrawDiagonalWallUpLeft_4EE7D0();
    void DrawDiagonalWallUpRight_4EE8A0();
    void DrawDiagonalWallDownLeft_4EE970();
    void DrawDiagonalWallDownRight_4EEA40();
    void DrawPartialBlockLeft_4F5160();
    void DrawPartialBlockRight_4F5360();
    void DrawPartialBlockTop_4F5560();
    void DrawPartialBlockBottom_4F5760();
    void DrawPartialBlockTopLeftCorner_4F5960();
    void DrawPartialBlockTopRightCorner_4F5B70();
    void DrawPartialBlockBottomRightCorner_4F5D80();
    void DrawPartialBlockBottomLeftCorner_4F5FD0();
    void DrawPartialCentreBlock_4F6350();

    void Draw3SidedDiagonalUpLeft_4EEAF0();
    void Draw3SidedDiagonalUpRight_4EEE60();
    void Draw3SidedDiagonalDownLeft_4EF1C0();
    void Draw3SidedDiagonalDownRight_4EF520();
    void Draw4SidedDiagonalUpLeft_4EF880();
    void Draw4SidedDiagonalUpRight_4EFB20();
    void Draw4SidedDiagonalDownLeft_4EFDB0();
    void Draw4SidedDiagonalDownRight_4F0030();
    void DrawDiagonalWall_4F02D0();
    void DrawTriangularDiagonal_4F0340();
    void DrawGradientSlopeNorthwards_4F0420();
    char_type GetColour_4F0BD0(s32 lid_type);
    void DrawGradientSlopeSouthwards_4F1660();
    void DrawGradientSlopeWestwards_4F22F0();
    void DrawGradientSlopeEastwards_4F33B0();
    void draw_left_4F3C00(u16& side_word, Fix16& x_off, Fix16& y_off_a, Fix16& y_off_b);
    void draw_right_4F4250(u16& side_word, Fix16& x_off, Fix16& y_off_a, Fix16& y_off_b);
    void draw_top_4F4600(u16& side_word, Fix16& x_off_a, Fix16& x_off_b, Fix16& y_off);
    void draw_bottom_4F49B0(u16& side_word, Fix16& x_off_a, Fix16& x_off_b, Fix16& y_off);
    void draw_lid_4F4D60(Fix16& unk1, Fix16& unk2, Fix16& unk3, Fix16& unk4);
    void DrawPartialBlocks_4F6580();
    void DrawGradientSlope_4F6630();
    void RenderFlatBlock_4F66C0();
    void RenderBlockAt_4F6880(s32& pXCoord, s32& pYCoord);
    void ClearDrawnTileCount_4F6A10();
    void Draw_4F6A20();

    u32 update_and_get_gradient_direction(u32 idx)
    {
        gGradientSize_6F6480 = gGmpSlopes_6F5BA8[idx].field_1_gradient_size;
        gGradientLevel_6F647C = gGmpSlopes_6F5BA8[idx].field_2_gradient_level;
        gCurrentSlope_6F646C.field_0_gradient_direction = gGmpSlopes_6F5BA8[idx].field_0_gradient_direction;
        return gCurrentSlope_6F646C.field_0_gradient_direction;
    }

    u8 GetColour_46B5E0(s32 a1)
    {
        //u8 diffuseColour;
        switch (a1)
        {
            case 0U:
                return -1;
                break;
            case 1U:
                return field_C_colour_t1;
                break;
            case 2U:
                return field_E_colour_t2;
                break;
            case 3U:
                return field_F_colour_t3;
                break;
            default:
                return 0;
                break;
        }
        //return diffuseColour;
    }

    void AddToDrawList_46BB90(s32& maybe_x, s32& maybe_y)
    {
        Nanobotz_8* pPos = &field_1C_draw_list[field_2EFC_curr_draw_layer_size];
        pPos->field_0_x = maybe_x;
        pPos->field_4_y = maybe_y;

        Nanobotz_8* pIter = &field_1C_draw_list[field_2EFC_curr_draw_layer_size - 1];
        for (s32 i = field_2EFC_curr_draw_layer_size - 1; i >= 0; i--, pIter--)
        {
            if (pIter->IsEqual_46BB60(pPos))
            {
                return;
            }
        }
        ++field_2EFC_curr_draw_layer_size;
    }

    void ResetCount_45B040()
    {
        field_2EFC_curr_draw_layer_size = 0;
    }

    Fix16 field_0_ambient;
    Fix16 field_4_target_ambient;
    Fix16 field_8_ambient_step;
    u8 field_C_colour_t1;
    u8 field_D_right_colour;
    u8 field_E_colour_t2;
    u8 field_F_colour_t3;
    u8 field_10_diag_up_left_colour;
    u8 field_11_diag_up_right_colour;
    u8 field_12_diag_down_left_colour;
    u8 field_13_diag_down_right_colour;
    u8 field_14_dcolour;
    u8 field_15_slope_south_colour;
    u8 field_16_slope_west_colour;
    u8 field_17_slope_east_colour;
    u8 field_18_color;
    u8 field_19_tri_diag_up_right_colour;
    u8 field_1A_tri_diag_down_left_colour;
    u8 field_1B_tri_diag_down_right_colour;
    Nanobotz_8 field_1C_draw_list[1500];
    s32 field_2EFC_curr_draw_layer_size;
    s32 field_2F00_drawn_tile_count;
};

void set_vert_xyz_relative_to_cam_4EAD90(Fix16 xCoord, Fix16 yCoord, Fix16 z_val, Vert* pVerts);

void draw_4F3FB0(s32 arg);

void Set_UV_4F4190(Fix16& a1, Fix16& a2, const u32& pVertIdx);

EXTERN_GLOBAL(MapRenderer*, gpMapRenderer_6F66E4);

struct BlockSideWord
{
    u32 tile_idx : 10;
    u32 wall : 1;
    u32 bullet_wall : 1;
    u32 flat : 1;
    u32 flip : 1;
    u32 rotation_code : 2;
};
