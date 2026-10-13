// The built-in renderer standing in for d3ddll.dll and dmavideo.dll (see renderer.cpp)
#pragma once
#include <windows.h>

FARPROC port_renderer_lookup(const char* name);
bool port_is_renderer_library(const char* fileName);
