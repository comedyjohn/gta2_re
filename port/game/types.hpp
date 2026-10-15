#pragma once

typedef unsigned char u8;
typedef signed char s8;
typedef char char_type;

typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned int u32;
typedef signed int s32;

// An integer that holds a pointer: s32 in the original (VC6, 32-bit), pointer-sized in the native
// 64-bit build (port/)
// uptr is the unsigned one, for the places the original kept a pointer in a u32.
#if defined(_MSC_VER)
typedef s32 sptr;
typedef u32 uptr;
#else
typedef __INTPTR_TYPE__ sptr;
typedef __UINTPTR_TYPE__ uptr;
#endif

typedef float f32;
typedef double f64;

// Maximum number of players in a (network) game
#define MAX_PLAYERS 6
