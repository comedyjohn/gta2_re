// The subset of the Win32 API that GTA2 uses, for the native build. Types match their
// Windows sizes under -m32 (long is 32-bit there too). Functions are implemented in port/platform/.
#pragma once

#include <stdint.h>

#define WINAPI
#define APIENTRY
#define CALLBACK
#define PASCAL
#define WINAPIV
#define CONST const
#define VOID void
#define FAR
#define NEAR
#define far
#define near
#define IN
#define OUT
#define OPTIONAL

// Microsoft source annotations (SAL), used by the DirectX headers
#define _Return_type_success_(expr)
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_
#define _In_z_
#define _In_reads_(x)
#define _In_reads_bytes_(x)
#define _Out_writes_(x)
#define _Out_writes_bytes_(x)
#define _Outptr_
#define _Outptr_opt_
#define _COM_Outptr_
#define _Reserved_
#define _Check_return_
#define _Success_(x)

#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef NULL
    #define NULL 0
#endif
#define MAX_PATH 260

typedef int BOOL;
typedef BOOL* LPBOOL;
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef BYTE* PBYTE;
typedef unsigned char UCHAR;
typedef unsigned short WORD;
typedef WORD* LPWORD;
typedef unsigned short USHORT;
typedef short SHORT;
// Windows keeps long at 32 bits even in 64-bit builds (LLP64); Linux doesn't (LP64). The 32-bit
// build keeps MSVC's exact types, which the game's code was written against.
#if __SIZEOF_LONG__ == 4
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef long HRESULT;
#else
typedef unsigned int DWORD;
typedef int LONG;
typedef unsigned int ULONG;
typedef int HRESULT;
#endif
typedef DWORD* LPDWORD;
typedef DWORD* PDWORD;
typedef LONG* LPLONG;
typedef LONG* PLONG;
typedef ULONG* PULONG;
typedef int INT;
typedef unsigned int UINT;
typedef UINT* PUINT;
typedef float FLOAT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef CHAR TCHAR;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef unsigned long long DWORDLONG;
typedef void* LPVOID;
typedef void* PVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef char* PSTR;
typedef const char* LPCSTR;
typedef const char* PCSTR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t INT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef ULONG_PTR SIZE_T;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef WORD ATOM;
typedef DWORD COLORREF;

#define DECLARE_HANDLE(name) \
    struct name##__          \
    {                        \
        int unused;          \
    };                       \
    typedef struct name##__* name
typedef void* HANDLE;
typedef HANDLE* LPHANDLE;
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HKEY);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HGDIOBJ_);
DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HGLOBAL_);
typedef HICON HCURSOR;
typedef HINSTANCE HMODULE;
typedef void* HGDIOBJ;
typedef HANDLE HGLOBAL;
typedef HKEY* PHKEY;
// void (*)() rather than Windows' int (*)(): the generic function pointer type, so casting it to the
// real signature is not a cast between incompatible function types (GCC -Wcast-function-type)
typedef void (*FARPROC)();

typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;
typedef const RECT* LPCRECT;

typedef struct tagPOINT
{
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagSIZE
{
    LONG cx;
    LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct _GUID
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;
typedef GUID IID;
typedef GUID CLSID;
typedef GUID* LPGUID;
typedef const GUID* LPCGUID;
#ifdef __cplusplus
inline bool operator==(const GUID& a, const GUID& b)
{
    return __builtin_memcmp(&a, &b, sizeof(GUID)) == 0;
}
inline bool operator!=(const GUID& a, const GUID& b)
{
    return !(a == b);
}
    #define REFGUID const GUID&
    #define REFIID const IID&
    #define REFCLSID const IID&
#else
    #define REFGUID const GUID*
    #define REFIID const IID*
    #define REFCLSID const IID*
#endif

typedef union _LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    };
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define MAKEINTRESOURCE(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEA MAKEINTRESOURCE

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define MAKE_HRESULT(sev, fac, code) ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))

#include "port_win32_api.h"

// The real windows.h pulls these in too, and game code relies on it (timeGetTime)
#include <mmsystem.h>
