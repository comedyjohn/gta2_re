// MSVC <io.h>: the _findfirst family (wildcard directory listing), plus _access.
#pragma once
#include <time.h>

struct _finddata_t
{
    unsigned attrib;
    long time_create;
    long time_access;
    long time_write;
    unsigned long size;
    char name[260];
};

#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

#ifdef __cplusplus
extern "C" {
#endif
long port_findfirst(const char* pattern, struct _finddata_t* data);
int port_findnext(long handle, struct _finddata_t* data);
int port_findclose(long handle);
int port_access(const char* path, int mode);
#ifdef __cplusplus
}
#endif
#define _findfirst port_findfirst
#define _findnext port_findnext
#define _findclose port_findclose
#define _access port_access
