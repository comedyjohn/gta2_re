#pragma once

#include "core/function.hpp"

class PatrolPoint_3
{
  public:
    // ctor 463F90
    // dtor 463FA0
    PatrolPoint_3();
    ~PatrolPoint_3() = default; // 0x463FA0

    u8 field_0_x;
    u8 field_1_y;
    u8 field_2_z;
};

// A patrol route of up to 50 waypoints
class PatrolRoute_96
{
  public:
    enum
    {
        k_max_points = 50
    };

    ~PatrolRoute_96() = default; // 0x4bdf90
    void sub_543EC0();
    PatrolRoute_96();
    PatrolPoint_3 field_0_points[k_max_points];
};

// Pool of 50 patrol routes handed out to peds by AllocPatrolList_543F10
class PatrolRoutePool_1D7E
{
  public:
    enum
    {
        k_max_routes = 50
    };

    ~PatrolRoutePool_1D7E() = default; // 0x4bdf70
    PatrolRoutePool_1D7E();
    PatrolRoute_96* AllocPatrolList_543F10(u8* pRet);
    PatrolRoute_96 field_0_routes[k_max_routes];
    u8 field_1D4C_bUsed[k_max_routes];
};

EXTERN_GLOBAL(PatrolRoutePool_1D7E*, gPatrolRoutePool_6FD784);
