// The renderer the game loads at runtime: d3ddll.dll (the gbh_* drawing functions) and
// dmavideo.dll (the Vid_* display functions), both looked up by name with GetProcAddress
// (gbh_graphics.cpp, dma_video.cpp). Here they are native functions over OpenGL 2.1, handed out by
// port_renderer_lookup() when the game asks win32.cpp's GetProcAddress for them.
//
// What each function does follows GTA2Hax's Direct3D 9 reimplementation of the two DLLs
// (3rdParty/GTA2Hax: d3ddll/d3ddll.cpp and DmaVideo/DmaVideo.cpp, CriminalRETeam), with the Direct3D
// calls replaced by OpenGL ones. The game draws pre-transformed vertices in a 640x480 space; that is
// scaled into the window, keeping 4:3 with black bars.
#include "renderer.h"
#include "gl.h"
#include "platform.h"
#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <vector>

// ---------------------------------------------------------------- OpenGL loading
#define PORT_GL_DEFINE(ret, name, params) PFN_##name name = 0;
PORT_GL_FUNCTIONS(PORT_GL_DEFINE)
#undef PORT_GL_DEFINE

bool port_gl_load()
{
    bool ok = true;
#define PORT_GL_LOAD(ret, name, params)                                       \
    name = reinterpret_cast<PFN_##name>(plat::GLGetProcAddress(#name));       \
    if (!name)                                                                \
    {                                                                         \
        fprintf(stderr, "OpenGL function %s is unavailable\n", #name);        \
        ok = false;                                                           \
    }
    PORT_GL_FUNCTIONS(PORT_GL_LOAD)
#undef PORT_GL_LOAD
    return ok;
}

namespace
{
const float kGameWidth = 640.0f;
const float kGameHeight = 480.0f;

// ---------------------------------------------------------------- the OpenGL backend
struct GLVertex
{
    float x, y, z, rhw;
    DWORD diff; // D3DCOLOR: bytes B, G, R, A in memory
    float u, v;
};

class GLRenderer
{
  public:
    bool Open(HWND)
    {
        if (mReady)
        {
            return true;
        }
        if (!plat::OpenWindow(1280, 960, "GTA2"))
        {
            return false;
        }
        if (!port_gl_load())
        {
            return false;
        }
        printf("OpenGL %s on %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

        static const char* kVertex =
            "#version 120\n"
            "attribute vec4 aPos;\n" // x, y in game pixels; w = 1/w for perspective-correct texturing
            "attribute vec4 aColor;\n"
            "attribute vec2 aUV;\n"
            "uniform vec2 uScale;\n"
            "varying vec4 vColor;\n"
            "varying vec2 vUV;\n"
            "void main()\n"
            "{\n"
            "    float w = aPos.w > 0.0 ? 1.0 / aPos.w : 1.0;\n"
            "    vec2 ndc = vec2(aPos.x * uScale.x - 1.0, 1.0 - aPos.y * uScale.y);\n"
            "    gl_Position = vec4(ndc * w, 0.0, w);\n"
            "    vColor = aColor.bgra;\n" // D3DCOLOR is ARGB: B, G, R, A in memory
            "    vUV = aUV;\n"
            "}\n";
        static const char* kFragment =
            "#version 120\n"
            "uniform sampler2D uTex;\n"
            "uniform int uModulateAlpha;\n"
            "varying vec4 vColor;\n"
            "varying vec2 vUV;\n"
            "void main()\n"
            "{\n"
            "    vec4 t = texture2D(uTex, vUV);\n"
            "    if (t.a < 0.5)\n" // palette colour 0 is transparent
            "        discard;\n"
            "    float a = uModulateAlpha != 0 ? t.a * vColor.a : t.a;\n"
            "    gl_FragColor = vec4(t.rgb * vColor.rgb, a);\n"
            "}\n";
        mProgram = Link(kVertex, kFragment);
        if (!mProgram)
        {
            return false;
        }
        glUseProgram(mProgram);
        mScaleLoc = glGetUniformLocation(mProgram, "uScale");
        mModulateAlphaLoc = glGetUniformLocation(mProgram, "uModulateAlpha");
        glUniform1i(glGetUniformLocation(mProgram, "uTex"), 0);
        glUniform2f(mScaleLoc, 2.0f / kGameWidth, 2.0f / kGameHeight);
        glUniform1i(mModulateAlphaLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLVertex), &mVerts[0].x);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLVertex), &mVerts[0].diff);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), &mVerts[0].u);

        // A 1x1 white texture for untextured draws
        const DWORD white = 0xFFFFFFFF;
        glGenTextures(1, &mWhite);
        Upload(mWhite, 1, 1, &white);

        mReady = true;
        SetViewport();
        Clear(0, 0, 0, 0, 0, 0, 0);
        return true;
    }

    bool Ready() const
    {
        return mReady;
    }

    void Clear(u8 r, u8 g, u8 b, s32 left, s32 top, s32 right, s32 bottom)
    {
        if (!mReady)
        {
            return;
        }
        const bool useRect = !(left == 0 && top == 0 && right == 0 && bottom == 0);
        if (useRect)
        {
            glEnable(GL_SCISSOR_TEST);
            const float sx = mViewW / kGameWidth;
            const float sy = mViewH / kGameHeight;
            glScissor(mViewX + static_cast<int>(left * sx), mViewY + static_cast<int>((kGameHeight - bottom) * sy),
                      static_cast<int>((right - left) * sx), static_cast<int>((bottom - top) * sy));
        }
        glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (useRect)
        {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    // GTA2Hax's Renderer::SetRenderStates. The modulated-alpha texture stage state, once set, stays
    // set in Direct3D, so it does here too.
    void SetRenderStates(u32 flags)
    {
        if (!mReady)
        {
            return;
        }
        if (flags & 0x200)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        else if (flags & 0x180)
        {
            if (!mModulateAlpha)
            {
                mModulateAlpha = true;
                glUniform1i(mModulateAlphaLoc, 1);
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDisable(GL_BLEND);
        }
        mFilter = (flags & 0x20000) ? GL_LINEAR : GL_NEAREST;
    }

    void Draw(const GLVertex* verts, u32 count, GLuint texture)
    {
        if (!mReady || count < 3 || count > 4)
        {
            return;
        }
        memcpy(mVerts, verts, sizeof(GLVertex) * count);
        glBindTexture(GL_TEXTURE_2D, texture ? texture : mWhite);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);
        glDrawArrays(count == 4 ? GL_TRIANGLE_FAN : GL_TRIANGLES, 0, count);
    }

    GLuint CreateTexture()
    {
        GLuint id = 0;
        glGenTextures(1, &id);
        return id;
    }

    void Upload(GLuint id, u32 w, u32 h, const DWORD* argb)
    {
        // ARGB words are B, G, R, A bytes; swap to RGBA for glTexImage2D
        mScratch.resize(static_cast<size_t>(w) * h);
        for (size_t i = 0; i < mScratch.size(); i++)
        {
            const DWORD p = argb[i];
            mScratch[i] = (p & 0xFF00FF00) | ((p >> 16) & 0xFF) | ((p & 0xFF) << 16);
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mScratch.data());
    }

    void FreeTextureDelayed(GLuint id)
    {
        if (id)
        {
            mFreeLater.push_back(id);
        }
    }

    void Present()
    {
        if (!mReady)
        {
            return;
        }
        plat::SwapWindow();
        if (!mFreeLater.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(mFreeLater.size()), mFreeLater.data());
            mFreeLater.clear();
        }
        // The bars around the 4:3 picture (glClear ignores the viewport), and the viewport for a window
        // that may have changed size
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        SetViewport();
    }

    // Reads back the game's 640x480 picture (for the headless self-test)
    void ReadPixels(std::vector<DWORD>& out, int& w, int& h)
    {
        w = mViewW;
        h = mViewH;
        out.resize(static_cast<size_t>(w) * h);
        glReadPixels(mViewX, mViewY, w, h, GL_RGBA, GL_UNSIGNED_BYTE, out.data());
    }

  private:
    void SetViewport()
    {
        int dw = 0;
        int dh = 0;
        plat::GetDrawableSize(dw, dh);
        mViewW = dw;
        mViewH = dw * 3 / 4;
        if (mViewH > dh)
        {
            mViewH = dh;
            mViewW = dh * 4 / 3;
        }
        mViewX = (dw - mViewW) / 2;
        mViewY = (dh - mViewH) / 2;
        glViewport(mViewX, mViewY, mViewW, mViewH);
    }

    static GLuint Compile(GLenum type, const char* src)
    {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, 0);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetShaderInfoLog(s, sizeof(log), 0, log);
            fprintf(stderr, "Shader compile failed: %s\n", log);
            return 0;
        }
        return s;
    }

    static GLuint Link(const char* vs, const char* fs)
    {
        GLuint v = Compile(GL_VERTEX_SHADER, vs);
        GLuint f = Compile(GL_FRAGMENT_SHADER, fs);
        if (!v || !f)
        {
            return 0;
        }
        GLuint p = glCreateProgram();
        glAttachShader(p, v);
        glAttachShader(p, f);
        glBindAttribLocation(p, 0, "aPos");
        glBindAttribLocation(p, 1, "aColor");
        glBindAttribLocation(p, 2, "aUV");
        glLinkProgram(p);
        GLint ok = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            char log[1024];
            glGetProgramInfoLog(p, sizeof(log), 0, log);
            fprintf(stderr, "Shader link failed: %s\n", log);
            return 0;
        }
        return p;
    }

    bool mReady = false;
    GLuint mProgram = 0;
    GLint mScaleLoc = -1;
    GLint mModulateAlphaLoc = -1;
    bool mModulateAlpha = false;
    GLint mFilter = GL_NEAREST;
    GLuint mWhite = 0;
    GLVertex mVerts[4];
    std::vector<DWORD> mScratch;
    std::vector<GLuint> mFreeLater;
    int mViewX = 0;
    int mViewY = 0;
    int mViewW = 640;
    int mViewH = 480;
};

GLRenderer gRenderer;

// The GL texture name lives in STexture::field_1C_ptr, where Direct3D kept its texture
GLuint TextureId(const STexture* pTexture)
{
    return static_cast<GLuint>(reinterpret_cast<uintptr_t>(pTexture->field_1C_ptr));
}

void SetTextureId(STexture* pTexture, GLuint id)
{
    pTexture->field_1C_ptr = reinterpret_cast<IDirect3DTexture9*>(static_cast<uintptr_t>(id));
}

// ---------------------------------------------------------------- colour conversion (GTA2Hax)
u16 RGB888ToRGB565(u32 rgb888)
{
    const u8 r = (rgb888 >> 16) & 0xFF;
    const u8 g = (rgb888 >> 8) & 0xFF;
    const u8 b = rgb888 & 0xFF;
    return static_cast<u16>((((r * 31) / 255) << 11) | (((g * 63) / 255) << 5) | ((b * 31) / 255));
}

u32 RGB565ToRGB888(u16 c)
{
    const u32 r = (((c >> 11) & 0x1F) * 255) / 31;
    const u32 g = (((c >> 5) & 0x3F) * 255) / 63;
    const u32 b = ((c & 0x1F) * 255) / 31;
    return (r << 16) | (g << 8) | b;
}

u32 RGB555ToRGB888(u16 c)
{
    const u32 r = (c & 0x7C00) >> 10;
    const u32 g = (c & 0x03E0) >> 5;
    const u32 b = c & 0x001F;
    return (((r << 3) | (r >> 2)) << 16) | (((g << 3) | (g >> 2)) << 8) | ((b << 3) | (b >> 2));
}

// GTA2Hax's Renderer::SetTexture: an 8-bit paletted texture laid out in 256-byte rows (tile and sprite
// pages), or a 16-bit bottom-up TGA image whose pixels start pitch bytes in.
GLuint SetTexture(GLuint existing, u32 w, u32 h, const u8* pPixelData, const u16* pPalData, const s32* pitch)
{
    if (!gRenderer.Ready())
    {
        return existing;
    }
    std::vector<DWORD> argb(static_cast<size_t>(w) * h, 0);
    if (pPixelData && pPalData)
    {
        for (u32 y = 0; y < h; y++)
        {
            const u8* row = pPixelData + static_cast<size_t>(y) * 256;
            for (u32 x = 0; x < w; x++)
            {
                const u8 idx = row[x];
                argb[static_cast<size_t>(y) * w + x] = RGB565ToRGB888(pPalData[idx]) | (idx == 0 ? 0 : 0xFF000000u);
            }
        }
    }
    else if (pPixelData && pitch)
    {
        const u16* src = reinterpret_cast<const u16*>(pPixelData + *pitch);
        for (u32 y = 0; y < h; y++)
        {
            for (u32 x = 0; x < w; x++)
            {
                u16 c;
                memcpy(&c, src + static_cast<size_t>(y) * w + x, sizeof(c));
                argb[static_cast<size_t>(h - 1 - y) * w + x] = RGB555ToRGB888(c) | 0xFF000000u;
            }
        }
    }
    GLuint id = existing ? existing : gRenderer.CreateTexture();
    gRenderer.Upload(id, w, h, argb.data());
    return id;
}

// ---------------------------------------------------------------- d3ddll.dll's state (GTA2Hax)
const DWORD kSGlobalsValues[] = {10, 62, 62, 88, 1, 0, 110, 126, 126, 40, 10, 0};

struct SGlobals
{
    DWORD mNumPolysDrawn;
    DWORD mNumTextureSwaps;
    DWORD mNumBatchFlushes;
    DWORD mSceneTime;
    DWORD mCacheSizes[12];
    DWORD mCacheSizes2[12];
    DWORD mCacheHitRates[12];
    DWORD mCacheUnknown[12];
    void* mCache[12];
};
SGlobals gGlobals;

struct SImageTableEntry
{
    BOOL bLoaded;
    DWORD field_4_w;
    DWORD field_8_h;
    STexture* field_C_pTexture;
};

struct SPalData
{
    DWORD* mPOriginalData;
    WORD* mPData;
    DWORD mbLoaded;
};

struct SLightInternal
{
    DWORD flags;
    float brightness;
    float radius;
    float radiusSquared;
    float radiusNormalized;
    float x, y, z;
    float r, g, b;
};

float gWindowLeft;
float gWindowRight;
float gWindowTop;
float gWindowBottom;
int gScreenTableSize = 0;
intptr_t gScreenTable[1700]; // the address of each row of the surface
float gAmbient = 1.0f;
SImageTableEntry* gImageTable = 0;
DWORD gImageTableCount = 0;
u32 gTextureId = 0;
SPalData gPals[16384];
SLightInternal gLights[256];
DWORD gNumLights = 0;

// ---------------------------------------------------------------- dmavideo.dll's state
SVideo gVideo;
SDevice gDevice;
SDisplayMode gMode;
SVidVersion gVersion = {0x10000, "GTA2 native renderer (OpenGL)"};
const s32 kModeId = 1;

// ---------------------------------------------------------------- vertex processing (GTA2Hax)
const u32 FLAG_FIT_TO_TEXTURE = 0x10000;
const u32 FLAG_TEXTURE_FILTERING = 0x20000;
const u32 FLAG_LIGHTING_SHADOW = 0x8000;
const u32 FLAG_USE_ALPHA_DIFFUSE = 0x2000;

s32 ComputeLighting(const Vert& vert, u8 colorMultiplier, s32 vertexColor)
{
    float lr = 0.0f;
    float lg = 0.0f;
    float lb = 0.0f;
    for (DWORD j = 0; j < gNumLights; j++)
    {
        const SLightInternal& light = gLights[j];
        if ((light.flags & 0x30000) != 0x10000)
        {
            continue;
        }
        const float dx = vert.x - light.x;
        const float dy = vert.y - light.y;
        const float dz = vert.z - light.z;
        const float d2 = dx * dx + dy * dy + dz * dz;
        if (d2 > light.radiusSquared)
        {
            continue;
        }
        const float attenuation = (light.radius - sqrtf(d2)) * light.radiusNormalized;
        if (attenuation > 0.0f)
        {
            const float brightness = attenuation * light.brightness;
            lr += light.r * brightness;
            lg += light.g * brightness;
            lb += light.b * brightness;
        }
    }
    const float n = colorMultiplier / 255.0f;
    const s32 r = static_cast<s32>(std::min(n * ((vertexColor >> 16) & 0xFF) * lr + gAmbient, 255.0f));
    const s32 g = static_cast<s32>(std::min(n * ((vertexColor >> 8) & 0xFF) * lg + gAmbient, 255.0f));
    const s32 b = static_cast<s32>(std::min(n * (vertexColor & 0xFF) * lb + gAmbient, 255.0f));
    return static_cast<s32>(vertexColor & 0xFF000000) | b | (g << 8) | (r << 16);
}

void RenderVerts(int flags, STexture* pTexture, Vert* pVerts, int vertsCount, int baseColour)
{
    if (vertsCount < 3 || vertsCount > 4 || pVerts[0].z <= 0.0f || !pTexture)
    {
        return;
    }
    if (flags & FLAG_FIT_TO_TEXTURE)
    {
        flags |= FLAG_TEXTURE_FILTERING;
    }
    gRenderer.SetRenderStates(flags);

    const float tw = static_cast<float>(pTexture->field_E_width);
    const float th = static_cast<float>(pTexture->field_10_height);

    if (flags & FLAG_FIT_TO_TEXTURE)
    {
        static const float offsets[4][4] = {{0, 0, 0, 0}, {1, 0, 1, 0}, {1, 1, 1, 1}, {0, 1, 0, 1}};
        for (int i = 0; i < vertsCount; i++)
        {
            pVerts[i].x = pVerts[0].x + offsets[i][0] * tw;
            pVerts[i].y = pVerts[0].y + offsets[i][1] * th;
            pVerts[i].z = pVerts[0].z;
            pVerts[i].u = offsets[i][2] * tw;
            pVerts[i].v = offsets[i][3] * th;
        }
    }
    for (int i = 0; i < vertsCount; i++)
    {
        pVerts[i].w = pVerts[i].z;
    }
    if (!(flags & FLAG_USE_ALPHA_DIFFUSE))
    {
        const u32 c = static_cast<u8>(baseColour);
        for (int i = 0; i < vertsCount; i++)
        {
            pVerts[i].diff = c | (c << 8) | (c << 16) | 0xFF000000u;
            pVerts[i].spec = 255;
        }
    }
    if ((flags & FLAG_LIGHTING_SHADOW) && gAmbient != 255.0f)
    {
        // The game passes the lit positions in the four verts after the drawn ones
        for (int i = 0; i < vertsCount; i++)
        {
            pVerts[i].diff = ComputeLighting(pVerts[i + 4], static_cast<u8>(baseColour), pVerts[i].diff);
        }
    }

    GLVertex out[4];
    for (int i = 0; i < vertsCount; i++)
    {
        // Direct3D samples pixel centres at .0, OpenGL at .5: GTA2Hax's -0.5 shift has no counterpart here
        out[i].x = pVerts[i].x;
        out[i].y = pVerts[i].y;
        out[i].z = pVerts[i].z;
        out[i].rhw = pVerts[i].w;
        out[i].diff = pVerts[i].diff;
        out[i].u = pVerts[i].u / tw;
        out[i].v = pVerts[i].v / th;
    }
    gGlobals.mNumPolysDrawn++;
    gRenderer.Draw(out, vertsCount, TextureId(pTexture));
}

STexture* InternalRegisterTexture(s32 width, s32 height, BYTE* pData, s32* palIdx, s32* pitch)
{
    STexture* pTexture = static_cast<STexture*>(malloc(sizeof(STexture)));
    if (!pTexture)
    {
        return 0;
    }
    memset(pTexture, 0, sizeof(STexture));
    pTexture->field_0_id = static_cast<u16>(gTextureId++);
    pTexture->field_E_width = static_cast<u16>(width);
    pTexture->field_10_height = static_cast<u16>(height);
    pTexture->field_14_original_pixel_data_ptr = pData;
    if (palIdx)
    {
        pTexture->field_12_bPalIsValid = static_cast<u8>(gPals[*palIdx].mbLoaded);
        pTexture->field_18_pPaltData = gPals[*palIdx].mPData;
    }
    SetTextureId(pTexture, SetTexture(0, width, height, pData, pTexture->field_18_pPaltData, pitch));
    return pTexture;
}
} // namespace

// ================================================================ dmavideo.dll
SVideo* CC Vid_Init_SYS(HINSTANCE, u16)
{
    memset(&gVideo, 0, sizeof(gVideo));
    memset(&gDevice, 0, sizeof(gDevice));
    memset(&gMode, 0, sizeof(gMode));

    gDevice.field_0_id = 0;
    gDevice.field_4_flags = 1; // keeps the window from minimising on WM_KILLFOCUS
    gDevice.field_8_driver_name = const_cast<char*>("opengl");
    gDevice.field_C_driver_desc = const_cast<char*>("OpenGL");

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

// The game always draws at 640x480; any 16-bit mode it asks for maps to that, scaled to the window
s32 CC Vid_CheckMode(SVideo*, s32, s32, s32 bpp)
{
    return bpp == 16 ? kModeId : 0;
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
    if (!gRenderer.Open(hWnd))
    {
        MessageBoxA(0, "Couldn't open an OpenGL 2.1 window", "GTA2", MB_OK);
        return 1;
    }
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
    gRenderer.Present();
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

s32 CC Vid_ClearScreen(SVideo*, u8 aR, u8 aG, u8 aB, s32 aLeft, s32 aTop, s32 aRight, s32 aBottom)
{
    gRenderer.Clear(aR, aG, aB, aLeft, aTop, aRight, aBottom);
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

// ================================================================ d3ddll.dll
void CC ConvertColourBank(s32)
{
}

int CC DrawLine(int, int a2, int, int a4, int)
{
    return abs(a4 - a2);
}

void CC SetShadeTableA(int, int, int, int, int)
{
}

// MakeScreenTable with pointer-sized rows (d3ddll.hpp declares it with int ones, which can't hold an
// address at 64 bits); exported under the original name below
static intptr_t* CC MakeScreenTablePtr(intptr_t value, int elementSize, unsigned int size)
{
    intptr_t* result = 0;
    gScreenTableSize = static_cast<int>(size);
    if (size)
    {
        result = gScreenTable;
        do
        {
            *result++ = value;
            value += elementSize;
        } while (--size);
    }
    return result;
}

int CC gbh_AddLight(SLight* pLight)
{
    if (gNumLights >= 256)
    {
        return 0;
    }
    const DWORD idx = gNumLights++;
    SLightInternal& l = gLights[idx];
    l.flags = pLight->field_0;
    l.brightness = static_cast<float>(pLight->field_0 & 0xFF) * 0.0039215689f;
    l.x = pLight->field_4_x;
    l.y = pLight->field_8_y;
    l.z = pLight->field_C_z;
    l.r = static_cast<float>((pLight->field_10_colour >> 16) & 0xFF) * 0.0039215689f;
    l.g = static_cast<float>((pLight->field_10_colour >> 8) & 0xFF) * 0.0039215689f;
    l.b = static_cast<float>(pLight->field_10_colour & 0xFF) * 0.0039215689f;
    l.radius = ((pLight->field_0 >> 8) & 0xFF) * 0.0039215689f * 8.0f;
    l.radiusSquared = l.radius * l.radius;
    l.radiusNormalized = 1.0f / l.radius;
    return static_cast<int>(idx * sizeof(SLightInternal));
}

void CC gbh_AssignPalette(STexture* pTexture, int palId)
{
    const bool needUnlock = !(pTexture->field_13_flags & 1);
    if (needUnlock)
    {
        gbh_LockTexture(pTexture);
    }
    pTexture->field_18_pPaltData = gPals[palId].mPData;
    pTexture->field_12_bPalIsValid = static_cast<u8>(gPals[palId].mbLoaded);
    if (needUnlock)
    {
        gbh_UnlockTexture(pTexture);
    }
}

void CC gbh_BeginLevel()
{
}

void CC gbh_SetBeginSceneCB(TBeginSceneCB)
{
    // The ImGui debug overlay needs an ImGui backend for OpenGL first; until then it is left out
}

void CC gbh_BeginScene()
{
}

int CC gbh_BlitBuffer(int, int, int, int, int, int)
{
    return 0;
}

int CC gbh_BlitImage(int imageIndex, int srcLeft, int srcTop, int srcRight, int srcBottom, int dstX, int dstY)
{
    if (imageIndex < 0 || static_cast<DWORD>(imageIndex) >= gImageTableCount || !gImageTable[imageIndex].bLoaded)
    {
        return -1;
    }
    const SImageTableEntry& img = gImageTable[imageIndex];
    if (srcLeft < 0 || srcTop < 0 || srcRight <= srcLeft || srcBottom <= srcTop || static_cast<DWORD>(srcRight) > img.field_4_w ||
        static_cast<DWORD>(srcBottom) > img.field_8_h)
    {
        return -2;
    }
    Vert verts[4];
    memset(verts, 0, sizeof(verts));
    const float x0 = static_cast<float>(dstX);
    const float y0 = static_cast<float>(dstY);
    const float x1 = static_cast<float>(dstX + (srcRight - srcLeft));
    const float y1 = static_cast<float>(dstY + (srcBottom - srcTop));
    const float u[4] = {static_cast<float>(srcLeft), static_cast<float>(srcRight), static_cast<float>(srcRight), static_cast<float>(srcLeft)};
    const float v[4] = {static_cast<float>(srcTop), static_cast<float>(srcTop), static_cast<float>(srcBottom), static_cast<float>(srcBottom)};
    const float x[4] = {x0, x1, x1, x0};
    const float y[4] = {y0, y0, y1, y1};
    for (int i = 0; i < 4; i++)
    {
        verts[i].x = x[i];
        verts[i].y = y[i];
        verts[i].z = 1.0f;
        verts[i].w = 1.0f;
        verts[i].u = u[i];
        verts[i].v = v[i];
    }
    // GTA2Hax passes 0x2DC81C3 here, whose low byte (0xC3) darkens images to 76%; a blit in the
    // original is a straight copy, so full brightness
    RenderVerts(0, img.field_C_pTexture, verts, 4, 0xFF);
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
    return RGB888ToRGB565((static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8) | b);
}

int CC gbh_DrawFlatRect(int, int)
{
    return 0;
}

void CC gbh_DrawQuad(int flags, STexture* pTexture, Vert* pVerts, int baseColour)
{
    RenderVerts(flags, pTexture, pVerts, 4, baseColour);
}

void CC gbh_DrawQuadClipped(int, int, int, int, int)
{
}

void CC gbh_DrawTriangle(int flags, STexture* pTexture, Vert* pVerts, int diffuseColour)
{
    RenderVerts(flags, pTexture, pVerts, 3, diffuseColour);
}

// GTA2Hax's gbh_DrawTilePart: a 64x64 tile, rotated (flags & 0x60) and flipped (0x08, 0x10) by its UVs
s32 CC gbh_DrawTilePart(unsigned int flags, STexture* pTexture, Vert* pData, int diffuseColour)
{
    unsigned int drawFlags = flags;
    if (!(flags & 0x4000))
    {
        // Half-texel insets keep neighbouring tiles from bleeding in
        const float lo = 0.5f;
        const float hi = 63.499901f;
        pData[0].u = lo;
        pData[0].v = lo;
        pData[1].u = hi;
        pData[1].v = lo;
        pData[2].u = hi;
        pData[2].v = hi;
        pData[3].u = lo;
        pData[3].v = hi;
    }

    struct UV
    {
        float u;
        float v;
    };
    UV uvs[4];
    for (int i = 0; i < 4; i++)
    {
        uvs[i].u = pData[i].u;
        uvs[i].v = pData[i].v;
    }
    auto setFrom = [&](const int order[4]) {
        for (int i = 0; i < 4; i++)
        {
            pData[i].u = uvs[order[i]].u;
            pData[i].v = uvs[order[i]].v;
        }
    };
    auto reload = [&]() {
        for (int i = 0; i < 4; i++)
        {
            uvs[i].u = pData[i].u;
            uvs[i].v = pData[i].v;
        }
    };

    static const int kRotate90[4] = {3, 0, 1, 2};
    static const int kRotate270[4] = {1, 2, 3, 0};
    static const int kFlipUpDown[4] = {1, 0, 3, 2};
    static const int kFlipLeftRight[4] = {3, 2, 1, 0};
    switch (flags & 0x60)
    {
        case 0x20:
            setFrom(kRotate90);
            reload();
            break;
        case 0x40:
            drawFlags = flags ^ 0x18; // 180 degrees: both flips
            break;
        case 0x60:
            setFrom(kRotate270);
            reload();
            break;
    }
    if (drawFlags & 8)
    {
        setFrom(kFlipUpDown);
        if (drawFlags & 0x10)
        {
            reload();
        }
    }
    if (drawFlags & 0x10)
    {
        setFrom(kFlipLeftRight);
    }
    gbh_DrawQuad(static_cast<int>(drawFlags), pTexture, pData, diffuseColour);
    return 0;
}

// d3ddll.dll exports the same function under both names
s32 CC gbh_DrawTile(unsigned int flags, STexture* pTexture, Vert* pData, int diffuseColour)
{
    return gbh_DrawTilePart(flags, pTexture, pData, diffuseColour);
}

void CC gbh_EndLevel()
{
}

void CC gbh_EndScene()
{
}

int CC gbh_FreeImageTable()
{
    for (DWORD i = 0; i < gImageTableCount; i++)
    {
        if (gImageTable[i].bLoaded)
        {
            gbh_FreeTexture(gImageTable[i].field_C_pTexture);
            gImageTable[i].field_C_pTexture = 0;
            gImageTable[i].bLoaded = FALSE;
        }
    }
    free(gImageTable);
    gImageTable = 0;
    gImageTableCount = 0;
    return 0;
}

void CC gbh_FreePalette(int palId)
{
    free(gPals[palId].mPData);
    gPals[palId].mPData = 0;
}

void CC gbh_FreeTexture(STexture* pTexture)
{
    if (!pTexture)
    {
        return;
    }
    gRenderer.FreeTextureDelayed(TextureId(pTexture));
    free(pTexture);
}

u32* CC gbh_GetGlobals()
{
    return reinterpret_cast<u32*>(&gGlobals);
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
    for (int i = 0; i < 12; i++)
    {
        gGlobals.mCacheUnknown[i] = kSGlobalsValues[i];
    }
    return 1;
}

signed int CC gbh_InitImageTable(int tableSize)
{
    gImageTable = static_cast<SImageTableEntry*>(calloc(tableSize, sizeof(SImageTableEntry)));
    if (!gImageTable)
    {
        return -1;
    }
    gImageTableCount = static_cast<DWORD>(tableSize);
    return 0;
}

// A 16-bit uncompressed TGA (SImage is its header)
signed int CC gbh_LoadImage(SImage* pToLoad)
{
    DWORD slot = 0;
    while (slot < gImageTableCount && gImageTable[slot].bLoaded)
    {
        slot++;
    }
    if (slot == gImageTableCount)
    {
        return -1;
    }
    if (!pToLoad || pToLoad->field_1 || pToLoad->field_2 != 2 || pToLoad->field_10 != 16)
    {
        return -2;
    }
    SImageTableEntry& img = gImageTable[slot];
    img.bLoaded = TRUE;
    img.field_4_w = pToLoad->field_C_width;
    img.field_8_h = pToLoad->field_E_height;
    s32 pitch = pToLoad->field_0; // the TGA's ID length: pixels start after it
    img.field_C_pTexture = InternalRegisterTexture(pToLoad->field_C_width, pToLoad->field_E_height,
                                                   reinterpret_cast<BYTE*>(&pToLoad->field_12), 0, &pitch);
    if (!img.field_C_pTexture)
    {
        img.bLoaded = FALSE;
        return -3;
    }
    return static_cast<signed int>(slot);
}

STexture* CC gbh_LockTexture(STexture* pTexture)
{
    pTexture->field_8_locked_pixels_ptr = pTexture->field_14_original_pixel_data_ptr;
    pTexture->field_13_flags |= 1;
    pTexture->field_6_pal_size = 256;
    return pTexture;
}

void CC gbh_Plot(int, int, int, int)
{
}

int CC gbh_PrintBitmap(int, int)
{
    return 0;
}

// Palettes are stored in columns: entry i is 64 DWORDs after entry i - 1
unsigned int CC gbh_RegisterPalette(int palId, DWORD* pData)
{
    DWORD* pOriginal = pData;
    for (int i = 0; i < 256; i++, pData += 64)
    {
        if (i == 0)
        {
            *pData = 0; // colour 0 of each palette is transparent
        }
        else if (*pData == 0)
        {
            *pData = 0x10000; // a black that isn't the transparent colour
        }
    }
    WORD* pConverted = static_cast<WORD*>(malloc(256 * 2 * sizeof(u16)));
    gPals[palId].mPOriginalData = pOriginal;
    gPals[palId].mbLoaded = 1;
    gPals[palId].mPData = pConverted;
    pData = pOriginal;
    for (int i = 0; i < 256; i++, pData += 64)
    {
        pConverted[i] = RGB888ToRGB565(*pData);
        pConverted[256 + i] = pConverted[i];
    }
    return static_cast<unsigned int>(palId);
}

STexture* CC gbh_RegisterTexture(s32 width, s32 height, BYTE* pData, s32 palIdx, char_type)
{
    return InternalRegisterTexture(width, height, pData, &palIdx, 0);
}

void CC gbh_ResetLights()
{
    gNumLights = 0;
}

void CC gbh_SetAmbient(float ambient)
{
    gAmbient = ambient * 255.0f;
}

int CC gbh_SetCamera(float, float, float, float)
{
    return 0;
}

int CC gbh_SetColourDepth()
{
    return 1;
}

void CC gbh_SetWindow(float left, float top, float right, float bottom)
{
    gWindowLeft = left;
    gWindowRight = right;
    gWindowTop = top;
    gWindowBottom = bottom;
}

STexture* CC gbh_UnlockTexture(STexture* pTexture)
{
    pTexture->field_6_pal_size = 0;
    pTexture->field_8_locked_pixels_ptr = 0;
    pTexture->field_13_flags &= 0xFE;
    SetTextureId(pTexture, SetTexture(TextureId(pTexture), pTexture->field_E_width, pTexture->field_10_height,
                                      pTexture->field_14_original_pixel_data_ptr, pTexture->field_18_pPaltData, 0));
    return pTexture;
}

// ================================================================ lookup by exported name
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
    ENTRY(SetShadeTableA),      {"MakeScreenTable", reinterpret_cast<FARPROC>(&MakeScreenTablePtr)},   ENTRY(gbh_AddLight),        ENTRY(gbh_AssignPalette),
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

// The headless self-test (port_selftest.cpp) checks pixels through this
void port_renderer_read_pixels(std::vector<DWORD>& out, int& w, int& h)
{
    gRenderer.ReadPixels(out, w, h);
}
