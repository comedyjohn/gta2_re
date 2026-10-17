#include "TextureCache_15D8.hpp"
#include "core/function.hpp"
#include "core/memory.hpp"
#include "gbh_graphics.hpp"
#include "gtx_0x106C.hpp"
#include "sprite.hpp"
#include <stdio.h>
#include <stdlib.h>

DEFINE_GLOBAL(TextureCache_15D8*, gTextureCache_15D8_705064, 0x705064);
DEFINE_GLOBAL(u32, gRemappedSpriteCounter_704ED0, 0x704ED0);
DEFINE_GLOBAL(u32, gRemappedTextureCounter_704F28, 0x704F28);

// 0x5B8E90
void RemapTextureSet_14::Alloc_5B8E90(s16 sprite_count, s16 remaps_per_sprite, s32 sprite_type, s32 pal_type)
{
    if (!field_0_pTextures)
    {
        if (sprite_count)
        {
            field_4_texture_count = remaps_per_sprite * sprite_count;
            field_6_remaps_per_sprite = remaps_per_sprite;
            field_10_bDoFree = 1;
            field_8_sprite_type = sprite_type;
            field_C_pal_type = pal_type;
            field_0_pTextures = static_cast<STexture**>(Memory::Malloc(sizeof(STexture*) * field_4_texture_count));

            for (u32 i = 0; i < field_4_texture_count; i++)
            {
                field_0_pTextures[i] = nullptr;
            }
        }
    }
}

// 0x5B8F00
void RemapTextureSet_14::LoadTextures_5B8F00()
{
    if (field_10_bDoFree)
    {
        u32 i = 0;
        SpriteIndex* pSpriteIndex;
        u16 tmp;
        u16 t2;
        while (i < field_4_texture_count)
        {
            pSpriteIndex = gGtx_0x106C_703DD4->GetSpriteIndex(i);
            tmp = gGtx_0x106C_703DD4->GetTruePalette(PaletteTypes::kSprites, i);
            t2 = gGtx_0x106C_703DD4->GetPhysPal(tmp);

            field_0_pTextures[i++] = pgbh_RegisterTexture(
                pSpriteIndex
                    ->width_, // note: missing xor of register due to passing BYTE -> BYTE param instead of BYTE -> s32 param, xor clears up 24 bits
                pSpriteIndex->height_,
                pSpriteIndex->data_,
                t2, // pal idx
                0);
        }
    }
}

// 0x5B8F70
void RemapTextureSet_14::LoadRemappedTextures_5B8F70()
{
    if (field_10_bDoFree)
    {
        const u32 palTotal = field_4_texture_count / field_6_remaps_per_sprite;
        for (u32 pal_idx = 0; pal_idx < palTotal; pal_idx++)
        {
            const u16 sprite_idx = gGtx_0x106C_703DD4->GetSpriteTrueIndex(field_8_sprite_type, pal_idx);
            SpriteIndex* pSpriteIndex = gGtx_0x106C_703DD4->GetSpriteIndex(sprite_idx);

            gRemappedSpriteCounter_704ED0++;

            for (u32 texture_idx = 0; texture_idx < field_6_remaps_per_sprite; texture_idx++)
            {
                const s16 converted_pal_idx = gGtx_0x106C_703DD4->GetTruePalette(field_C_pal_type, texture_idx);
                const u16 physPal = gGtx_0x106C_703DD4->GetPhysPal(converted_pal_idx);
                field_0_pTextures[texture_idx + (pal_idx * field_6_remaps_per_sprite)] =
                    pgbh_RegisterTexture(pSpriteIndex->width_, pSpriteIndex->height_, pSpriteIndex->data_, physPal, 1);

                gRemappedTextureCounter_704F28++;
            }
        }
    }
}

// 0x5B9050
RemapTextureSet_14::~RemapTextureSet_14()
{
    if (field_10_bDoFree && field_0_pTextures)
    {
        for (u16 i = 0; i < field_4_texture_count; ++i)
        {
            pgbh_FreeTexture(field_0_pTextures[i]);
        }

        ::free(reinterpret_cast<void*>(field_0_pTextures));
        field_0_pTextures = nullptr;
    }
}

// 0x5B90A0
STexture* RemapTextureSet_14::get_texture_5B90A0(s32 sprite_type, s16 sprite_idx)
{
    return field_0_pTextures[gGtx_0x106C_703DD4->GetSpriteTrueIndex(sprite_type, sprite_idx)];
}

// 0x5B90D0
STexture* RemapTextureSet_14::GetRemappedTexture_5B90D0(s16 sprite_idx, s16 remap)
{
    return field_0_pTextures[static_cast<u16>(remap + (sprite_idx * field_6_remaps_per_sprite))];
}

// 0x5B90F0
void TextureCache_15D8::LoadPals_5B90F0()
{
    field_15D6_pal_count = gGtx_0x106C_703DD4->GetPhysicalPalettesLen();

    for (u16 palId = 0; palId < field_15D6_pal_count; palId++)
    {
        pgbh_RegisterPalette(palId, reinterpret_cast<DWORD*>(gGtx_0x106C_703DD4->GetPalData(palId)));
    }
}

// 0x5B9140
void TextureCache_15D8::FreePals_5B9140()
{
    for (u16 i = 0; i < field_15D6_pal_count; ++i)
    {
        pgbh_FreePalette(i);
    }
    field_15D6_pal_count = 0;
}

// 0x5B9180
void TextureCache_15D8::LoadCarDamageTextures_5B9180()
{
    u16 width_height = 64;
    if (gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kCar))
    {
        field_1000_bFreeCarDamageTextures = 1;
        u16 sprite_idx = gGtx_0x106C_703DD4->GetSpriteTrueIndex(SpriteTypes::kCar, 0);
        u16 true_pal_idx = gGtx_0x106C_703DD4->GetTruePalette(PaletteTypes::kSprites, sprite_idx);
        u16 pal_idx = gGtx_0x106C_703DD4->GetPhysPal(true_pal_idx);

        for (u32 idx = 0; idx < 48; idx++)
        {
            if (idx == 32)
            {
                width_height = 128;
            }

            field_1004_car_damage_textures[idx] =
                pgbh_RegisterTexture(width_height, width_height, gSpriteMaskCache_67AF1C->GetMaskPixels(idx), pal_idx, 0);
        }
    }
}

// 0x5B9220
s16 TextureCache_15D8::RegisterDigits_5B9220(u16 num_of_digits, u16 palette)
{
    const u16 og_idx = field_15D4_next_digit_idx;
    const s16 sprite_idx = gGtx_0x106C_703DD4->GetSpriteTrueIndex(SpriteTypes::kUser, palette);
    SpriteIndex* sprite_index_5AA440 = gGtx_0x106C_703DD4->GetSpriteIndex(sprite_idx);
    u8* field_0_pData = sprite_index_5AA440->data_;
    const u16 phys_pal_5AA6F0 = gGtx_0x106C_703DD4->GetPhysPal(gGtx_0x106C_703DD4->GetTruePalette(PaletteTypes::kSprites, sprite_idx));

    field_15D4_next_digit_idx += num_of_digits;

    for (s32 i = 0; i < num_of_digits; i++)
    {
        field_10C4_digit_textures[og_idx + i].field_4_pTexture =
            pgbh_RegisterTexture(sprite_index_5AA440->width_, sprite_index_5AA440->height_, field_0_pData, phys_pal_5AA6F0, 0);
        field_10C4_digit_textures[og_idx + i].field_0_pPixelData = field_0_pData;
    }
    return og_idx;
}

// 0x5B92E0
void TextureCache_15D8::ReadTextures_5B92E0()
{
    if (gGtx_0x106C_703DD4->HasTiles())
    {
        field_1001_bFreeTileTextures = 1;
        STexture** p = field_0_tile_textures;
        for (u16 i = 0; i < GTA2_COUNTOF(field_0_tile_textures); i++, p++)
        {
            // Not in the original: guards its out-of-range tile read, which crashes standalone. This guard is
            // the only difference from 10.5; without it the function matches (see docs/match_attempts.md).
            if (i > 992) // avoid original bug crashing standalone
            {
                return;
            }
            // 64 256x256 pages of 64x64 8 bit tiles
            *p = pgbh_RegisterTexture(64, 64, gGtx_0x106C_703DD4->GetTilePixels(i), gGtx_0x106C_703DD4->GetPhysPal(i), 0);
        }
    }
}

// 0x5B9350
void TextureCache_15D8::LoadStyleTextures_5B9350()
{
    LoadPals_5B90F0();
    ReadTextures_5B92E0();

    field_1548_sprite_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kSprites),
                                            1,
                                            SpriteTypes::kUnknown,
                                            PaletteTypes::kSprites);
    field_1548_sprite_textures.LoadTextures_5B8F00();

    field_155C_car_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kCar),
                                               gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kCarRemaps),
                                               SpriteTypes::kCar,
                                               PaletteTypes::kCarRemaps);
    field_155C_car_remap_textures.LoadRemappedTextures_5B8F70();

    field_1570_ped_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kPed),
                                               gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kPedRemaps),
                                               SpriteTypes::kPed,
                                               PaletteTypes::kPedRemaps);
    field_1570_ped_remap_textures.LoadRemappedTextures_5B8F70();

    field_1584_code_obj_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kCodeObj1),
                                                    gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kCodeObjRemaps),
                                                    SpriteTypes::kCodeObj1,
                                                    PaletteTypes::kCodeObjRemaps);
    field_1584_code_obj_remap_textures.LoadRemappedTextures_5B8F70();

    field_1598_map_obj_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kMapObj),
                                                   gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kMapObjRemaps),
                                                   SpriteTypes::kMapObj,
                                                   PaletteTypes::kMapObjRemaps);
    field_1598_map_obj_remap_textures.LoadRemappedTextures_5B8F70();

    field_15AC_font_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kFont),
                                                gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kFontRemaps),
                                                SpriteTypes::kFont,
                                                PaletteTypes::kFontRemaps);
    field_15AC_font_remap_textures.LoadRemappedTextures_5B8F70();

    field_15C0_user_remap_textures.Alloc_5B8E90(gGtx_0x106C_703DD4->GetSpriteBaseOfType(SpriteTypes::kUser),
                                                gGtx_0x106C_703DD4->GetPaletteBaseOfType(PaletteTypes::kUserRemaps),
                                                SpriteTypes::kUser,
                                                PaletteTypes::kUserRemaps);
    field_15C0_user_remap_textures.LoadRemappedTextures_5B8F70();

    LoadCarDamageTextures_5B9180();

    field_1544_pShared_texture = pgbh_RegisterTexture(128, 128, nullptr, 0, 0);
}

// 0x5B94F0
STexture* TextureCache_15D8::GetSpriteTexture_5B94F0(s32 sprite_type, u16 sprite_id, s32 palette_type, s16 remap)
{
    STexture* result;

    switch (palette_type)
    {
        case PaletteTypes::kSprites:
            result = field_1548_sprite_textures.get_texture_5B90A0(sprite_type, sprite_id);
            break;
        case PaletteTypes::kCarRemaps:
            result = field_155C_car_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        case PaletteTypes::kPedRemaps:
            result = field_1570_ped_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        case PaletteTypes::kCodeObjRemaps:
            result = field_1584_code_obj_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        case PaletteTypes::kMapObjRemaps:
            result = field_1598_map_obj_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        case PaletteTypes::kFontRemaps:
            result = field_15AC_font_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        case PaletteTypes::kUserRemaps:
            result = field_15C0_user_remap_textures.GetRemappedTexture_5B90D0(sprite_id, remap);
            break;
        default:
            result = nullptr;
            break;
    }
    return result;
}

// 0x5B95D0
STexture* TextureCache_15D8::GetCarDamageTexture_5B95D0(u16 texture_idx)
{
    return field_1004_car_damage_textures[texture_idx];
}

// 0x5B95F0
STexture* TextureCache_15D8::GetDigitTexture_5B95F0(u16 idx, u16 row_offset, u16 height)
{
    DigitTexture_C* pDigit = &field_10C4_digit_textures[idx];
    STexture* pTexture = pDigit->field_4_pTexture;

    if (height != pDigit->field_8_height || row_offset != pDigit->field_A_row_offset)
    {
        STexture* pTextureInternal = pDigit->field_4_pTexture;
        pDigit->field_8_height = height;
        pDigit->field_A_row_offset = row_offset;
        pgbh_LockTexture(pTextureInternal);
        pTexture->field_14_original_pixel_data_ptr = &pDigit->field_0_pPixelData[256 * row_offset];
        pTexture->field_10_height = height;
        pgbh_UnlockTexture(pTexture);
    }
    return pTexture;
}

// 0x5B9660
void TextureCache_15D8::SetPal_5B9660(u16 digit_idx, u16 pal_idx)
{
    STexture* pTexture = field_10C4_digit_textures[digit_idx].field_4_pTexture;
    u16 pal = gGtx_0x106C_703DD4->GetPhysPal(pal_idx);
    pgbh_LockTexture(pTexture);
    pgbh_AssignPalette(pTexture, pal);
    pgbh_UnlockTexture(pTexture);
}

// 0x5B96B0
void TextureCache_15D8::SetCarDamageTextureSizeAndPalette_5B96B0(u16 texture_idx, u16 new_width, u16 new_height, u16 pal)
{
    u16 pal_idx = gGtx_0x106C_703DD4->GetPhysPal(pal);
    STexture* pTexture = field_1004_car_damage_textures[texture_idx];
    pgbh_LockTexture(pTexture);
    pTexture->field_E_width = new_width;
    pTexture->field_10_height = new_height;
    pgbh_AssignPalette(pTexture, pal_idx);
    pgbh_UnlockTexture(pTexture);
}

// 0x5B9710
STexture* TextureCache_15D8::SetSharedTextureData_5B9710(s16 width, s16 height, u8* pPixelData, u16 pal)
{
    u16 phys_pal_5AA6F0 = gGtx_0x106C_703DD4->GetPhysPal(pal);
    pgbh_LockTexture(field_1544_pShared_texture);
    field_1544_pShared_texture->field_14_original_pixel_data_ptr = pPixelData;
    field_1544_pShared_texture->field_E_width = width;
    field_1544_pShared_texture->field_10_height = height;
    pgbh_AssignPalette(field_1544_pShared_texture, phys_pal_5AA6F0);
    pgbh_UnlockTexture(field_1544_pShared_texture);
    return field_1544_pShared_texture;
}

// 0x5B9790
TextureCache_15D8::TextureCache_15D8()
{

    field_1000_bFreeCarDamageTextures = 0;
    field_1001_bFreeTileTextures = 0;
    field_15D4_next_digit_idx = 0;
    field_15D6_pal_count = 0;
    s32 i;

    for (i = 0; i < GTA2_COUNTOF_S(field_0_tile_textures); i++)
    {
        field_0_tile_textures[i] = nullptr;
    }

    for (i = 0; i < GTA2_COUNTOF_S(field_1004_car_damage_textures); i++)
    {
        field_1004_car_damage_textures[i] = nullptr;
    }
    field_1544_pShared_texture = nullptr;
}

// 0x5B9900
TextureCache_15D8::~TextureCache_15D8()
{
    if (field_1001_bFreeTileTextures)
    {
        for (auto& tile_texture : field_0_tile_textures)
        {
            if (tile_texture)
            {
                pgbh_FreeTexture(tile_texture);
                tile_texture = nullptr;
            }
        }
    }

    if (field_1000_bFreeCarDamageTextures)
    {
        for (auto& car_damage_texture : field_1004_car_damage_textures)
        {
            if (car_damage_texture)
            {
                pgbh_FreeTexture(car_damage_texture);
                car_damage_texture = nullptr;
            }
        }
    }

    for (auto& digit_texture : field_10C4_digit_textures)
    {
        if (digit_texture.field_4_pTexture)
        {
            pgbh_FreeTexture(digit_texture.field_4_pTexture);
            digit_texture.field_4_pTexture = nullptr;
        }
    }

    if (field_1544_pShared_texture)
    {
        pgbh_FreeTexture(field_1544_pShared_texture);
        field_1544_pShared_texture = nullptr;
    }

    FreePals_5B9140();
}