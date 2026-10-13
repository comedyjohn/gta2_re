// The renderer the game loads at runtime: d3ddll.dll (the gbh_* drawing functions) and
// dmavideo.dll (the Vid_* display functions), both looked up by name with GetProcAddress
// (gbh_graphics.cpp, dma_video.cpp). Here they are native functions, handed out by
// port_renderer_lookup() when the game asks win32.cpp's GetProcAddress for them.
//
// Milestone 1: every function is a stub that reports success and draws nothing. Vid_Init_SYS offers
// one 640x480x16 mode on one device, which is what the game asks for when nothing is configured.
#include "renderer.h"
#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp"
#include <string.h>

namespace
{
SVideo gVideo;
SDevice gDevice;
SDisplayMode gMode;
SVidVersion gVersion = {0x10000, "GTA2 native renderer (stub)"};
u32 gGlobals[64];
STexture gTexture;
const s32 kModeId = 1;
} // namespace

// ---- dmavideo.dll
SVideo* CC Vid_Init_SYS(HINSTANCE, u16)
{
    memset(&gVideo, 0, sizeof(gVideo));
    memset(&gDevice, 0, sizeof(gDevice));
    memset(&gMode, 0, sizeof(gMode));

    gDevice.field_0_id = 0;
    gDevice.field_4_flags = 1; // keeps the window from minimising on WM_KILLFOCUS
    gDevice.field_8_driver_name = const_cast<char*>("native");
    gDevice.field_C_driver_desc = const_cast<char*>("GTA2 native renderer");

    gMode.field_0_display_mode_idx = kModeId;
    gMode.field_8_width = 640;
    gMode.field_C_height = 480;
    gMode.field_10_pitch = 640 * 2;
    gMode.field_14_rgb_bit_count = 16;

    gVideo.field_1C_num_display_modes = 1;
    gVideo.field_24_head_ptr = &gMode;
    gVideo.field_28_display_mode_array = &gMode;
    gVideo.field_2C_device_info_head_ptr = &gDevice;
    gVideo.field_30_device_info_last = &gDevice;
    gVideo.field_18_num_guids = 1;
    return &gVideo;
}

s32 CC Vid_CheckMode(SVideo*, s32 width, s32 height, s32 bpp)
{
    return (width == 640 && height == 480 && bpp == 16) ? kModeId : 0;
}

SDevice* CC Vid_FindDevice(SVideo*, s32)
{
    return &gDevice;
}

SDisplayMode* CC Vid_FindMode(SVideo*, s32 modeId)
{
    return modeId == kModeId ? &gMode : 0;
}

s32 CC Vid_FindFirstMode(SVideo* pVideo, s32)
{
    pVideo->field_3C_current_enum_ptr = &gMode;
    return kModeId;
}

s32 CC Vid_FindNextMode(SVideo* pVideo)
{
    pVideo->field_3C_current_enum_ptr = 0;
    return 0;
}

void CC Vid_CloseScreen(SVideo*)
{
}

s32 CC Vid_SetDevice(SVideo* pVideo, s32 deviceId)
{
    pVideo->field_34_active_device_id = deviceId;
    return 0;
}

s32 CC Vid_SetMode(SVideo* pVideo, HWND hWnd, s32 modeId)
{
    pVideo->field_4C0_hwnd = hWnd;
    pVideo->field_80_active_mode_q = modeId;
    pVideo->field_8_found_width = 640;
    pVideo->field_C_found_height = 480;
    pVideo->field_10_found_rgb_bit_count = 16;
    pVideo->field_48_rect_right = 640;
    pVideo->field_4C_rect_bottom = 480;
    return 0;
}

void CC Vid_GrabSurface(SVideo*)
{
}

void CC Vid_ReleaseSurface(SVideo*)
{
}

void CC Vid_FlipBuffers(SVideo*)
{
}

void CC Vid_ShutDown_SYS(SVideo*)
{
}

s32 CC Vid_EnableWrites(SVideo*)
{
    return 0;
}

s32 CC Vid_DisableWrites(SVideo*)
{
    return 0;
}

s32 CC Vid_GetSurface(SVideo*)
{
    return 0;
}

s32 CC Vid_FreeSurface(SVideo*)
{
    return 0;
}

s32 CC Vid_ClearScreen(SVideo*, u8, u8, u8, s32, s32, s32, s32)
{
    return 0;
}

s32 CC Vid_SetGamma(SVideo*, f32, f32, f32)
{
    return 0;
}

s32 CC Vid_WindowProc(SVideo*, HWND, DWORD, WPARAM, LPARAM)
{
    return 0;
}

s32 CC Vid_InitDLL(HINSTANCE, SVideoFunctions*)
{
    return 1;
}

SVidVersion* CC Vid_GetVersion()
{
    return &gVersion;
}

// ---- d3ddll.dll
void CC ConvertColourBank(s32)
{
}

int CC DrawLine(int, int, int, int, int)
{
    return 0;
}

void CC SetShadeTableA(int, int, int, int, int)
{
}

int* CC MakeScreenTable(int result, int, unsigned int)
{
    return reinterpret_cast<int*>(result);
}

int CC gbh_AddLight(SLight*)
{
    return 0;
}

void CC gbh_AssignPalette(STexture*, int)
{
}

void CC gbh_BeginLevel()
{
}

void CC gbh_SetBeginSceneCB(TBeginSceneCB)
{
}

void CC gbh_BeginScene()
{
}

int CC gbh_BlitBuffer(int, int, int, int, int, int)
{
    return 0;
}

int CC gbh_BlitImage(int, int, int, int, int, int, int)
{
    return 0;
}

void CC gbh_CloseDLL()
{
}

void CC gbh_CloseScreen(SVideo*)
{
}

unsigned int CC gbh_Convert16BitGraphic(int, unsigned int, WORD*, signed int)
{
    return 0;
}

unsigned int CC gbh_ConvertColour(unsigned __int8 r, unsigned __int8 g, unsigned __int8 b)
{
    return (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | b;
}

int CC gbh_DrawFlatRect(int, int)
{
    return 0;
}

void CC gbh_DrawQuad(int, STexture*, Vert*, int)
{
}

void CC gbh_DrawQuadClipped(int, int, int, int, int)
{
}

s32 CC gbh_DrawTilePart(unsigned int, STexture*, Vert*, int)
{
    return 0;
}

s32 CC gbh_DrawTile(int, STexture*, Vert*, int)
{
    return 0;
}

void CC gbh_DrawTriangle(int, STexture*, Vert*, int)
{
}

void CC gbh_EndLevel()
{
}

void CC gbh_EndScene()
{
}

int CC gbh_FreeImageTable()
{
    return 0;
}

void CC gbh_FreePalette(int)
{
}

void CC gbh_FreeTexture(STexture*)
{
}

u32* CC gbh_GetGlobals()
{
    return gGlobals;
}

int CC gbh_GetUsedCache(int)
{
    return 0;
}

s32 CC gbh_Init(int)
{
    return 0;
}

u32 CC gbh_InitDLL(SVideo*)
{
    return 1;
}

signed int CC gbh_InitImageTable(int)
{
    return 0;
}

signed int CC gbh_LoadImage(SImage*)
{
    return 0;
}

STexture* CC gbh_LockTexture(STexture* pTexture)
{
    return pTexture;
}

void CC gbh_Plot(int, int, int, int)
{
}

int CC gbh_PrintBitmap(int, int)
{
    return 0;
}

unsigned int CC gbh_RegisterPalette(int, DWORD*)
{
    return 0;
}

STexture* CC gbh_RegisterTexture(s32, s32, BYTE*, s32, char_type)
{
    return &gTexture;
}

void CC gbh_ResetLights()
{
}

void CC gbh_SetAmbient(float)
{
}

int CC gbh_SetCamera(float, float, float, float)
{
    return 0;
}

int CC gbh_SetColourDepth()
{
    return 16;
}

void CC gbh_SetWindow(float, float, float, float)
{
}

STexture* CC gbh_UnlockTexture(STexture* pTexture)
{
    return pTexture;
}

// ---- lookup by exported name
#define ENTRY(name) {#name, reinterpret_cast<FARPROC>(&name)}

namespace
{
struct Export
{
    const char* mName;
    FARPROC mFunc;
};

const Export kExports[] = {
    ENTRY(Vid_Init_SYS),        ENTRY(Vid_CheckMode),     ENTRY(Vid_FindDevice),      ENTRY(Vid_FindMode),
    ENTRY(Vid_FindFirstMode),   ENTRY(Vid_FindNextMode),  ENTRY(Vid_CloseScreen),     ENTRY(Vid_SetDevice),
    ENTRY(Vid_SetMode),         ENTRY(Vid_GrabSurface),   ENTRY(Vid_ReleaseSurface),  ENTRY(Vid_FlipBuffers),
    ENTRY(Vid_ShutDown_SYS),    ENTRY(Vid_EnableWrites),  ENTRY(Vid_DisableWrites),   ENTRY(Vid_GetSurface),
    ENTRY(Vid_FreeSurface),     ENTRY(Vid_ClearScreen),   ENTRY(Vid_SetGamma),        ENTRY(Vid_WindowProc),
    ENTRY(Vid_InitDLL),         ENTRY(Vid_GetVersion),    ENTRY(ConvertColourBank),   ENTRY(DrawLine),
    ENTRY(SetShadeTableA),      ENTRY(MakeScreenTable),   ENTRY(gbh_AddLight),        ENTRY(gbh_AssignPalette),
    ENTRY(gbh_BeginLevel),      ENTRY(gbh_SetBeginSceneCB), ENTRY(gbh_BeginScene),    ENTRY(gbh_BlitBuffer),
    ENTRY(gbh_BlitImage),       ENTRY(gbh_CloseDLL),      ENTRY(gbh_CloseScreen),     ENTRY(gbh_Convert16BitGraphic),
    ENTRY(gbh_ConvertColour),   ENTRY(gbh_DrawFlatRect),  ENTRY(gbh_DrawQuad),        ENTRY(gbh_DrawQuadClipped),
    ENTRY(gbh_DrawTilePart),    ENTRY(gbh_DrawTile),      ENTRY(gbh_DrawTriangle),    ENTRY(gbh_EndLevel),
    ENTRY(gbh_EndScene),        ENTRY(gbh_FreeImageTable), ENTRY(gbh_FreePalette),    ENTRY(gbh_FreeTexture),
    ENTRY(gbh_GetGlobals),      ENTRY(gbh_GetUsedCache),  ENTRY(gbh_Init),            ENTRY(gbh_InitDLL),
    ENTRY(gbh_InitImageTable),  ENTRY(gbh_LoadImage),     ENTRY(gbh_LockTexture),     ENTRY(gbh_Plot),
    ENTRY(gbh_PrintBitmap),     ENTRY(gbh_RegisterPalette), ENTRY(gbh_RegisterTexture), ENTRY(gbh_ResetLights),
    ENTRY(gbh_SetAmbient),      ENTRY(gbh_SetCamera),     ENTRY(gbh_SetColourDepth),  ENTRY(gbh_SetWindow),
    ENTRY(gbh_UnlockTexture),
};
} // namespace

FARPROC port_renderer_lookup(const char* name)
{
    for (const Export& e : kExports)
    {
        if (strcmp(e.mName, name) == 0)
        {
            return e.mFunc;
        }
    }
    return 0;
}

bool port_is_renderer_library(const char* fileName)
{
    const char* base = strrchr(fileName, '\\');
    base = base ? base + 1 : fileName;
    const char* slash = strrchr(base, '/');
    base = slash ? slash + 1 : base;
    return strcasecmp(base, "d3ddll.dll") == 0 || strcasecmp(base, "dmavideo.dll") == 0;
}
