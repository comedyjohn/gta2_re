#include "CarInfo_808.hpp"
#include "core/enums.hpp"
#include "error.hpp"
#include "file.hpp"
#include "gtx_0x106C.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

// Forward declarations: the functions below are in address order
Fix16 ComputeCarMassAndInertia(Fix16 width, Fix16 height, Fix16 mass, Fix16 front_mass_bias, Fix16* out_cg_height);
Fix16 ComputeThrustWithTurbo(Fix16 half_thrust, Fix16 thrust_div5, s8 turbo);

EXTERN_GLOBAL(Fix16, kOneFixed);
EXTERN_GLOBAL(Fix16, kHalfFixed);
EXTERN_GLOBAL(Fix16, kFiveFixed);

DEFINE_GLOBAL(CarInfoTable*, gCarInfo808, 0x678098);
DEFINE_GLOBAL_ARRAY(char, gFileName, 64, 0x677EC4);
DEFINE_GLOBAL(u32, gProcessedGciData, 0x676260);
DEFINE_GLOBAL(u32, gGciDataLen, 0x675F94);
DEFINE_GLOBAL(u8*, gInputData, 0x676170);
DEFINE_GLOBAL(u32, gFoundOpenBrackets, 0x67626C);
DEFINE_GLOBAL(s32, gLineNumber, 0x676258);
DEFINE_GLOBAL(u32, gProcessedOutput, 0x676250);
DEFINE_GLOBAL(u32, gModelPhyArrLen, 0x675F90);
DEFINE_GLOBAL(u8*, gModelPhyArrPtr, 0x675F98);
DEFINE_GLOBAL_INIT(Fix16, kOne, Fix16(0x4000, 0), 0x677F54);
DEFINE_GLOBAL_INIT(Fix16, kQuarter, Fix16(0x1000, 0), 0x677D78);
DEFINE_GLOBAL_INIT(Fix16, kOneTenth, Fix16(0x666, 0), 0x677D74);
DEFINE_GLOBAL_ARRAY(char, gBuffer, 80, 0x675FD4);
DEFINE_GLOBAL_ARRAY(char, gTokenBuffer, 256, 0x676024);
DEFINE_GLOBAL_ARRAY_INIT(const char*,
                         gErrorTable,
                         13,
                         0x61A6D4,
                         "Invalid integer",
                         "Number too big",
                         "Write error",
                         "EOF in comment",
                         "Token too big",
                         "Input file not found",
                         "Can't open output file",
                         "Invalid long integer",
                         "Invalid hex integer",
                         "Invalid hex long integer",
                         "Invalid f32",
                         "Output too big",
                         "Unknown error");

// 0x430a30
char* CarInfoTable::ParseGciFile(void* gci_data,
                                 size_t gci_data_len,
                                 ModelPhysics* model_phy_arr,
                                 size_t model_phy_arr_len,
                                 u32* next_position)
{
    gInputData = static_cast<u8*>(gci_data);
    gLineNumber = 1;
    gProcessedGciData = 0;
    gGciDataLen = gci_data_len;
    gModelPhyArrPtr = reinterpret_cast<u8*>(model_phy_arr);
    gProcessedOutput = 0;
    gModelPhyArrLen = model_phy_arr_len;

    s32 parse_result;
    s32 token_result = 0;
    while (token_result == 0)
    {
        token_result = ReadToken(gTokenBuffer);
        if (token_result < 0)
        {
            return SetErr(token_result);
        }

        parse_result = ParseTokenAndPush(gTokenBuffer);
        if (parse_result < 0)
        {
            return SetErr(parse_result);
        }
    }

    *next_position = gProcessedOutput;
    return nullptr;
}

// 0x430AC0
char* CarInfoTable::SetErr(s32 error_code)
{
    if (error_code > 0 || error_code < -GTA2_COUNTOF_S(gErrorTable) - 1)
    {
        error_code = -GTA2_COUNTOF_S(gErrorTable);
    }

    sprintf(gBuffer, "%s at line %d", gErrorTable[-error_code], gLineNumber);
    return gBuffer;
}

// 0x430b10
s32 CarInfoTable::ReadToken(char* out)
{
    s32 i = 0;

    *out = 0;
    while (255 > i)
    {
        s32 current_char = 0;

        // Get the next character
        if (gProcessedGciData == gGciDataLen)
        {
            current_char = -1;
        }
        else
        {
            current_char = *gInputData;
            gInputData++;
            gProcessedGciData++;
            if (current_char == '\n')
            {
                gLineNumber++;
            }
        }

        switch (gFoundOpenBrackets)
        {
            case 0:
                if (current_char == -1)
                {
                    return 1;
                }
                if (current_char == '{')
                {
                    gFoundOpenBrackets = 1;
                }
                else if (current_char != ' ' && current_char != '\t' && current_char != '\n' && current_char != ')' &&
                         current_char != '(' && current_char != '\r')
                {
                    gFoundOpenBrackets = 2;
                    out[i] = static_cast<char>(current_char);
                    i++;
                }
                break;
            case 1:
                if (current_char == -1)
                {
                    return -4;
                }
                if (current_char == '}')
                {
                    gFoundOpenBrackets = 0;
                }
                break;
            case 2:
                if (current_char == -1)
                {
                    out[i] = 0;
                    return 1;
                }
                if (current_char == '{')
                {
                    out[i] = 0;
                    gFoundOpenBrackets = 1;
                    return 0;
                }
                if (current_char == ' ' || current_char == '\t' || current_char == '\n' || current_char == ')' || current_char == '(' ||
                    current_char == '\r')
                {
                    out[i] = 0;
                    gFoundOpenBrackets = 0;
                    return 0;
                }
                out[i] = static_cast<char>(current_char);
                i++;

                break;
            default:
                break;
        }
    }
    return -5;
}

// 0x430C70
s32 ParseTokenAndPush(char_type* str)
{
    s32 type;
    s32 error_ret;
    s16 negated;
    s32 parsed_value;
    Fix16 fix16_num;
    s32 negated_value;
    s16 tmp;

    char_type* curr = str;
    BOOL hex = false;
    BOOL negate = false;

    if (*curr)
    {
        if (*curr == '-')
        {
            negate = 1;
            curr++;
        }

        if (*curr)
        {
            if (*curr == 'b')
            {
                type = DataType::kByte;
                ++curr;
            }
            else if (*curr == 'w')
            {
                type = DataType::kWord;
                ++curr;
            }
            else if (*curr == 'l')
            {
                type = DataType::kDword;
                ++curr;
            }
            else if (*curr == 'f')
            {
                type = DataType::kFix16Float;
                ++curr;
            }
            else
            {
                type = DataType::kByte;
            }

            if (*curr == 'h')
            {
                hex = 1;
                ++curr;
            }
            if (*curr == '-')
            {
                negate = !negate;
                ++curr;
            }

            switch (type)
            {
                case DataType::kByte:
                    if (hex == 1)
                    {
                        error_ret = CarInfoTable::HexStr2Int(curr, &tmp);
                    }
                    else
                    {
                        error_ret = CarInfoTable::StrToInt(curr, &tmp);
                    }

                    if (!error_ret)
                    {
                        if (negate)
                        {
                            negated = static_cast<char>(-static_cast<char>(tmp));
                            if (negated < -127)
                            {
                                return -2;
                            }
                            error_ret = CarInfoTable::PushData(&negated, sizeof(BYTE));
                        }
                        else
                        {
                            if (static_cast<u16>(tmp) > 0xFFU)
                            {
                                return -2;
                            }
                            error_ret = CarInfoTable::PushData(&tmp, sizeof(BYTE));
                        }
                        // Shares the word case's tail; VC6 doesn't merge identical case tails by itself
                        goto check_push_result;
                    }
                    return error_ret;

                case DataType::kWord:
                    if (hex == 1)
                    {
                        error_ret = CarInfoTable::HexStr2Int(curr, &tmp);
                    }
                    else
                    {
                        error_ret = CarInfoTable::StrToInt(curr, &tmp);
                    }

                    if (!error_ret)
                    {
                        if (negate)
                        {
                            negated = -tmp;
                            error_ret = CarInfoTable::PushData(&negated, sizeof(WORD));
                        }
                        else
                        {
                            error_ret = CarInfoTable::PushData(&tmp, sizeof(WORD));
                        }

                    check_push_result:
                        if (error_ret >= 0)
                        {
                            return 0;
                        }
                    }
                    return error_ret;

                case DataType::kDword:
                    if (hex == 1)
                    {
                        error_ret = CarInfoTable::HexStr2Int(curr, &parsed_value);
                    }
                    else
                    {
                        error_ret = CarInfoTable::StrToInt(curr, &parsed_value);
                    }

                    if (!error_ret)
                    {
                        if (negate)
                        {
                            negated_value = -static_cast<s16>(parsed_value);
                            error_ret = CarInfoTable::PushData(&negated_value, sizeof(DWORD));
                        }
                        else
                        {
                            error_ret = CarInfoTable::PushData(&parsed_value, sizeof(DWORD));
                        }

                        if (error_ret >= 0)
                        {
                            return 0;
                        }
                    }
                    return error_ret;

                case DataType::kFix16Float:
                    error_ret = CarInfoTable::FloatStrToFix16(static_cast<char*>(curr), fix16_num);

                    if (!error_ret)
                    {
                        if (negate)
                        {
                            fix16_num = -fix16_num;
                        }

                        error_ret = CarInfoTable::PushData(&fix16_num, sizeof(Fix16));
                        if (error_ret >= 0)
                        {
                            return 0;
                        }
                    }
                    return error_ret;

                default:
                    return 0;
            }
        }
    }
    return 0;
}

// 0x430e60
s32 CarInfoTable::PushData(void* src, u32 size)
{
    gProcessedOutput += size;
    if (gProcessedOutput > gModelPhyArrLen)
    {
        return -12;
    }

    memcpy(gModelPhyArrPtr, src, size);

    gModelPhyArrPtr = gModelPhyArrPtr + size;
    return 0;
}

// 0x430EC0
s32 CarInfoTable::HexStr2Int(const char* str, s32* out)
{
    *out = 0;
    s32 place_value = 1;
    s32 str_len = strlen(str);

    while (--str_len >= 0)
    {
        s8 current_char = str[str_len];
        if ((current_char < '0' || '9' < current_char) && (current_char < 'A' || 'F' < current_char))
        {
            return 0xfffffff6;
        }

        s32 digit_value;
        if (current_char < '0' || '9' < current_char)
        {
            digit_value = (current_char - 0x37) * place_value;
        }
        else
        {
            digit_value = (current_char - 0x30) * place_value;
        }

        *out += digit_value;
        place_value = place_value << 4;
    }

    return 0;
}

// 0x430f30
s32 CarInfoTable::HexStr2Int(const char* str, s16* out)
{
    *out = 0;
    s32 place_value = 1;
    s32 str_len = strlen(str);

    while (--str_len >= 0)
    {
        s8 current_char = str[str_len];
        if ((current_char < '0' || '9' < current_char) && (current_char < 'A' || 'F' < current_char))
        {
            return 0xfffffff7;
        }

        s32 digit_value;
        if (current_char < '0' || '9' < current_char)
        {
            digit_value = (current_char - 0x37) * place_value;
        }
        else
        {
            digit_value = (current_char - 0x30) * place_value;
        }

        *out += digit_value;
        place_value = place_value << 4;
    }

    return 0;
}

// 0x430fa0
s32 CarInfoTable::StrToInt(const char* str, s32* out)
{
    *out = 0;
    s32 place_value = 1;
    s32 str_len = strlen(str);

    while (--str_len >= 0)
    {
        s8 current_char = str[str_len];
        if (current_char < '0' || '9' < current_char)
        {
            return 0xfffffff8;
        }

        s32 digit_value = (current_char - 0x30) * place_value;
        *out += digit_value;
        place_value = place_value * 10;
    }

    return 0;
}

// 0x431000
s32 CarInfoTable::FloatStrToFix16(char* str, Fix16& out)
{
    out = kFixZero;
    bool past_decimal_point = false;
    s32 decimal_divisor = 1;
    s32 str_len = strlen(str);

    while (--str_len >= 0)
    {
        char current_char = str[str_len];
        if (current_char < '0' || '9' < current_char)
        {
            if (current_char != '.' || past_decimal_point)
            {
                return -11;
            }
            out.value_ /= decimal_divisor;
            decimal_divisor = 1;
            past_decimal_point = true;
        }
        else
        {
            out.value_ += (current_char - 0x30) * decimal_divisor * 0x4000;
            decimal_divisor = decimal_divisor * 10;
        }
    }
    return 0;
}

// 0x431080
s32 CarInfoTable::StrToInt(const char* str, s16* out)
{
    *out = 0;
    s32 place_value = 1;
    s32 str_len = strlen(str);

    while (--str_len >= 0)
    {
        s8 current_char = str[str_len];
        if (current_char < '0' || '9' < current_char)
        {
            return 0xffffffff;
        }

        s32 digit_value = (current_char - 0x30) * place_value;
        *out += digit_value;
        place_value = place_value * 10;
    }

    return 0;
}

// 0x4542A0
void CarModelInfo::CalculateCarInfo(s32 idx)
{
    StyleCarInfo* car_info = gGtx->GetCarInfo(idx);
    ModelPhysics* model_physics = gCarInfo808->GetModelPhysicsFromIdx(idx);
    Fix16 gear2_speed = model_physics->gear2_speed_;
    Fix16 gear3_speed = model_physics->gear3_speed_;
    Fix16 max_speed;
    if (gear2_speed > gear3_speed || gear3_speed > (max_speed = model_physics->max_speed_) || gear2_speed > max_speed)
    {
        FatalError_4A38C0(0x3F6, "C:\\Splitting\\Gta2\\Source\\carinfo.cpp", 91, static_cast<u8>(idx));
    }

    s8 front_wheel_offset = car_info->front_wheel_offset_;
    Fix16 new_front_wheel_offset;
    if (front_wheel_offset < 0)
    {
        new_front_wheel_offset = -gPixelsToFix16.list_[-front_wheel_offset];
    }
    else
    {
        new_front_wheel_offset = gPixelsToFix16.list_[front_wheel_offset];
    }
    this->front_wheel_offset_ = new_front_wheel_offset;
    s8 rear_wheel_offset = car_info->rear_wheel_offset_;
    Fix16 new_rear_wheel_offset;
    if (rear_wheel_offset < 0)
    {
        new_rear_wheel_offset = -gPixelsToFix16.list_[-rear_wheel_offset];
    }
    else
    {
        new_rear_wheel_offset = gPixelsToFix16.list_[rear_wheel_offset];
    }
    this->rear_wheel_offset_ = new_rear_wheel_offset;
    Fix16 out_y;
    this->moment_of_inertia_ = ComputeCarMassAndInertia(gPixelsToFix16.list_[car_info->w],
                                                        gPixelsToFix16.list_[car_info->h_],
                                                        model_physics->mass_,
                                                        model_physics->front_mass_bias_,
                                                        &out_y);
    this->center_of_mass_offset_.x = 0;
    this->center_of_mass_offset_.y = out_y; // out val of func above
    this->half_thrust_ = model_physics->thrust_ / 2;
    this->fith_thrust_ = model_physics->thrust_ / 5;
    this->max_thrust_with_turbo_ = ComputeThrustWithTurbo(half_thrust_, fith_thrust_, model_physics->turbo_);
    this->front_drive_bias_ = kOne - model_physics->front_drive_bias_;
    this->skid_threshhold_1_ = (model_physics->sked_threshold_ * (kOne - kOneTenth));
    this->skid_threshhold_2_ = (model_physics->sked_threshold_ * (kOne + kOneTenth));
}

// 0x454410
Fix16 ComputeCarMassAndInertia(Fix16 width, Fix16 height, Fix16 mass, Fix16 front_mass_bias, Fix16* out_cg_height)
{
    // The const refs (bound to the product temporaries) give the original's stack slots and the
    // __allmul operand order of the two inertia products (value locals swap them)
    const Fix16& inertia_base = ((((height * height) * kQuarter) + (width * width)) / 12);
    const Fix16& height_x_constant = (height * kQuarter);
    const Fix16& front_mass = (mass * front_mass_bias);
    Fix16 front_i = (front_mass * inertia_base);
    // Negated by hand: one inline expansion less, so both front (cg - h) subtractions stay inline (budget)
    Fix16 nh;
    nh.value_ = -height.value_;
    Fix16 neg_height_x_constant = (nh * kQuarter);

    const Fix16& rear_mass = (mass * (kOne - front_mass_bias));
    Fix16 rear_i = (rear_mass * inertia_base);

    *out_cg_height = (((height_x_constant * front_mass) + (neg_height_x_constant * rear_mass)) / mass);

    Fix16 front_total;
    front_total = front_i + front_mass * (*out_cg_height - height_x_constant) * (*out_cg_height - height_x_constant);
    Fix16 rear_total;
    rear_total = rear_i + rear_mass * (*out_cg_height - neg_height_x_constant) * (*out_cg_height - neg_height_x_constant);
    return front_total + rear_total;
}

// 0x454680
void ModelPhysics::ConvertMass()
{
    mass_ = mass_ * (kOneTenth + kOne);
}

// 0x4546b0
ModelPhysics* CarInfoTable::GetModelPhysicsFromIdx(u8 model_idx)
{
    return model_physics_array_[model_idx];
}

// 0x4546d0
void CarInfoTable::LoadModelPhysics()
{
    u32 number_of_cars = gGtx->GetNumberOfCars();
    u32 data_size;
    size_t file_size;
    void* file_content = File::ReadFileToBuffer(gFileName, &file_size);

    raw_model_physics_ = new ModelPhysics[number_of_cars];

    char* error_msg =
        CarInfoTable::ParseGciFile(file_content, file_size, raw_model_physics_, number_of_cars * sizeof(ModelPhysics), &data_size);

    ::free(file_content);

    if (error_msg != nullptr)
    {
        strcpy(gErrStr_67C29C, gFileName);
        strcpy(gErrStr2_67C3A8, error_msg);

        FatalError_4A38C0(Gta2Error::kBinmakeError, "C:\\Splitting\\Gta2\\Source\\carinfo.cpp", 0xbc, gErrStr_67C29C, gErrStr2_67C3A8);
    }

    if (data_size % sizeof(ModelPhysics))
    {
        FatalError_4A38C0(Gta2Error::kInvalidSize, "C:\\Splitting\\Gta2\\Source\\carinfo.cpp", 0xbd, gFileName, data_size);
    }

    if (data_size != number_of_cars * sizeof(ModelPhysics))
    {
        FatalError_4A38C0(Gta2Error::kModelCountMismatchInStyleFile,
                          "C:\\Splitting\\Gta2\\Source\\carinfo.cpp",
                          0xbe,
                          static_cast<s32>(data_size / sizeof(ModelPhysics)),
                          gFileName,
                          number_of_cars);
    }

    for (u32 i = 0; i < number_of_cars; i++)
    {
        ModelPhysics* m_var5 = &raw_model_physics_[i];
        model_physics_array_[m_var5->model_] = m_var5;
    }
}

// 0x454840
CarModelInfo* CarInfoTable::GetInfoAtIdx(u8 idx)
{
    return ptr_array_[idx];
}

// 0x454850
void CarInfoTable::CalculateAllCarInfo()
{
    const u32 count = gGtx->GetNumberOfCars();
    raw_car_info_ = new CarModelInfo[count];

    for (u32 i = 0, j = 0; i < 256; i++)
    {
        if (gGtx->DoesCarExist(i))
        {
            ptr_array_[i] = &raw_car_info_[j];
            raw_car_info_[j].CalculateCarInfo(i);
            j++;
        }
    }
}

// 0x4549A0
CarModelInfo::CarModelInfo() = default;

// 0x4549c0
void CarInfoTable::ConvertAllMass()
{
    u32 number_of_cars = gGtx->GetNumberOfCars();
    for (u32 i = 0; i < number_of_cars; i++)
    {
        raw_model_physics_[i].ConvertMass();
    }
}

// 0x454a00
void CarInfoTable::LoadFromGciFile(const char_type* gci_file_path)
{
    strcpy(&gFileName[0], gci_file_path);

    LoadModelPhysics();
    ConvertAllMass();
    CalculateAllCarInfo();
}

// 0x454a50
void CarInfoTable::Reload()
{
    Free();
    Clear();
    LoadModelPhysics();
    ConvertAllMass();
    CalculateAllCarInfo();
}

// 0x454a80
void CarInfoTable::Clear()
{
    for (int i = 0; i < 0x100; i++)
    {
        model_physics_array_[i] = nullptr;
        ptr_array_[i] = nullptr;
    }
}

// 0x454aa0
void CarInfoTable::Free()
{
    delete[] raw_car_info_;
    raw_car_info_ = nullptr;

    delete[] raw_model_physics_;

    raw_model_physics_ = nullptr;
}

// 0x454b00
CarInfoTable::CarInfoTable()
{
    raw_car_info_ = nullptr;
    raw_model_physics_ = nullptr;
    Clear();
}

// 0x454b20
CarInfoTable::~CarInfoTable()
{
    Free();
}

// 0x5618F0
Fix16 ComputeThrustWithTurbo(Fix16 half_thrust, Fix16 thrust_div5, s8 turbo)
{
    Fix16 thrust;
    if (turbo)
    {
        thrust = (kOneFixed + kHalfFixed) * ((thrust_div5 * kFiveFixed));
    }
    else
    {
        thrust = thrust_div5 * kFiveFixed;
    }
    thrust = thrust + half_thrust;
    return thrust;
}