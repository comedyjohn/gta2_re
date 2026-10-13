// MSVC <direct.h>
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int port_chdir(const char* path);
char* port_getcwd(char* buf, int size);
int port_mkdir(const char* path);
#ifdef __cplusplus
}
#endif
#define _chdir port_chdir
#define _getcwd port_getcwd
#define _mkdir port_mkdir
