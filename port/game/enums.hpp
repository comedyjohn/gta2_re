#pragma once

// TODO: Replace CarDamageAreasFlags etc with this
namespace CarDeltaBitsEnum
{
enum
{
    TopLeftDamage_0 = 0, // (1 << 0)  = 0x00000001
    TopRightDamage_1 = 1, // (1 << 1)  = 0x00000002
    BottomRightDamage_2 = 2, // (1 << 2)  = 0x00000004
    BottomLeftDamage_3 = 3, // (1 << 3)  = 0x00000008
    WindshieldDamage_4 = 4, // (1 << 4)  = 0x00000010

    BackRightBrakeLight_5 = 5, // (1 << 5)  = 0x00000020
    FrontRightHeadlight_6 = 6, // (1 << 6)  = 0x00000040

    BottomRightDoor1_7 = 7, // (1 << 7)  = 0x00000080
    BottomRightDoor2_8 = 8, // (1 << 8)  = 0x00000100
    BottomRightDoor3_9 = 9, // (1 << 9)  = 0x00000200
    BottomRightDoor4_10 = 10, // (1 << 10) = 0x00000400

    TopRightDoor1_11 = 11, // (1 << 11) = 0x00000800
    TopRightDoor2_12 = 12, // (1 << 12) = 0x00001000
    TopRightDoor3_13 = 13, // (1 << 13) = 0x00002000
    TopRightDoor4_14 = 14, // (1 << 14) = 0x00004000

    BottomLeftRoofLight_15 = 15, // (1 << 15) = 0x00008000
    BottomRightRoofLight_16 = 16, // (1 << 16) = 0x00010000
    TopLeftRoofLight_17 = 17, // (1 << 17) = 0x00020000
    TopRightRoofLight_18 = 18, // (1 << 18) = 0x00040000

    Bit19_19 = 19, // (1 << 19) = 0x00080000
    Bit20_20 = 20, // (1 << 20) = 0x00100000
    Bit21_21 = 21, // (1 << 21) = 0x00200000

    BackLeftBrakeLight_22 = 22, // (1 << 22) = 0x00400000
    FrontLeftHeadlight_23 = 23, // (1 << 23) = 0x00800000

    BottomLeftDoor1_24 = 24, // (1 << 24) = 0x01000000
    BottomLeftDoor2_25 = 25, // (1 << 25) = 0x02000000
    BottomLeftDoor3_26 = 26, // (1 << 26) = 0x04000000
    BottomLeftDoor4_27 = 27, // (1 << 27) = 0x08000000

    TopLeftDoor1_28 = 28, // (1 << 28) = 0x10000000
    TopLeftDoor2_29 = 29, // (1 << 29) = 0x20000000
    TopLeftDoor3_30 = 30, // (1 << 30) = 0x40000000
    TopLeftDoor4_31 = 31 // (1 << 31) = 0x80000000
};
} // namespace CarDeltaBitsEnum

namespace CarManagerCarType
{
enum
{
    New_Name = 0,
    New_Name_1 = 1,
    New_Name_2 = 2,
    New_Name_3 = 3,
    ambulance = 4,
    firetruck = 5,
    police_cars = 6
};
// static_assert(sizeof(car_manager_car_type_enum) == 4);
} // namespace CarManagerCarType
namespace CarModel
{
enum
{
    alfa = 0,
    allard = 1,
    amdb4 = 2,
    apc = 3,
    bank_van = 4,
    bmw = 5,
    boxcar = 6,
    boxtruck = 7,
    bug = 8,
    CAR9 = 9,
    BUICK = 10,
    BUS = 11,
    COPCAR = 12,
    DART = 13,
    EDSEL = 14,
    CAR15 = 15,
    FIAT = 16,
    FIRETRUK = 17,
    GRAHAM = 18,
    GT24640 = 19,
    CAR20 = 20,
    GTRUCK = 21,
    GUNJEEP = 22,
    HOTDOG = 23,
    HOTDOG_D1 = 24,
    HOTDOG_D2 = 25,
    HOTDOG_D3 = 26,
    ICECREAM = 27,
    ISETLIMO = 28,
    ISETTA = 29,
    JEEP = 30,
    JEFFREY = 31,
    LIMO = 32,
    LIMO2 = 33,
    MEDICAR = 34,
    MERC = 35,
    MESSER = 36,
    MIURA = 37,
    MONSTER = 38,
    MORGAN = 39,
    MORRIS = 40,
    PICKUP = 41,
    RTYPE = 42,
    CAR43 = 43,
    SPIDER = 44,
    SPRITE = 45,
    STINGRAY = 46,
    STRATOS = 47,
    STRATOSB = 48,
    STRIPETB = 49,
    STYPE = 50,
    STYPECAB = 51,
    SWATVAN = 52,
    T2000GT = 53,
    TANK = 54,
    TANKER = 55,
    TAXI = 56,
    TBIRD = 57,
    TOWTRUCK = 58,
    TRAIN = 59,
    TRAINCAB = 60,
    TRAINFB = 61,
    TRANCEAM = 62,
    TRUKCAB1 = 63,
    TRUKCAB2 = 64,
    TRUKCONT = 65,
    TRUKTRNS = 66,
    TVVAN = 67,
    VAN = 68,
    VESPA = 69,
    VTYPE = 70,
    WBTWIN = 71,
    WRECK0 = 72,
    WRECK1 = 73,
    WRECK2 = 74,
    WRECK3 = 75,
    WRECK4 = 76,
    WRECK5 = 77,
    WRECK6 = 78,
    WRECK7 = 79,
    WRECK8 = 80,
    WRECK9 = 81,
    XK120 = 82,
    ZCX5 = 83,
    EDSELFBI = 84,
    HOTDOG_D4 = 85,
    KRSNABUS = 86,
    none = 87
};
// static_assert(sizeof(car_model_enum) == 4);
} // namespace CarModel

namespace CarDoorLock
{
enum
{
    no_lock_0 = 0,
    locked_1 = 1,
    lockout_thief_2 = 2,
    unlocked_3 = 3,
    locked_permanently_4 = 4,
    lockout_player_5 = 5,
    none_6 = 6,
};
} // namespace CarDoorLock

// Wanted level (star count / cop head), what Police_7B8 sends at each
namespace CopLevel
{
enum cop_level_enum
{
    none_0 = 0,
    police_1 = 1, // one police car
    police_2 = 2, // two police cars
    police_3 = 3, // roadblocks start
    swat_4 = 4, // a SWAT crew is added
    fbi_5 = 5, // FBI crews
    army_6 = 6, // army
};
} // namespace CopLevel

namespace CopLevelPed
{
enum
{ /* The cop level stored in the PED */
    cop_0_stars = 0,
    cop_1_stars = 600,
    cop_2_stars = 1600,
    cop_3_stars = 3000,
    cop_4_stars = 5000,
    cop_5_stars = 8000,
    cop_6_stars = 12000
};
// static_assert(sizeof(cop_level_ped_enum) == 2);
} // namespace CopLevelPed

namespace GmpZoneType
{
enum
{
    general_purpose = 0,
    navigation = 1,
    traffic_light = 2,
    unused1 = 3,
    unused2 = 4,
    arrow_blocker = 5,
    railway_station_platform = 6,
    bus_stop_pavement = 7,
    general_trigger = 8,
    unused3 = 9,
    information = 10,
    railway_station_entry_point = 11,
    railway_station_exit_point = 12,
    railway_stop_point = 13,
    gang = 14,
    local_navigation = 15,
    restart = 16,
    unused4 = 17,
    unused5 = 18,
    unsued6 = 19,
    arrest_restart = 20
};
// static_assert(sizeof(gmp_zone_type_enum) == 4);
} // namespace GmpZoneType

namespace CrewType // KFC type
{
enum
{
    paramedic_1 = 1,
    police_3 = 3,
    fbi_4 = 4,
    swat_5 = 5,
    army_6 = 6,
};
} // namespace CrewType

// Which kind of crew (or roadblock guard) is built: PoliceCrew_38::field_20_crew_kind and gCrewKind_6FEDB8
namespace CrewKind
{
enum
{
    police_1 = 1,
    swat_2 = 2,
    fbi_3 = 3,
    army_4 = 4,
};
} // namespace CrewKind

namespace PaletteTypes
{
enum
{
    wrong_type_0 = 0,
    tiles_1 = 1,
    sprites_2 = 2,
    car_remaps_3 = 3,
    ped_remaps_4 = 4,
    code_obj_remaps_5 = 5,
    map_obj_remaps_6 = 6,
    user_remaps_7 = 7,
    font_remaps_8 = 8,
};
// static_assert(sizeof(palette_types_enum) == 4);
} // namespace PaletteTypes

namespace PedBitStatus
{
enum
{
    k_ped_0x00000001 = 0x00000001,
    k_ped_0x00000002 = 0x00000002,
    k_ped_0x00000004 = 0x00000004,
    k_ped_0x00000008 = 0x00000008,
    k_ped_0x00000010 = 0x00000010,
    k_ped_0x00000020 = 0x00000020,
    k_ped_0x00000040 = 0x00000040,
    k_ped_0x00000080 = 0x00000080,
    k_ped_0x00000100 = 0x00000100,
    k_ped_0x00000200 = 0x00000200,
    k_ped_0x00000400 = 0x00000400,
    k_ped_has_weapon = 0x00000800,
    k_ped_0x00001000 = 0x00001000,
    k_ped_0x00002000 = 0x00002000,
    k_ped_0x00004000 = 0x00004000,
    k_ped_0x00008000 = 0x00008000,
    k_ped_0x00010000 = 0x00010000,
    k_ped_0x00020000 = 0x00020000,
    k_ped_0x00040000 = 0x00040000,
    k_ped_0x00080000 = 0x00080000,
    k_ped_0x00100000 = 0x00100000,
    k_ped_0x00200000 = 0x00200000,
    k_ped_0x00400000 = 0x00400000,
    k_ped_0x00800000 = 0x00800000,
    k_ped_in_flames = 0x01000000,
    k_ped_invisible = 0x02000000,
    k_ped_0x04000000 = 0x04000000,
    k_ped_0x08000000 = 0x08000000,
    k_ped_0x10000000 = 0x10000000,
    k_ped_0x20000000 = 0x20000000,
    k_ped_0x40000000 = 0x40000000,
    k_ped_electro_fingers = 0x40000000,
    k_ped_0x80000000 = 0x80000000,
};
// static_assert(sizeof(ped_bit_status_enum) == 0x4);
} // namespace PedBitStatus

namespace PedMajorState
{
enum
{
    ped_major_New_Name = 0,
    ped_major_walking = 1,
    ped_major_New_Name_1 = 2,
    ped_major_New_Name_2 = 3,
    ped_major_staying = 4,
    ped_major_driving = 5,
    ped_major_entering_a_car = 6,
    ped_major_getting_out_a_car = 7
};
// static_assert(sizeof(ped_major_state) == 0x1);
} // namespace PedMajorState

namespace PedOcupation
{
enum
{ /* The ocupation that a PED can have */
    player = 0,
    empty = 1,
    unknown_1 = 2,
    dummy = 3,
    unknown_2 = 4,
    driver = 5,
    unknown_3 = 6,
    taxi_customer_7 = 7,
    bus_customer_8 = 8,
    train_customer_9 = 9,
    robbed_driver_10 = 10, // victim of a steal
    fleeing_robbed_driver_11 = 11,
    angry_armed_robbed_driver_12 = 12, // get the car back from the thief
    very_angry_armed_robbed_driver_13 = 13, // shoots at the thief
    psycho = 14,
    mugger = 15,
    car_thief = 16,
    bank_robber = 17,
    criminal = 18,
    armed_gang_member_19 = 19,
    unknown_11 = 20,
    unknown_12 = 21,
    elvis = 22,
    paramedic_23 = 23,
    police = 24,
    swat = 25,
    fbi = 26,
    army_army = 27,
    guard = 28,
    walking_guard_29 = 29,
    unknown_cop_occu_30 = 30,
    unknown_cop_occu_31 = 31,
    guard_against_player = 32,
    criminal_type_1 = 33,
    criminal_type_2 = 34,
    special_groups_member = 35,
    tank_driver = 36,
    roadblock_cop_37 = 37,
    fireman = 38,
    road_block_tank_man = 39,
    mad_mugger_40 = 40, // npcs spawned when MAKE_ALL_CHARS_MUGGERS is active
    drone = 41,
    gang_driver_42 = 42,
    stand_still_bloke = 43,
    elvis_leader = 44,
    refugees = 45,
    any_law_enforcement = 46,
    any_emergency_service_man = 47,
    any_gang_member = 48,
    any_elvis = 49,
    scared_driver_50 = 50, // scared by shooting at his car
    no_occupation = 51,
};
// static_assert(sizeof(ped_ocupation_enum) == 0x4);
} // namespace PedOcupation

namespace PedRemap
{
enum
{
    ped_remap_blue_police = 0,
    ped_remap_green_police = 1,
    ped_remap_red_police = 2,
    ped_remap_yellow_police = 3,
    ped_remap_army = 4,
    ped_remap_red_head_redneck = 5,
    ped_remap_blond_head_redneck = 6,
    ped_remap_yellow_scientist = 7,
    ped_remap_zaibatsu = 8,
    ped_remap_kristna = 9,
    ped_remap_russian = 10,
    ped_remap_lonny = 11,
    ped_remap_elvis = 12,
    ped_remap_yakuza = 13,
    ped_remap_fireman = 14,
    ped_remap_green_shorts = 15,
    ped_remap_medic = 16,
    ped_remap_mugger = 17,
    ped_remap_blue_dummy = 18,
    ped_remap_light_blue_dummy = 19,
    ped_remap_tshirt_and_shorts_dummy = 20,
    ped_remap_short_sleeve = 21,
    ped_remap_prison_uniform = 22,
    ped_remap_hulk1_normal = 23,
    ped_remap_hulk2_green = 24,
    ped_remap_player = 25,
    ped_remap_naked_dummy = 26,
};
// static_assert(sizeof(ped_remap_enum) == 0x1);
} // namespace PedRemap

namespace PedState1Alias
{
enum
{
    ped_walking = 0,
    unused1 = 1,
    ped_going_to_car = 2,
    ped_entering_a_car = 3,
    ped_getting_out_a_car = 4,
    unused2 = 5,
    unused3 = 6,
    ped_staying = 7,
    ped_fall_on_ground = 8,
    ped_wasted = 9,
    ped_driving = 10,
};
// static_assert(sizeof(ped_state1_enum) == 0x4);
} // namespace PedState1Alias

namespace PedState2
{
enum
{
    ped2_walking_0 = 0,
    Unknown_1 = 1,
    Unknown_2 = 2,
    Unknown_3 = 3,
    ped2_following_a_car_4 = 4,
    Unknown_5 = 5,
    ped2_entering_a_car_6 = 6,
    ped2_getting_out_a_car_7 = 7,
    Unknown_8 = 8,
    Unknown_9 = 9,
    ped2_driving_10 = 10,
    ped2_staying_14 = 14,
    Unknown_15 = 15,
    Unknown_16 = 16,
    Unknown_17 = 17,
    Unknown_18 = 18,
    falling_19 = 19,
    sinking_20 = 20,
    Unknown_21 = 21,
    lying_on_floor_22 = 22,
    Unknown_23 = 23,
    Unknown_24 = 24,
    Unknown_25 = 25,
    Unknown_26 = 26,
    electrocuted_27 = 27,
    Unknown_28 = 28,
    Unknown_29 = 29,
    Unknown_30 = 30,
};
// static_assert(sizeof(ped_state_2) == 0x4);
} // namespace PedState2

namespace CharB4State
{
enum
{
    Unknown_0 = 0,
    Unknown_1 = 1,
    Unknown_2 = 2,
    Unknown_3 = 3,
    Unknown_4 = 4,
    Unknown_5 = 5,
    Unknown_6 = 6,
    Unknown_7 = 7,
    Unknown_8 = 8,
    Unknown_9 = 9,
    Unknown_10 = 10,
    Unknown_11 = 11,
    Unknown_12 = 12,
    Unknown_13 = 13,
    Unknown_14 = 14,
    Jumping_15 = 15,
    Colliding_With_Car_27 = 27,
    Smoking_35 = 35,
    Interacting_Car_Door_36 = 36,
};
} // namespace CharB4State

namespace CharAnimState
{
enum
{
    Unknown_0 = 0,
    Unknown_1 = 1,
    Unknown_2 = 2,
    Unknown_3 = 3,
    Unknown_4 = 4,
    Jumping_5 = 5,
    Entering_Car_6 = 6,
    Exiting_Car_7 = 7,
    Getting_pulled_out_of_car_8 = 8,
    Unknown_9 = 9,
    Unknown_10 = 10,
    Normal_Fall_11 = 11,
    Lethal_Fall_12 = 12,
    Unknown_13 = 13,
    Unknown_14 = 14,
};
} // namespace CharAnimState

namespace SpecSurfaceType
{
enum
{
    spec_grass = 2,
    spec_road_special = 3,
    spec_water = 4,
    spec_electrified = 5,
    spec_electrified_platform = 6,
    spec_wood_floor = 7,
    spec_metal_floor = 8,
    spec_metal_wall = 9,
    spec_grass_dirt_wall = 10,
};
// static_assert(sizeof(spec_surface_type_enum) == 4);
} // namespace SpecSurfaceType

namespace CarSurfaceType
{
enum
{
    flat_surface_0 = 0,
    slope_northwards_1 = 1,
    slope_southwards_2 = 2,
    slope_westwards_3 = 3,
    slope_eastwards_4 = 4,
    air_surface_6 = 6,
    unknown_surface_7 = 7,
    water_surface_8 = 8,
    unknown_surface_9 = 9,
};
} // namespace CarSurfaceType

namespace SpriteTypes
{
enum
{
    unknown_0 = 0, // At least in the function sprite_FUN_004b9aa0, but don't know what represents.
    object_1 = 1, // has an Object_2C like code_obj1_4 and map_obj_5
    car_2 = 2,
    ped_3 = 3,
    code_obj1_4 = 4,
    map_obj_5 = 5,
    user_6 = 6,
    font_7 = 7,
    code_obj2_8 = 8,
};
// static_assert(sizeof(sprite_types_enum) == 4);
} // namespace SpriteTypes

namespace WeaponType
{
enum
{
    pistol = 0,
    smg = 1,
    rocket = 2,
    shocker = 3,
    molotov = 4,
    grenade = 5,
    shotgun = 6,
    electro_batton = 7,
    flamethrower = 8,
    silence_smg = 9,
    dual_pistol = 10,
    weapon_11 = 11,
    weapon_12 = 12,
    weapon_13 = 13,
    weapon_14 = 14,
    car_bomb = 15,
    oil_stain = 16,
    car_mines = 17,
    car_smg = 18,
    tank_main_gun = 19,
    fire_truck_gun = 20,
    fire_truck_flamethrower = 21,
    army_gun_jeep = 22,
    weapon_0x17 = 23,
};
// static_assert(sizeof(weapon_type) == 0x4);
} // namespace WeaponType

namespace Objects
{
enum
{
    object_0 = 0,
    bin_lid_1 = 1,
    bollard_2 = 2,
    cone_3 = 3,
    boxes_4 = 4,
    blaster_5 = 5,
    rubbish_6 = 6,
    bin_7 = 7,
    animating_oil_8 = 8,
    oil_9 = 9,
    mine_10 = 10,
    bush_11 = 11,
    crate_12 = 12,
    footy_13 = 13,
    hardbox_14 = 14,
    newsdis_15 = 15,
    oildrum_16 = 16,
    tyre_17 = 17,
    hydrant_lid_18 = 18,
    hydrant_19 = 19,
    hydrant_unlid_20 = 20,
    roadblock_21 = 21,
    bench_22 = 22,
    package_23 = 23,
    object_24 = 24,
    tower_25 = 25,
    object_26 = 26,
    object_27 = 27,
    object_28 = 28,
    object_29 = 29,
    object_30 = 30,
    object_31 = 31,
    object_32 = 32,
    object_33 = 33,
    object_34 = 34,
    object_35 = 35,
    object_36 = 36,
    object_37 = 37,
    object_38 = 38,
    object_39 = 39,
    object_40 = 40,
    explode_medium_41 = 41,
    object_42 = 42,
    object_43 = 43,
    object_44 = 44,
    object_45 = 45,
    object_46 = 46,
    moving_collect_00_96 = 96,
    moving_collect_01_97 = 97,
    moving_collect_02_98 = 98,
    moving_collect_03_99 = 99,
    moving_collect_04_100 = 100,
    moving_collect_05_101 = 101,
    moving_collect_06_102 = 102,
    moving_collect_07_103 = 103,
    moving_collect_08_104 = 104,
    moving_collect_09_105 = 105,
    moving_collect_10_106 = 106,
    moving_collect_11_107 = 107,
    moving_collect_12_108 = 108,
    moving_collect_13_109 = 109,
    ped_pushed_110 = 110,
    moving_collect_15_111 = 111,
    moving_collect_16_112 = 112,
    explosion_113 = 113,
    water_cannon_114 = 114,
    moving_collect_19_115 = 115,
    moving_collect_20_116 = 116,
    moving_collect_21_117 = 117,
    moving_collect_22_118 = 118,
    moving_collect_23_119 = 119,
    road_skidmark_120 = 120,
    moving_collect_25_121 = 121,
    pedestrian_crossing_marker_122 = 122,
    moving_collect_27_123 = 123,
    moving_collect_28_124 = 124,
    moving_collect_29_125 = 125,
    moving_collect_30_126 = 126,
    explosion_fragments_127 = 127,
    rocket_bullet_128 = 128,
    bus_stop_marker_129 = 129,
    car_shop_130 = 130,
    car_shop_busy_131 = 131,
    moving_collect_36_132 = 132,
    moving_collect_37_133 = 133,
    crane_unknown_134 = 134,
    crane_rotor_135 = 135,
    crane_hook_136 = 136,
    crane_base_137 = 137,
    moving_molotov_138 = 138,
    conveyor_139 = 139,
    crane_counterweight_140 = 140,
    destructor_141 = 141,
    crusher_piston_142 = 142,
    crusher_central_spot_143 = 143,
    object_144 = 144,
    particle_system_145 = 145,
    firejet_146 = 146,
    object_147 = 147,
    small_brown_skid_148 = 148,
    small_grey_skid_149 = 149,
    crusher_piston_base_150 = 150,
    medium_brown_skid_151 = 151,
    medium_grey_skid_152 = 152,
    medium_red_skid_153 = 153,
    flamethrower_fire_154 = 154,
    car_stop_155 = 155,
    big_white_skid_156 = 156,
    medium_white_skid_157 = 157,
    small_white_skid_158 = 158,
    object_159 = 159,
    rocket_160 = 160,
    savepoint_161 = 161,
    car_shop_162 = 162,
    busy_car_shop_163 = 163,
    blue_phone_164 = 164,
    maybe_train_headlight_165 = 165,
    diagonal_wall_collision_obj_166 = 166,
    maybe_door_trigger_167 = 167,
    door_unknown_169 = 169,
    traffic_light_hull_170 = 170,
    red_phone_177 = 177,
    yellow_phone_179 = 179,
    green_phone_181 = 181,
    grenade_obj_183 = 183,
    grenade_obj_on_ground_184 = 184,
    huge_red_skid_189 = 189,
    huge_white_skid_190 = 190,
    huge_brown_skid_191 = 191,
    shotgun_bullet_192 = 192,
    tanktop_193 = 193,
    fire_hitting_194 = 194,
    object_195 = 195,
    animating_rubbish_196 = 196,
    fire_197 = 197, // like the one in peds
    maybe_bullet_on_fire_198 = 198,
    object_199 = 199,
    remote_200 = 200,

    machine_gun_weapon_201 = 201,
    molotov_weapon_204 = 204,
    shotgun_weapon_206 = 206,
    flamethrower_weapon_208 = 208,

    health_powerup_230 = 230,
    armor_powerup_231 = 231,
    get_out_jail_powerup_232 = 232,
    invulnerability_powerup_234 = 234,
    invisibility_powerup_239 = 239,

    shop_car_smg_250 = 250,
    shop_car_oil_stain_251 = 251,
    shop_car_mines_252 = 252,

    machine_gun_bullet_254 = 254,
    traffic_light_lighting_255 = 255,
    ped_crossing_trigger_258 = 258,
    pistol_bullet_265 = 265,
    secret_token_266 = 266,

    electrobaton_bullet_277 = 277,
    sound_object_type_1_278 = 278,
    sound_object_type_2_279 = 279,
    power_generator_281 = 281,
    destroyed_powergen_282 = 282,
    invisible_powergen_283 = 283,
    powergen_light_284 = 284,
    destroyed_invisible_powergen_285 = 285,
    kill_frenzy_icon_286 = 286,
    loonies_icon_287 = 287,
    yakuza_icon_288 = 288,
    zaibatsu_icon_289 = 289,
    redneck_icon_290 = 290,
    scientists_icon_291 = 291,
    krishna_icon_292 = 292,
    russian_mafia_icon_293 = 293,
    tunnel_blocker_294 = 294,
};
} // namespace Objects

namespace ThreatReaction
{
enum
{
    no_reaction_0 = 0,
    react_as_emergency_1 = 1,
    react_as_normal_2 = 2,
    run_away_3 = 3,
};
} // namespace ThreatReaction

namespace ObjectiveStatus
{
enum
{
    not_finished_0 = 0,
    passed_1 = 1,
    failed_2 = 2,
};
} // namespace ObjectiveStatus

namespace Objectives
{
enum
{
    no_obj_0 = 0,
    flee_on_foot_till_safe_1 = 1,
    flee_char_on_foot_till_safe_2 = 2,
    flee_char_on_foot_always_3 = 3,
    flee_char_any_means_till_safe_4 = 4,
    flee_char_any_means_always_5 = 5,
    flee_char_always_once_car_stopped_6 = 6,
    objective_7 = 7,
    objective_8 = 8,
    objective_9 = 9,
    objective_10 = 10,
    objective_11 = 11,
    goto_area_on_foot_12 = 12,
    goto_area_any_means_13 = 13,
    goto_area_in_car_14 = 14,
    objective_15 = 15,
    goto_char_on_foot_16 = 16,
    objective_17 = 17,
    objective_18 = 18,
    kill_char_any_means_19 = 19,
    kill_char_on_foot_20 = 20,
    kill_car_21 = 21,
    kill_frenzy_22 = 22,
    punch_char_23 = 23,
    guard_spot_24 = 24,
    guard_area_25 = 25,
    wait_on_foot_26 = 26,
    wait_in_car_27 = 27,
    objective_28 = 28,
    objective_29 = 29,
    objective_30 = 30,
    time_waited_in_car_31 = 31,
    objective_32 = 32,
    objective_33 = 33,
    objective_34 = 34,
    enter_car_as_driver_35 = 35,
    leave_car_36 = 36,
    enter_train_37 = 37,
    leave_train_38 = 38,
    objective_39 = 39,
    objective_40 = 40,
    objective_41 = 41,
    patrol_on_foot_42 = 42,
    objective_43 = 43,
    objective_44 = 44,
    objective_45 = 45,
    objective_46 = 46,
    objective_47 = 47,
    objective_48 = 48,
    objective_49 = 49,
    objective_50 = 50,
    objective_51 = 51,
    objective_52 = 52,
    objective_53 = 53,
    objective_54 = 54,
    follow_car_in_car_55 = 55,
    follow_car_on_foot_with_offset_56 = 56,
    fire_at_object_from_vehicle_57 = 57,
    destroy_object_58 = 58,
    destroy_car_59 = 59,
    turret_put_out_car_fire_60 = 60,
    objective_61 = 61,
};
} // namespace Objectives

namespace ThreatSearch
{
enum
{
    no_threats_0 = 0,
    line_of_sight_1 = 1,
    area_2 = 2,
    area_player_threat_only_3 = 3,
    line_of_sight_player_threat_only_4 = 4,
    area_player_only_5 = 5,
    line_of_sight_player_only_6 = 6,
};
} // namespace ThreatSearch

namespace PedType
{
enum
{
    unknown_0 = 0,
    unknown_1 = 1,
    player_2 = 2,
    dummy_3 = 3,
    special_ped_4 = 4,
    script_created_5 = 5,
    dummy_with_occupation_6 = 6,
};
} // namespace PedType

// Keeps the number of distinct names in headers MapRenderer.cpp includes unchanged after merging ped_type_enum into
// ped_type; it flips Draw3SidedDiagonalDownRight_4EF520 / Draw4SidedDiagonalUpLeft_4EF880 otherwise (see car_flags.hpp
// and docs/matching_quirks.md). Do not remove without running the whole build.
enum
{
    kPedTypeNameCount0,
    kPedTypeNameCount1,
    kPedTypeNameCount2,
    kPedTypeNameCount3,
    kPedTypeNameCount4
};

namespace PedState1
{
enum
{
    walking_0 = 0,
    flee_or_running_1 = 1,
    findind_path_2 = 2,
    entering_car_3 = 3,
    exiting_car_4 = 4,
    unknown_5 = 5,
    unknown_6 = 6,
    standing_still_7 = 7,
    immobilized_8 = 8, // ped lying on ground, or getting repeled by blast waves, or getting pushed, or on air, or sinking
    dead_9 = 9,
    in_car_10 = 10,
};
} // namespace PedState1

namespace CarDamageAreas
{
enum
{
    FrontLeft_0 = 0x0,
    FrontRight_1 = 0x1,
    BackLeft_2 = 0x2,
    BackRight_3 = 0x3,
    Window_4 = 0x4,
};
} // namespace CarDamageAreas

namespace CarDamageAreasFlags
{
enum
{
    FlagsBackRight_1 = 0x1,
    FlagsBackLeft_2 = 0x2,
    FlagsFrontLeft_4 = 0x4,
    FlagsFrontRight_8 = 0x8,
    FlagsWindow_10 = 0x10,
};
} // namespace CarDamageAreasFlags

namespace Gta2Error
{
enum Gta2Error
{
    DMAVideoInitFail = 2U,
    SetWindowModeFail = 5U,
    GetValidHDCFail = 6U,
    SetWindowPosFail = 7U,
    DirectInputInitFail = 8U,
    DirectInputSetDataFormatFail = 9U,
    DirectInputSetCooperativeLevelFail = 10U,
    FtellError = 13U,
    FseekError = 14U,
    FileReadFailure = 15U,
    FreeloaderEpisodeUnknown = 16U,
    FileCloseError = 17U,
    FileBiggerThanBuffer = 18U,
    WritingZeroBytesToFile = 19U,
    FileWriteFailure = 20U,
    FileReadingNotOpenedFile = 21U,
    ungetcFailrue = 24U,
    FileInvalidInteger = 25U,
    FileEndOfFileInTextString = 26U,
    TextStringTooLong = 27U,
    ZeroBytesAlloc = 28U,
    DirectInputNoObject = 30U,
    DirectInputCreateFail = 31U,
    OutOfMemoryNewOperator = 32U,
    FileOverflow = 34U,
    DataNotLoaded = 35U,
    InvalidStringInParameterList = 36U,
    RepeatedFileInParameterList = 37U,
    InvalidZoneData = 39U,
    InvalidMapObjectData = 40U,
    InvalidLoadedObjectInfoData = 41U,
    CloseRegistryKeyFail = 42U,
    CreateRegistryKeyFail = 43U,
    NoPlayers = 44U,
    ClassNotAllocated = 45U,
    SetRegistryValueFail = 46U,
    LoopInTree = 47U,
    DisplayTreeOutOfSpace = 48U,
    DivideByZero = 49U,
    FileWriteByteFail = 50U,
    InvalidMapLightData = 51U,
    UsingUninitialisedSprite = 53U,
    InvalidCorners = 54U,
    MissionProcessAllocMemFail = 55U,
    MissionProcessOutOfSlots = 56U,
    CheckWallsSlopeInvalidSlope = 59U,
    NoPlayerPointer = 60U,
    CharacterHasNoPedPointer_1 = 61U,
    CharacterDefaultCase = 62U,
    InvalidObjectiveCoordinates = 64U,
    ScriptfileNo_LEVELSTART_Declaration = 65U,
    ScriptfileTooMany_PLAYER_PED_Declarations = 66U,
    MapChangeOverflowBlocks = 67U,
    MapChangeOverflowColumns = 68U,
    ScriptfileFailedToCreatePed = 69U,
    MissionStackAllocFail = 70U,
    MissionStackPushNullItem = 71U,
    SecurityFail = 72U,
    MissStackItemGrabNoStackSpaceLeftFail = 73U,
    RemoveCharFromCharlistFail = 74U,
    InvalidDoor = 75U,
    ScriptfileCreateCarFail = 76U,
    ObjectHasDataWhenItShouldnt = 77U,
    CharacterHasNoCarObjectivePointer_1 = 79U,
    CharacterHasNoCarObjectivePointer_2 = 80U,
    CharacterHasNoPedPointer_2 = 81U,
    GroupMemberNotRemoved = 82U,
    TooManyGroupMembers = 83U,
    GroupCreateTooManyGroupsInUse = 84U,
    PedDefaultCase = 85U,
    PedSpriteZIs0 = 88U,
    PedsCarInfoIsNULL = 89U,
    SidTooManyReferences = 90U,
    SidNegativeReferences = 91U,
    NoFreeTiles = 92U,
    InvalidFileType = 93U,
    InvalidFileVersion = 94U,
    CharacterNoFootOrCarPointer = 95U,
    CreateMedicalUnitFail = 96U,
    CharacterHasNoInCarPointer = 97U,
    TooManyTrafficLights = 98U,
    TooManyInfoAndNavigationZonesInMap = 99U,
    InvalidZoneForInfoSetting = 100U,
    FailedToCreateCarInRecycle = 101U,
    EmptyStringPassedToFunction = 102U,
    FirstPlayerIsNULL = 103U,
    NextPlayerIsNULL = 104U,
    NullBlockPassedToCheckForArrowsInRoutefinderCode = 105U,
    TooManyJunctionsInMapForRoutefinder = 106U,
    CarTryingSuicideDidntDealWithDriver = 107U,
    AIObjectiveRequiredByCharNoLongerActive = 108U,
    NoMoreSpaceIn_simp_text_DisplayBuffer = 109U,
    TooManyScoreTextures = 120U,
    TryingToGetUnitOfZeroVector = 121U,
    AssertionFail = 123U,
    TryingToExpandAnEmptyNodeInRouterfinder = 124U,
    NoValidBackcarForAnCarRelatedStruct = 125U,
    Gta2ManagerLaunchError = 126U,
    MessageTextTooLongToWrap = 127U,
    TooManyTimers = 128U,
    TooManyMessages = 129U,
    TooManyArrows = 130U,
    NoRestartZone = 131U,
    MapNotCompressed = 132U,
    InvalidSmybolDataInSymboltableBuffer = 133U,
    InvalidMissionScriptFileSpecifiedInBob = 134U,
    PointingArrowAtANoLongerValidCharacter = 136U,
    PointingArrowAtANoLongerValidCar = 137U,
    PointingArrowAtANoLongerValidObject = 138U,
    CarRoutefindNoRoutfinderDataInMap = 139U,
    IllegalTrainStationNoPlatformZone = 143U,
    NoMoreTrainSpace = 144U,
    BaseChangeLogOverflow = 145U,
    GswTrainMovingAcrossSlope = 146U,
    GswTrainUnknownDirection = 147U,
    GswTrainUnknownSlope = 148U,
    InvalidCarRating = 149U,
    KeyboardTypeNotHandled = 150U,
    KeyboardTypeFileOpenError = 151U,
    SeqFileNotFound = 152U,
    MultipleSeqFilesFound = 153U,
    SeqFileOpenError = 154U,
    TooManyMainBlocks = 155U,
    MainBlockMustPrecedeBonus = 156U,
    TooManyBonusBlocks = 157U,
    InvalidBlockType = 158U,
    InvalidFirstLineCharacter = 159U,
    LineInterruptedByNewline = 160U,
    LabelTooLong = 161U,
    UndefinedLabel = 162U,
    LineDataTooLong = 163U,
    UnexpectedLabel = 164U,
    NoValidMenuOptions = 165U,
    GmpFilenameMismatch = 166U,
    StyFilenameMismatch = 167U,
    ScrFilenameMismatch = 168U,
    BinkBufferOpenError = 169U,
    BinkOpenError = 170U,
    CreditsLineTooLong = 183U,
    TooManyCreditsLines = 184U,
    InvalidLevelAdvancement = 185U,
    LevelNotOpened = 186U,
    InvalidBackgroundImageSize = 187U,
    InvalidCreditTextColor = 188U,
    InvalidMultiplayerGameType = 189U,
    UnableToHandleAttachedPed = 171U,
    EmptyReplayFile = 172U,
    ReplaySyncError = 173U,
    TargaMemoryAllocationError = 174U,
    TargaFileInvalidImage = 175U,
    TargaFileInvalidFormat = 176U,
    TargaFileInvalidBitsPerPixel = 177U,
    TargaFileHasPalette = 178U,
    TargaFileInvalidOrigin = 179U,
    TargaFileZeroWidthOrHeight = 180U,
    TargaFileIdReadError = 181U,
    TargaFileImageReadError = 182U,
    InvalidCarModel = 1001U,
    MultipleSpriteDeltas = 1002U,
    TooManySpritesWithDeltas = 1003U,
    InvalidAbsoluteSpriteNumber = 1004U,
    TooManySprites = 1005U,
    InvalidCase = 1006U,
    InvalidAbsoluteVirtualPaletteNumber = 1007U,
    InvalidFontNumber = 1008U,
    InvalidTileNumber = 1009U,
    InvalidNewTileNumber = 1010U,
    DllLoadError = 1011U,
    InvalidPhysicalPaletteNumber = 1012U,
    ZoneNotFoundError = 1013U,
    InvalidCarModelGearSpeeds = 1014U,
    TooManyZones = 1016U,
    InvalidZoneNumber = 1017U,
    InvalidMapObjectNumber = 1018U,
    FileTooLarge = 1022U,
    OutOfMemory = 1029U,
    InvalidChunkSize = 1033U,
    InvalidParameterListSwitch = 1034U,
    InvalidLineInfo = 1073U,
    ReplayFileTooLarge = 1035U,
    InvalidVideomode = 1036U,
    FullscreenVideomodeSetError = 1037U,
    TooManyLoadedObjectInfos = 1020U,
    TooManyPlayers = 1023U,
    InvalidPlayerNumber = 1024U,
    InvalidGridWidth = 1025U,
    InvalidGridHeight = 1026U,
    InvalidZCoordinate = 1027U,
    UnprocessedEvent = 1028U,
    ListTypeOutOfSpace = 1040U,
    NoCollisionBoxInSprite = 1039U,
    InvalidCollisionBoxInSprite = 1021U,
    InvalidObjectModelNumber = 1041U,
    ObjectModelNumberAlreadyAdded = 1042U,
    NoSpaceForObjectModelNumber = 1043U,
    CheckWallsSlopeSizeTooBig = 1044U,
    FailedToCreateObjectInScriptfile = 1045U,
    AttemptToKillDeadMissionThread = 1046U,
    TooManyCarDoors = 1047U,
    ReleasingNullPtrInReleasePassive = 1048U,
    TryingToPacifyNullListElement = 1049U,
    ObjectBackPointerMissingInSprite = 1050U,
    NoRigidBodyInCar = 1051U,
    NoFreeSidForCharacter = 1052U,
    InvalidDoorInfoNumber = 1053U,
    IllegalAngleValue = 1054U,
    InvalidTileTexture = 1055U,
    InvalidCacheNumber = 1056U,
    InvalidScoreTextureNumber = 1068U,
    InvalidZoneInformation = 1019U,
    CounterOutOfBoundsInternalCount = 1057U,
    OutOfRangeRouteId = 1058U,
    InvalidScorePosition = 1066U,
    InvalidPlayerId = 1067U,
    TooManyCharactersInScore = 1069U,
    InvalidTextFile = 1071U,
    StringStartsWithEndOfLine = 1072U,
    CarInCollisionWithWallAtSafePosition = 1076U,
    ObjectInCollisionWithWallAtSafePosition = 1080U,
    IllegalDeviceCode = 1077U,
    ActivatingNullPtrInActivate = 1078U,
    ActivatingItemTwice = 1079U,
    IllegalTurntableIndex = 1081U,
    TooManyYRoadSegments = 1082U,
    TooManyXRoadSegments = 1083U,
    InvalidTimerHandle = 1084U,
    InvalidArrowHandle = 1085U,
    UnallocatedArrowHandle = 1086U,
    CarAlreadyDoingRoutefind = 1090U,
    CounterCheckFailed = 1137U,
    InvalidCarModelStyleData = 1107U,
    ReleasingTwiceInReleasePassive = 1138U,
    ReleasingTwiceInRelease = 1139U,
    AssertionFailure = 1095U,
    TooManySymbols = 1096U,
    FailedToCreateCharacterInScript = 1098U,
    FailedToFindUidInScript = 1099U,
    IllegalZoneIdInSymbolTable = 1100U,
    FailedToCreateArrowInScript = 1101U,
    MultipleDeltaEntries = 1002U,
    UnableToLoadDLL = 1011U,
    ZoneNotFound = 1013U,
    InvalidGearSpeedsInGciFile = 1014U,
    InvalidSwitchInParameterList = 1034U,
    InvalidLine = 1073U,
    InvalidVideomodeToSearch = 1036U,
    FailedToSetFullscreenVideomode = 1037U,
    EventLeftUnprocessed = 1028U,
    OutOfSpaceForListType = 1040U,
    CollisionBoxInvalidInSprite = 1021U,
    AttemptToKillAlreadyDeadMissionThread = 1046U,
    CarModelHasTooManyDoors = 1047U,
    IllegalValueForAnAngle = 1054U,
    SettingInvalidZoneInformation = 1019U,
    CounterHasOutOfBoundsInternalCount = 1057U,
    InvalidPlayerID = 1067U,
    IllegalDeviceCodeForThisMachine = 1077U,
    IllegalIndexIntoTurntable = 1081U,
    CarModelHasInvalidStyleData = 1107U,
    AssertionFailure_2 = 1095U,
    LineAccessingCarThatHasntBeenCreated = 1102U,
    LineAccessingCharacterThatHasntBeenCreated = 1103U,
    LineAccessingObjectThatHasntBeenCreated = 1104U,
    LineAccessingCounterThatHasntBeenCreated = 1110U,
    LineAccessingCharacterWithNoGroup = 1111U,
    LineAccessingTriggerThatHasntBeenCreated = 1106U,
    LineAccessingCraneThatHasntBeenCreated = 1108U,
    LineOrderingDriverOutOfCarButCharIsNotDriver = 1109U,
    LineAccessingArrowThatHasntBeenCreated = 1112U,
    AttemptingToDoSetGangInfoOnInvalidGangName = 1113U,
    FailedToCreateCraneInScriptLine = 1114U,
    FailedToCreateConveyorInScriptLine = 1115U,
    FailedToCreateGeneratorInScriptLine = 1116U,
    FailedToCreateDestructorInScriptLine = 1117U,
    AttemptingToPos32ArrowAtLogicalItem = 1118U,
    LineAccessingDoorThatHasntBeenCreated = 1119U,
    FailedToCreateCrusherInScriptLine = 1120U,
    LineAccessingConveyorThatHasntBeenCreated = 1121U,
    LineAccessingGeneratorThatHasntBeenCreated = 1123U,
    InvalidSpecialTileNumber = 1124U,
    RepeatedSpecialTileNumber = 1125U,
    LineAccessingDestructorThatHasntBeenCreated = 1122U,
    CheckingPhoneInMissionscriptWithoutExecutingAnswerPhone = 1126U,
    MapHasTooManyColumns = 1127U,
    MapHasTooManyBlocks = 1129U,
    StyleFileContainsUnexpectedNumberOfFonts = 1128U,
    AttemptingIsItemOnscreenOnIllegalType = 1130U,
    UnableToCreateCarToPlaceOnTopOfTrailerInScript = 1131U,
    UnableToFindZonenameToSetupInfoForInScript = 1132U,
    UnableToFindZonenameInSetGangInfoInScript = 1133U,
    UnableToFindZonenameInSetStationInfoInScript = 1134U,
    UnableToFindPlatformInSetStationInfoInScript = 1135U,
    UnableToFindTrainstationAssociatedWithPlatformInSetStationInfoInScript = 1136U,
    UnableToFindGangMatchingNameInSetGangRespect = 1140U,
    UnableToFindGangMatchingNameInSetGangRespect2 = 1141U,
    UnableToFindGangnameInAlterGangRespect = 1142U,
    TryingToChangeGangRespectOnNonPlayerControlledCharacter = 1143U,
    UnableToFindGangMatchingNameInCheckGangRespect = 1144U,
    UnableToFindZoneMatchingNameInIsCharInGangzone = 1145U,
    UnableToFindZoneMatchingNameInIsCharInZone = 1146U,
    UnableToFindGangMatchingNameInAddCharToGang = 1147U,
    MissionScriptIsReferringToTextIdThatDoesNotExist = 1148U,
    ErrorWith1Param = 1400U,
    InvalidRelativeSpriteNumber = 2001U,
    InvalidRelativeVirtualPaletteNumber = 2002U,
    InvalidRelativeFontSpriteNumber = 2003U,
    IncorrectAspectRatioInScreenMode = 2004U,
    ListTypeCorruptedAtGrab = 2024U,
    ListTypeCorruptedAtRelease = 2025U,
    ListTypeCorruptedAtTraverse = 2026U,
    ListTypeCorruptedAtActivate = 2030U,
    ListTypeCorruptedAtPacify = 2031U,
    CantDeleteElementFromList = 2005U,
    InvalidZsInSprite = 2006U,
    DoorTooCloseInCarModel = 2007U,
    InvalidSpriteTypeAndNumber = 2008U,
    TrafficLightsTooCloseToEdgeOfWorld = 2009U,
    InvalidFrameInObjectModel = 2010U,
    IllegalCoordinate = 2011U,
    IllegalGoodBadRatioForCarModelChoice = 2012U,
    ModelRepeatedInFile = 2019U,
    BinmakeError = 2014U,
    InvalidSize = 2018U,
    CarInCollisionWithSpriteAtSafePosition = 2015U,
    ObjectInCollisionWithSpriteAtSafePosition = 2016U,
    InvalidCraneTarget = 2017U,
    TooManyDoorInfosDeclaredInScript = 2020U,
    IllegalTrainStationNoEntryZone = 2021U,
    IllegalTrainStationNoExitZone = 2022U,
    IllegalTrainStationNoStopZone = 2023U,
    ErrorWith2Params = 2401U,
    IncorrectReplayVersion = 2027U,
    SyncErrorRandom = 2028U,
    SyncErrorMission = 2029U,
    VideomodeNotAvailable = 3003U,
    FailedToSetVideomode = 3004U,
    CantDeleteSpriteFromCollisionList = 3005U,
    AddingSpriteToCollisionListTwice = 3006U,
    InvalidMapAccess = 3007U,
    InvalidMapChangeAccess = 3008U,
    InvalidPaletteTypeForSpriteTypeAndNumber = 3009U,
    InvalidSpriteTypeAndNumberRemap = 3010U,
    ModelCountMismatchInStyleFile = 3013U,
    InvalidBlockInTrainCode = 3011U,
    TrainOnBlockWithIncorrectArrow = 3012U,
    AttemptingToDisableRoadAtIllegalCoordinates = 3014U,
    AttemptingToEnableRoadAtIllegalCoordinates = 3015U,
    IllegalBlockForTrainCreation = 3016U,
    FailedToCreateCarriage = 3017U,
    ErrorInSetupOfParkCommand = 3018U,
    InvalidXCoordinateInSpriteType = 4001U,
    InvalidYCoordinateInSpriteType = 4002U,
    InvalidZCoordinateInSpriteType = 4003U,
    PlacingGangPhoneWhenNoGang = 4007U,
    IllegalCoordinate3D = 4004U,
    InvalidDiagonalBlock = 4008U,
    NoDirectionArrowFoundInBlockWithSlope = 4005U,
    DirectXError = 9011U,
    TooManyPhonesForGang = 8001U,
    TooManyLinesInBriefing = 7004U,
    CouldntFindAttractModeFile = 7006U,
    TextIdentifierNotFound = 7001U,
    BriefingTooLong = 7002U,
    BriefingTooLongToWrap = 7003U,
    InvalidFraction = 5001U,
    InvalidAmbientLightValue = 5002U,
    UnableToFindRoute = 6001U,
    ListError = 666U,
    NoErrorMessageDeclared = 1000U,
};
} // namespace Gta2Error
