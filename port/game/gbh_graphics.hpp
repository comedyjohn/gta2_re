#pragma once

#include "Function.hpp"
// DmaVideo.hpp (GTA2Hax) includes <set> and <vector> for two members of its Renderer class, which no
// game code uses. Through them every TU that includes sprite.hpp got <new> and its throw() operator delete,
// and with it VC6 drops the EH frame of destructors that delete through an inline destructor
// (Door_4D4::~Door_4D4). The original TUs had no STL there, so unless a TU already has <set>/<vector>, their
// include guards are set and the two members get placeholder types.
#if !defined(_SET_) && !defined(_VECTOR_)
    #define _SET_
    #define _VECTOR_
namespace std
{
template<class T>
class vector
{
    T* mFirst;
    T* mLast;
    T* mEnd;
};
template<class T>
class set
{
    void* mHead;
    unsigned int mSize;
};
} // namespace std
#endif
#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp"
#include <windows.h>

s32 GBH_GraphicsLoadDll_5EA680(const char_type* lpLibFileName);

struct Vert;
struct SLight;
struct SImage;

using T_ConvertColourBank = void (*)(BYTE* unknown);
using T_DrawLine = s32 (*)(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5);
using T_SetShadeTableA = void (*)(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5);
using T_MakeScreenTable = sptr* (*)(sptr result, s32 a2, u32 a3);
using T_gbh_AddLight = s32 (*)(SLight* pLight);
using T_gbh_AssignPalette = void (*)(STexture* pTexture, s32 palId);
using T_gbh_BeginLevel = void (*)();
using T_gbh_SetBeginSceneCB = void (*)(TBeginSceneCB pBeginSceneCB);
using T_gbh_BeginScene = void (*)();
using T_gbh_BlitBuffer = s32 (*)(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
using T_gbh_BlitImage = s32 (*)(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
using T_gbh_CloseDLL = void (*)();
using T_gbh_CloseScreen = void (*)(SVideo* pVideo);
using T_gbh_Convert16BitGraphic = u32 (*)(s32 a1, u32 a2, WORD* a3, s32 a4);
using T_gbh_ConvertColour = u32 (*)(u8 a1, u8 a2, u8 a3);
using T_gbh_DrawFlatRect = s32 (*)(Vert* pVerts, s32 colour);
using T_gbh_DrawQuad = void (*)(s32 flags, STexture* pTexture, Vert* pVerts, s32 baseColour);
using T_gbh_DrawQuadClipped = void (*)(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5);
using T_gbh_DrawTile = int (*)(u32 flags, STexture* pTexture, Vert* pData, u8 diffuseColour);
using T_gbh_DrawTilePart = s32 (*)(u32 flags, STexture* pTexture, Vert* pData, s32 diffuseColour);
using T_gbh_DrawTriangle = void (*)(s32 triFlags, STexture* pTexture, Vert* pVerts, u8 diffuseColour);
using T_gbh_EndLevel = void (*)();
using T_gbh_EndScene = void (*)();
using T_gbh_FreeImageTable = s32 (*)();
using T_gbh_FreePalette = void (*)(s32 a1);
using T_gbh_FreeTexture = void (*)(STexture* pTexture);
using T_gbh_GetGlobals = u32* (*)();
using T_gbh_GetUsedCache = s32 (*)(s32 a1);
using T_gbh_Init = s32 (*)(s32 a1);
using T_gbh_InitDLL = u32 (*)(SVideo* pVideoDriver);
using T_gbh_InitImageTable = s32 (*)(s32 tableSize);
using T_gbh_LoadImage = s32 (*)(SImage* pImage);
using T_gbh_LockTexture = STexture* (*)(STexture* pTexture);
using T_gbh_Plot = void (*)(f32 x, f32 y, s32 a3, s32 colour);
using T_gbh_PrintBitmap = s32 (*)(s32 a1, s32 a2);
using T_gbh_RegisterPalette = u32 (*)(s32 a1, DWORD* a2);
using T_gbh_RegisterTexture = STexture* (*)(s32 width, s32 height, BYTE* pData, s32 pal_idx, char_type flag);
using T_gbh_ResetLights = void (*)();
using T_gbh_SetAmbient = void (*)(float a1);
using T_gbh_SetCamera = s32 (*)(float a1, float a2, float a3, float a4);
using T_gbh_SetColourDepth = s32 (*)();
using T_gbh_SetWindow = void (*)(float left, float top, float right, float bottom);
using T_gbh_UnlockTexture = STexture* (*)(STexture* pTexture);

EXTERN_GLOBAL(T_gbh_InitDLL, pgbh_InitDLL);

EXTERN_GLOBAL(T_gbh_Init, pgbh_Init);

EXTERN_GLOBAL(T_gbh_CloseDLL, pgbh_CloseDLL);

EXTERN_GLOBAL(T_gbh_SetWindow, pgbh_SetWindow);

EXTERN_GLOBAL(T_gbh_GetGlobals, pgbh_GetGlobals);

EXTERN_GLOBAL(T_gbh_DrawTile, pgbh_DrawTile);

EXTERN_GLOBAL(T_gbh_DrawTilePart, pgbh_DrawTilePart);

EXTERN_GLOBAL(T_gbh_DrawQuad, pgbh_DrawQuad);

EXTERN_GLOBAL(T_gbh_DrawQuadClipped, pgbh_DrawQuadClipped);

EXTERN_GLOBAL(T_gbh_DrawFlatRect, pgbh_DrawFlatRect);

EXTERN_GLOBAL(T_gbh_DrawTriangle, pgbh_DrawTriangle);

EXTERN_GLOBAL(T_gbh_PrintBitmap, pgbh_PrintBitmap);

EXTERN_GLOBAL(T_gbh_SetColourDepth, pgbh_SetColourDepth);

EXTERN_GLOBAL(T_gbh_Plot, pgbh_Plot);

EXTERN_GLOBAL(T_gbh_ConvertColour, pgbh_ConvertColour);

EXTERN_GLOBAL(T_gbh_RegisterTexture, pgbh_RegisterTexture);

EXTERN_GLOBAL(T_gbh_SetCamera, pgbh_SetCamera);

EXTERN_GLOBAL(T_gbh_ResetLights, pgbh_ResetLights);

EXTERN_GLOBAL(T_gbh_AddLight, pgbh_AddLight);

EXTERN_GLOBAL(T_gbh_SetAmbient, pgbh_SetAmbient);

EXTERN_GLOBAL(T_gbh_GetUsedCache, pgbh_GetUsedCache);

EXTERN_GLOBAL(T_gbh_FreeTexture, pgbh_FreeTexture);

EXTERN_GLOBAL(T_gbh_RegisterPalette, pgbh_RegisterPalette);

EXTERN_GLOBAL(T_gbh_FreePalette, pgbh_FreePalette);

EXTERN_GLOBAL(T_gbh_AssignPalette, pgbh_AssignPalette);

EXTERN_GLOBAL(T_gbh_LockTexture, pgbh_LockTexture);

EXTERN_GLOBAL(T_gbh_UnlockTexture, pgbh_UnlockTexture);

EXTERN_GLOBAL(T_gbh_BeginScene, pgbh_BeginScene);

EXTERN_GLOBAL(T_gbh_EndScene, pgbh_EndScene);

EXTERN_GLOBAL(T_gbh_BeginLevel, pgbh_BeginLevel);

EXTERN_GLOBAL(T_gbh_EndLevel, pgbh_EndLevel);

EXTERN_GLOBAL(T_MakeScreenTable, pMakeScreenTable);

EXTERN_GLOBAL(T_ConvertColourBank, pConvertColourBank);

EXTERN_GLOBAL(T_SetShadeTableA, pSetShadeTableA);

EXTERN_GLOBAL(T_gbh_InitImageTable, pgbh_InitImageTable);

EXTERN_GLOBAL(T_gbh_FreeImageTable, pgbh_FreeImageTable);

EXTERN_GLOBAL(T_gbh_LoadImage, pgbh_LoadImage);

EXTERN_GLOBAL(T_gbh_BlitImage, pgbh_BlitImage);

EXTERN_GLOBAL(T_gbh_BlitBuffer, pgbh_BlitBuffer);

EXTERN_GLOBAL(s32, gGBH_GraphicsLoaded_7085E0);

void GBH_Graphics_Stub_5EA4D0();
s32 GBH_GraphicsLoad_5EB680(const char_type* lpLibFileName, SVideo* pVidSys);
void GBH_Graphics_Unload_5EA500();
s32 GBH_Graphics_Free_5EA640();