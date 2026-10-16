#pragma once

#include "Fix16_Point.hpp"
#include "Function.hpp"
#include "fix16.hpp"

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
        Init_4F79F0();
    }

    // 10.5 0x4F79F0
    void Init_4F79F0()
    {
        for (s32 i = 0; i < 256; i++)
        {
            list[i] = Fix16(i) / 64;
        }
    }

    Fix16 SignedPixelsToFix16_41FE70(const s8& var)
    {
        if (var < 0)
        {
            return -list[-var];
        }
        return list[var];
    }

    // This one won't match with a ref argument even though its the
    // same as the above function :')
    Fix16 SignedPixelsToFix16_440860(s8 var);

    Fix16 list[256];
};

EXTERN_GLOBAL(PixelsToFix16Table, gPixelsToFix16_6F6850);

class ModelPhysics_48
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

    Fix16 get_anti_strngth()
    {
        return field_2C_anti_strngth;
    }

    void ConvertMass_454680();
};

class CarInfo_2C
{
  public:
    CarInfo_2C();
    ~CarInfo_2C() = default; // 0x4549B0
    void CalculateCarInfo_4542A0(s32 idx);

    Fix16 field_0_moment_of_inertia;
    Fix16 field_4_front_wheel_offset;
    Fix16 field_8_rear_wheel_offset;
    Fix16_Point field_C_center_of_mass_offset;
    Fix16 field_14_half_thrust;
    Fix16 field_18_fith_thrust;
    Fix16 field_1C_max_thrust_with_turbo;
    Fix16 field_20_front_drive_bias;
    Fix16 field_24_skid_threshhold_1;
    Fix16 field_28_skid_threshhold_2;
};

class CarInfo_808
{
  public:
    ModelPhysics_48* GetModelPhysicsFromIdx_4546B0(u8 model_idx);
    void LoadModelPhysics_4546D0();
    CarInfo_2C* GetInfoAtIdx_454840(u8 idx);
    void CalculateAllCarInfo_454850();
    void ConvertAllMass_4549C0();
    void LoadFromGciFile_454A00(const char_type* pGciFilePath);
    void Reload_454A50();
    void Clear_454A80();
    void Free_454AA0();
    CarInfo_808();
    ~CarInfo_808();

    static char* parse_gci_file_430A30(void* input, size_t input_size, ModelPhysics_48* output, size_t output_size, u32* next_position);
    static char* SetErr_430AC0(s32 error_code);
    static s32 ReadToken_430B10(char* pOut);
    static s32 PushData_430E60(void* pSrc, u32 size);
    static s32 HexStr2Int_430EC0(const char* pStr, s32* pOut);
    static s32 HexStr2Int_430F30(const char* pStr, s16* pOut);
    static s32 StrToInt_430FA0(const char* pStr, s32* pOut);
    static s32 FloatStrToFix16_431000(char* pStr, Fix16& pOut);
    static s32 StrToInt_431080(const char* pStr, s16* pOut);

    CarInfo_2C* field_0_ptr_array[256];
    CarInfo_2C* field_400_raw_data;
    ModelPhysics_48* field_404_model_physics_array[256];
    ModelPhysics_48* field_804_raw_data;
};

EXTERN_GLOBAL(CarInfo_808*, gCarInfo_808_678098);

s32 ParseTokenAndPush_430C70(char_type* a1);
