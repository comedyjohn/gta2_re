#pragma once

using u8 = unsigned char;
using s8 = signed char;
using char_type = char;

using u16 = unsigned short int;
using s16 = signed short int;

using u32 = unsigned int;
using s32 = signed int;

// An integer that holds a pointer: s32 in the original (VC6, 32-bit), pointer-sized in the native
// 64-bit build (port/)
// uptr is the unsigned one, for the places the original kept a pointer in a u32.
using sptr = __INTPTR_TYPE__;
using uptr = __UINTPTR_TYPE__;

using f32 = float;
using f64 = double;

// Maximum number of players in a (network) game
inline constexpr s32 MAX_PLAYERS = 6;
