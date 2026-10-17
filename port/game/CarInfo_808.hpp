#pragma once

#include "core/fix16.hpp"
#include "core/fix16_point.hpp"
#include "core/function.hpp"

namespace DataType
{
enum
{
    byte = 0,
    word = 1,
    dword = 2,
    fix16_float = 3,
};
} // namespace DataType

struct PixelsToFix16Table
{

    PixelsToFix16Table()
    {
        Init();
    }

    // 10.5 0x4F79F0
    void Init()
    {
        for (s32 i = 0; i < 256; i++)
        {
            list[i] = Fix16(i) / 64;
        }
    }

    Fix16 SignedPixelsToFix16(const s8& var)
    {
        if (var < 0)
        {
            return -list[-var];
        }
        return list[var];
    }

    Fix16 list[256];
};

EXTERN_GLOBAL(PixelsToFix16Table, gPixelsToFix16_6F6850);

class ModelPhysics
{
  public:
    u8 field_0_model;
    s8 field_1_turbo;
    u8 field_2_value;
    s8 field_3_padding;
    Fix16 field_4_mass;
    Fix16 field_8_front_drive_bias;
    Fix16 field_C_front_mass_bias;
    Fix16 field_10_brake_friction;
    Fix16 field_14_turn_in;
    Fix16 field_18_turn_ratio;
    Fix16 field_1C_rear_end_stability;
    Fix16 field_20_handbrake_slide_value;
    Fix16 field_24_thrust;
    Fix16 field_28_max_speed;
    Fix16 field_2C_anti_strngth;
    Fix16 field_30_sked_threshold;
    Fix16 field_34_gear1_multiplier;
    Fix16 field_38_gear2_multiplier;
    Fix16 field_3C_gear3_multiplier;
    Fix16 field_40_gear2_speed;
    Fix16 field_44_gear3_speed;

    Fix16 GetAntiStrngth()
    {
        return field_2C_anti_strngth;
    }

    void ConvertMass();
};

class CarModelInfo
{
  public:
    CarModelInfo();
    ~CarModelInfo() = default; // 0x4549B0
    void CalculateCarInfo(s32 idx);

    Fix16 field_0_moment_of_inertia;
    Fix16 field_4_front_wheel_offset;
    Fix16 field_8_rear_wheel_offset;
    Fix16Point field_C_center_of_mass_offset;
    Fix16 field_14_half_thrust;
    Fix16 field_18_fith_thrust;
    Fix16 field_1C_max_thrust_with_turbo;
    Fix16 field_20_front_drive_bias;
    Fix16 field_24_skid_threshhold_1;
    Fix16 field_28_skid_threshhold_2;
};

class CarInfoTable
{
  public:
    ModelPhysics* GetModelPhysicsFromIdx(u8 model_idx);
    void LoadModelPhysics();
    CarModelInfo* GetInfoAtIdx(u8 idx);
    void CalculateAllCarInfo();
    void ConvertAllMass();
    void LoadFromGciFile(const char_type* pGciFilePath);
    void Reload();
    void Clear();
    void Free();
    CarInfoTable();
    ~CarInfoTable();

    static char* ParseGciFile(void* input, size_t input_size, ModelPhysics* output, size_t output_size, u32* next_position);
    static char* SetErr(s32 error_code);
    static s32 ReadToken(char* pOut);
    static s32 PushData(void* pSrc, u32 size);
    static s32 HexStr2Int(const char* pStr, s32* pOut);
    static s32 HexStr2Int(const char* pStr, s16* pOut);
    static s32 StrToInt(const char* pStr, s32* pOut);
    static s32 FloatStrToFix16(char* pStr, Fix16& pOut);
    static s32 StrToInt(const char* pStr, s16* pOut);

    CarModelInfo* field_0_ptr_array[256];
    CarModelInfo* field_400_raw_data;
    ModelPhysics* field_404_model_physics_array[256];
    ModelPhysics* field_804_raw_data;
};

EXTERN_GLOBAL(CarInfoTable*, gCarInfo_808_678098);

s32 ParseTokenAndPush(char_type* a1);
