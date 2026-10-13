// <tchar.h> for an ANSI (non-UNICODE) build
#pragma once
#define _T(x) x
#define _TEXT(x) x
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcscmp strcmp
#define _stprintf sprintf
typedef char _TCHAR;
