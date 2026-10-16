// gta2 --selftest: draws through the renderer's entry points the way the game does (a palette, a
// paletted 64x64 tile with a transparent corner, an additive quad, a 16-bit TGA image), reads the
// pixels back and checks them. Needs a display (Xvfb will do); no game data.
#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp"
#include "renderer.h"

#include <stdio.h>
#include <string.h>

#include <vector>

namespace
{
template<class T>
T Get(const char* name)
{
    return reinterpret_cast<T>(port_renderer_lookup(name));
}

struct Picture
{
    std::vector<DWORD> mPixels; // RGBA bytes, bottom row first
    int mW;
    int mH;

    // The colour at a point of the game's 640x480 picture, as 0xRRGGBB
    unsigned At(int gx, int gy) const
    {
        const int x = gx * mW / 640 + mW / 1280;
        const int y = mH - 1 - (gy * mH / 480 + mH / 960);
        const DWORD p = mPixels[static_cast<size_t>(y) * mW + x];
        return ((p & 0xFF) << 16) | (p & 0xFF00) | ((p >> 16) & 0xFF);
    }
};

int gFailures = 0;

void Expect(const Picture& pic, int x, int y, unsigned rgb, const char* what)
{
    const unsigned got = pic.At(x, y);
    const bool ok = got == rgb;
    gFailures += ok ? 0 : 1;
    printf("%s %-44s at (%3d,%3d): expected %06X, got %06X\n", ok ? "PASS" : "FAIL", what, x, y, rgb, got);
}

void Quad(Vert* v, float x0, float y0, float x1, float y1)
{
    memset(v, 0, sizeof(Vert) * 8);
    const float xs[4] = {x0, x1, x1, x0};
    const float ys[4] = {y0, y0, y1, y1};
    for (int i = 0; i < 4; i++)
    {
        v[i].x = xs[i];
        v[i].y = ys[i];
        v[i].z = 1.0f;
    }
}
} // namespace

int port_selftest()
{
    SVideo* video = Get<SVideo*(CC*)(HINSTANCE, u16)>("Vid_Init_SYS")(0, 0);
    if (Get<s32(CC*)(SVideo*, HWND, s32)>("Vid_SetMode")(video, 0, 1) != 0)
    {
        printf("FAIL couldn't open the window\n");
        return 1;
    }
    Get<u32(CC*)(SVideo*)>("gbh_InitDLL")(video);
    Get<void(CC*)(float)>("gbh_SetAmbient")(1.0f);

    // Blue background
    Get<s32(CC*)(SVideo*, u8, u8, u8, s32, s32, s32, s32)>("Vid_ClearScreen")(video, 0, 0, 255, 0, 0, 0, 0);

    // A palette (stored in columns, 64 DWORDs apart): 1 red, 2 green
    static DWORD pal[256 * 64];
    pal[1 * 64] = 0xFF0000;
    pal[2 * 64] = 0x00FF00;
    Get<unsigned(CC*)(int, DWORD*)>("gbh_RegisterPalette")(7, pal);

    // A 64x64 tile in a 256-wide page: left half red, right half green, top-left 16x16 transparent
    static BYTE page[256 * 64];
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            page[y * 256 + x] = (x < 16 && y < 16) ? 0 : (x < 32 ? 1 : 2);
        }
    }
    STexture* tile = Get<STexture*(CC*)(s32, s32, BYTE*, s32, char_type)>("gbh_RegisterTexture")(64, 64, page, 7, 0);

    Vert v[8];
    Quad(v, 100, 100, 228, 228); // the 64x64 tile drawn at twice its size
    Get<s32(CC*)(unsigned, STexture*, Vert*, int)>("gbh_DrawTilePart")(0, tile, v, 255);

    // The same tile rotated a quarter turn (flags 0x20): the tile's bottom-left texel lands top-left
    Quad(v, 300, 100, 428, 228);
    Get<s32(CC*)(unsigned, STexture*, Vert*, int)>("gbh_DrawTilePart")(0x20, tile, v, 255);

    // Additive blending (0x200): the red half added onto the blue background is magenta
    Quad(v, 100, 300, 228, 428);
    for (int i = 0; i < 4; i++)
    {
        v[i].u = (i == 1 || i == 2) ? 64.0f : 0.0f;
        v[i].v = (i >= 2) ? 64.0f : 0.0f;
    }
    Get<void(CC*)(int, STexture*, Vert*, int)>("gbh_DrawQuad")(0x200, tile, v, 255);

    // A 16-bit TGA, bottom row first: white above, yellow below
    const int w = 8;
    const int h = 8;
    std::vector<BYTE> tga(0x12 + w * h * 2, 0);
    tga[2] = 2; // uncompressed true-colour
    tga[0xC] = w;
    tga[0xE] = h;
    tga[0x10] = 16;
    for (int row = 0; row < h; row++) // file order: bottom row first
    {
        const u16 colour = row < h / 2 ? 0x7FE0 /* yellow */ : 0x7FFF /* white */;
        for (int x = 0; x < w; x++)
        {
            memcpy(&tga[0x12 + (row * w + x) * 2], &colour, 2);
        }
    }
    Get<signed(CC*)(int)>("gbh_InitImageTable")(4);
    const int image = Get<signed(CC*)(SImage*)>("gbh_LoadImage")(reinterpret_cast<SImage*>(tga.data()));
    Get<int(CC*)(int, int, int, int, int, int, int)>("gbh_BlitImage")(image, 0, 0, w, h, 500, 300);

    Picture pic;
    port_renderer_read_pixels(pic.mPixels, pic.mW, pic.mH);
    Expect(pic, 50, 50, 0x0000FF, "clear colour");
    Expect(pic, 110, 110, 0x0000FF, "tile: transparent texels show the background");
    Expect(pic, 140, 180, 0xFF0000, "tile: left half red");
    Expect(pic, 200, 180, 0x00FF00, "tile: right half green");
    Expect(pic, 310, 110, 0xFF0000, "rotated tile: top-left is red");
    Expect(pic, 418, 110, 0x0000FF, "rotated tile: transparent corner top-right");
    Expect(pic, 418, 218, 0x00FF00, "rotated tile: bottom-right is green");
    Expect(pic, 140, 360, 0xFF00FF, "additive quad: red + blue");
    Expect(pic, 503, 301, 0xFFFFFF, "TGA image: top half white");
    Expect(pic, 503, 306, 0xFFFF00, "TGA image: bottom half yellow");

    Get<void(CC*)(SVideo*)>("Vid_FlipBuffers")(video);
    printf(gFailures ? "SELFTEST FAILED (%d)\n" : "SELFTEST PASSED\n", gFailures);
    return gFailures ? 1 : 0;
}
