// The built-in renderer standing in for d3ddll.dll and dmavideo.dll (see renderer.cpp)
#pragma once
#include <windows.h>

#include <vector>

FARPROC port_renderer_lookup(const char* name);
bool port_is_renderer_library(const char* fileName);

// The game's picture as RGBA, bottom row first (for the headless self-test)
void port_renderer_read_pixels(std::vector<DWORD>& out, int& w, int& h);

// ---- for the 3D renderer (port/world3d, PLAN.md 3.4)
struct STexture;

// One light the game submitted this frame (gbh_AddLight)
struct PortLight // port: world3d
{
    DWORD flags;
    float brightness;
    float radius;
    float radiusSquared;
    float radiusNormalized;
    float x, y, z;
    float r, g, b;
};
// The frame's lights (valid until the next gbh_ResetLights / gbh_AddLight) and ambient (0..255)
int port_renderer_lights(const PortLight** out, float* ambient); // port: world3d
// The OpenGL texture of a game texture (0: none)
unsigned port_renderer_texture_id(const STexture* texture); // port: world3d
// After drawing with another program: makes the classic renderer's GL state current again
void port_renderer_restore_state(); // port: world3d
