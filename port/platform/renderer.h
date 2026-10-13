// The built-in renderer standing in for d3ddll.dll and dmavideo.dll (see renderer.cpp)
#pragma once
#include <windows.h>

#include <vector>

FARPROC port_renderer_lookup(const char* name);
bool port_is_renderer_library(const char* fileName);

// The game's picture as RGBA, bottom row first (for the headless self-test)
void port_renderer_read_pixels(std::vector<DWORD>& out, int& w, int& h);
