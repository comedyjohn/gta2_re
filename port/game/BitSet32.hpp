#pragma once

#include "Function.hpp"
#include "types.hpp"

EXTERN_GLOBAL(u32, kGlobalMask0_61A9AC);
EXTERN_GLOBAL(u32, kGlobalMask1_61A9A8);
EXTERN_GLOBAL(u32, kGlobalMask2_61A9A4);
EXTERN_GLOBAL(u32, gFlags_67ACF8);

// Dumb name to avoid conflict with BitField32
struct CompilerBitField32
{
    u32 b0 : 1;
    u32 b1 : 1;
    u32 b2 : 1;
    u32 b3 : 1;
    u32 b4 : 1;
    u32 b5 : 1;
    u32 b6 : 1;
    u32 b7 : 1;
    u32 b8 : 1;
    u32 b9 : 1;
    u32 b10 : 1;
    u32 b11 : 1;
    u32 b12 : 1;
    u32 b13 : 1;
    u32 b14 : 1;
    u32 b15 : 1;
    u32 b16 : 1;
    u32 b17 : 1;
    u32 b18 : 1;
    u32 b19 : 1;
    u32 b20 : 1;
    u32 b21 : 1;
    u32 b22 : 1;
    u32 b23 : 1;
    u32 b24 : 1;
    u32 b25 : 1;
    u32 b26 : 1;
    u32 b27 : 1;
    u32 b28 : 1;
    u32 b29 : 1;
    u32 b30 : 1;
    u32 b31 : 1;
};

// Byte-sized bit field: clearing a bit through it is a byte `and` whose mask VC6 does not share with
// the same mask on a 32 bit field (Ped::Reset_45AFC0)
struct CompilerBitField8
{
    u8 b0 : 1;
    u8 b1 : 1;
    u8 b2 : 1;
    u8 b3 : 1;
    u8 b4 : 1;
    u8 b5 : 1;
    u8 b6 : 1;
    u8 b7 : 1;
};

class BitSet32
{
  public:
    void ClearBit(int bit)
    {
        m_var &= ~(1 << bit);
    }

    void SetBit(int bit)
    {
        m_var |= (1 << bit);
    }

    // TODO: Find in 9.6f
    bool check_bit(s32 bit)
    {
        return (m_var & (1 << bit)) ? true : false;
    }

    // 9.6f 0x420E20 TODO: Check if matches
    bool mask_bit(u32 bit)
    {
        return (m_var & (1U << bit)) == (1U << bit);
    }

    BitSet32& operator=(u32& value) // Note: arg as reference
    {
        // TODO: Value is probably another BitSet32
        m_var = value;
        return *this;
    }

    u32 GetRaw()
    {
        return m_var;
    }

    void SetGlobal0()
    {
        gFlags_67ACF8 = kGlobalMask0_61A9AC;
    }

    void SetGlobal1()
    {
        gFlags_67ACF8 = kGlobalMask1_61A9A8;
    }

    void SetGlobal2()
    {
        gFlags_67ACF8 = kGlobalMask2_61A9A4;
    }

    void MaskWithGlobalFlags()
    {
        m_var &= gFlags_67ACF8;
    }

    void ClearGlobalFlags()
    {
        m_var &= ~gFlags_67ACF8;
    }

    void ClearAllBits()
    {
        m_var = 0;
    }

    BitSet32() : m_var(0)
    {
    }

    // TODO: Find in 9.6f
    BitSet32(s32 value) : m_var(value)
    {
    }

    u32 m_var;
};

struct DamageAreaFlags : BitSet32
{
    s16 Delta_48F8B0(u16& sprite_idx, u8& bRet, const u16& a4, const u32& a5);
    void Delta_48F820(u16& sprite_idx, u8* pArray, u32& a3, u8& width);
};
