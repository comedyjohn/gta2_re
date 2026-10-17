#pragma once

#include "core/fix16.hpp"
#include "core/fix16_point.hpp"
#include "core/function.hpp"

namespace DataType
{
enum
{
    kByte = 0,
    kWord = 1,
    kDword = 2,
    kFix16Float = 3,
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
            list_[i] = Fix16(i) / 64;
        }
    }

    Fix16 SignedPixelsToFix16(const s8& var)
    {
        if (var < 0)
        {
            return -list_[-var];
        }
        return list_[var];
    }

    Fix16 list_[256];
};

EXTERN_GLOBAL(PixelsToFix16Table, gPixelsToFix16_6F6850);

class ModelPhysics
{
  public:
    u8 model_;
    s8 turbo_;
    u8 value_;
    s8 padding_;
    Fix16 mass_;
    Fix16 front_drive_bias_;
    Fix16 front_mass_bias_;
    Fix16 brake_friction_;
    Fix16 turn_in_;
    Fix16 turn_ratio_;
    Fix16 rear_end_stability_;
    Fix16 handbrake_slide_value_;
    Fix16 thrust_;
    Fix16 max_speed_;
    Fix16 anti_strngth_;
    Fix16 sked_threshold_;
    Fix16 gear1_multiplier_;
    Fix16 gear2_multiplier_;
    Fix16 gear3_multiplier_;
    Fix16 gear2_speed_;
    Fix16 gear3_speed_;

    Fix16 GetAntiStrngth()
    {
        return anti_strngth_;
    }

    void ConvertMass();
};

class CarModelInfo
{
  public:
    CarModelInfo();
    ~CarModelInfo() = default; // 0x4549B0
    void CalculateCarInfo(s32 idx);

    Fix16 moment_of_inertia_;
    Fix16 front_wheel_offset_;
    Fix16 rear_wheel_offset_;
    Fix16Point center_of_mass_offset_;
    Fix16 half_thrust_;
    Fix16 fith_thrust_;
    Fix16 max_thrust_with_turbo_;
    Fix16 front_drive_bias_;
    Fix16 skid_threshhold_1_;
    Fix16 skid_threshhold_2_;
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

    CarModelInfo* ptr_array_[256];
    CarModelInfo* raw_car_info_;
    ModelPhysics* model_physics_array_[256];
    ModelPhysics* raw_model_physics_;
};

EXTERN_GLOBAL(CarInfoTable*, gCarInfo_808_678098);

s32 ParseTokenAndPush(char_type* a1);
