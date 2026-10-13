// MSVC C runtime names that glibc spells differently or lacks.
#pragma once

#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _stricmp strcasecmp
#define stricmp strcasecmp
#define _strcmpi strcasecmp
#define strcmpi strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#ifdef __cplusplus
// MSVC's struct _stat; _stat() resolves the path as fopen does
struct _stat : public stat
{
};
extern "C" int port_stat(const char* path, struct _stat* buf);
    #define _stat(path, buf) port_stat(path, buf)
#endif

#ifdef __cplusplus
extern "C" {
#endif
char* port_strupr(char* s);
char* port_strlwr(char* s);
char* port_itoa(int value, char* buf, int radix);
#ifdef __cplusplus
}
#endif

#define _strupr port_strupr
#define strupr port_strupr
#define _strlwr port_strlwr
#define strlwr port_strlwr
#define _itoa port_itoa
#define itoa port_itoa

// File names: the game spells them the Windows way ("data\\Audio\\wil.sdt", any case). These resolve
// separators and match each path component case-insensitively (port/platform/crt.cpp).
#ifdef __cplusplus
extern "C" {
#endif
FILE* port_fopen(const char* path, const char* mode);
FILE* port_freopen(const char* path, const char* mode, FILE* stream);
#ifdef __cplusplus
}
#endif
#define fopen(path, mode) port_fopen(path, mode)
#define freopen(path, mode, stream) port_freopen(path, mode, stream)
