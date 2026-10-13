// UTF-16 string functions with MSVC semantics. With -fshort-wchar, glibc's own wcs* functions
// (which expect 4-byte wchar_t) must never be called, so every one the game uses is redirected here.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef __builtin_va_list port_va_list;

size_t port_wcslen(const wchar_t* s);
wchar_t* port_wcscpy(wchar_t* dst, const wchar_t* src);
wchar_t* port_wcsncpy(wchar_t* dst, const wchar_t* src, size_t n);
int port_wcscmp(const wchar_t* a, const wchar_t* b);
size_t port_wcstombs(char* dst, const wchar_t* src, size_t n);
size_t port_mbstowcs(wchar_t* dst, const char* src, size_t n);
wchar_t* port_itow(int value, wchar_t* buf, int radix);
// MSVC's non-conforming swprintf: no buffer size, and %s / %c take wide arguments.
int port_swprintf(wchar_t* buf, const wchar_t* fmt, ...);
int port_vswprintf(wchar_t* buf, const wchar_t* fmt, port_va_list args);

#ifdef __cplusplus
}
#endif

#define wcslen port_wcslen
#define wcscpy port_wcscpy
#define wcsncpy port_wcsncpy
#define wcscmp port_wcscmp
#define wcstombs port_wcstombs
#define mbstowcs port_mbstowcs
#define _itow port_itow
#define swprintf port_swprintf
#define vswprintf port_vswprintf
