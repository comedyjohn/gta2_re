#include "Object_8.hpp"

// 0x52ad50
Object_8::Object_8()
{
    this->next_ = nullptr;
    this->field_7_anim_speed_counter = 99;
    this->field_6_frame_counter = 99;
    this->field_4_timer = 99;
}

// 0x52ad70
Object_8::~Object_8()
{
    next_ = nullptr;
}