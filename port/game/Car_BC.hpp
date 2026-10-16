#pragma once

#include "BitSet32.hpp"
#include "Car_10.hpp"
#include "Function.hpp"
#include "Object_3C.hpp"
#include "Ped.hpp"
#include "Ped_List_4.hpp"
#include "Pool.hpp"
#include "ang16.hpp"
#include "car_despawn_status.hpp"
#include "car_flags.hpp"
#include "fix16.hpp"
#include "gtx_0x106C.hpp"
#include "map_0x370.hpp"
#include "miss2_xyz.hpp"
#include "sprite.hpp"
#include <wchar.h>

struct gmp_zone_info;
struct SCR_THREAD;
class Camera_0xBC;
class Object_2C;
class Fix16_Rect;
class CarAI_78;
class CarPhysics_B0;
class Trailer;
class Ped;
class Char_8;
class CarChaseTask_40;
class Ped_List_4;
class car_info;
class SoundObject_10;
class Gang_144;
class Player;

EXTERN_GLOBAL(Fix16, kFP16One_6777D0);

Ang16 ReturnAngleFromRoadDirection_4F7940(s32* road_direction);

namespace car_kind
{
enum
{
    none_0 = 0,
    recycled_1 = 1,
    proto_recycled_2 = 2,
    Unknown_3 = 3,
    paramedic_car_4 = 4,
    firefighter_5 = 5,
    police_6 = 6,
    roadblock_car_7 = 7,
    mission_car_8 = 8,
    parked_car_9 = 9,
    Unknown_10 = 10,
};
} // namespace car_kind

namespace car_engine_status
{
enum
{
    off_1 = 1,
    unknown_2 = 2,
    on_3 = 3,
    unknown_4 = 4,
    unknown_5 = 5,
    destroyed_6 = 6,
    crushed_7 = 7,
};
} // namespace car_engine_status

struct car_rng_list
{
    car_rng_list(Fix16 unk)
    {
        this->unknown = unk;
    }

    Fix16 unknown;
    s32 values[256];
};

class CarThreadTrigger_18
{
  public:
    sptr field_0_pScriptCmd; // SCR_THREAD*, then read back as the command it waits on
    Object_2C* field_4_O2C;
    s32 field_8_type;
    s32 field_C;
    u8 field_10_remap_rng;
    u8 field_11_pad[3];
    s32 field_14_enable_state;
};

class CarThreadTriggers_214
{
  public:
    void FreeTrigger_5C8680(u8 idx);
    char_type AddThreadTrigger_5C86C0(const s32& type,
                                      const s32& param,
                                      SCR_THREAD* pScriptThread,
                                      Fix16 xpos,
                                      Fix16 ypos,
                                      Fix16 zpos,
                                      Fix16 width,
                                      Fix16 height);
    void Reset_5C8750();
    void CheckThreadTrigger_5C8780(u8 idx, Sprite* pSprite);

    // 9.6f 0x4768C0
    inline void SetTriggerEnableState_4768C0(u8 idx, s32 state)
    {
        field_0_triggers[idx].field_14_enable_state = state;
    }

    CarThreadTrigger_18 field_0_triggers[22];
    s16 field_210_count;
    s16 field_212_unused;
};

EXTERN_GLOBAL(CarThreadTriggers_214*, gCarThreadTriggers_214_705F20);
EXTERN_GLOBAL_ARRAY(u16, gRngRemapTable_679320, 1000);

class CarRemapRng_2
{
  public:
    CarRemapRng_2();
    ~CarRemapRng_2() = default;
    void IncNextRngRemapIdx_47BD90();

    u16 GetRngValue_4212D0()
    {
        return gRngRemapTable_679320[this->field_0];
    }

    u16 field_0;
};

EXTERN_GLOBAL(Fix16, kFpHalf_6F7690);

EXTERN_GLOBAL(Fix16, kFpFive_6F77D4);

EXTERN_GLOBAL(Ang16, kAngZero_6F804C);

EXTERN_GLOBAL(Ang16, kAng180_6F771E);

EXTERN_GLOBAL(Fix16, kFpZero_6F77C0);
EXTERN_GLOBAL(Fix16, kFpOne_6F77C4);
EXTERN_GLOBAL(Fix16, kFpOne_679E74);

class CarManager_6C
{
  public:
    // 9.6f 0x421960
    inline Ped* GetSpareDriver_421960()
    {
        return field_4_unused;
    }

    // 9.6f 0x421970
    inline void SetSpareDriver_421970(Ped* pPed)
    {
        field_4_unused = pPed;
    }

    void ToggleTrafficLightState_4C39F0()
    {
        if (field_20_light_state_a != 4)
        {
            field_20_light_state_a = 4;
        }
        else
        {
            field_20_light_state_a = 3;
        }

        if (field_24_light_state_b != 1)
        {
            field_24_light_state_b = 1;
        }
        else
        {
            field_24_light_state_b = 2;
        }
    }

    void DistributeCarsByRating_444980();
    u32 SelectTrafficCarModel_444AB0(Player* pPlayer, gmp_zone_info* pZoneInfo, Fix16 density_scale, u16* pOut);
    char_type NoRefs_444CE0(s32 a1, s32 a2, s32 a3, s32 a4, s32 a5);
    Car_BC* SpawnCarAtRoadDirection_444CF0(s32 car_model_type, Fix16 xpos, Fix16 ypos, Fix16 zpos);
    Car_BC* GetNearestCarFromCoord_444F80(Fix16 x, Fix16 y, Fix16 z, Ped* pPed);
    Car_BC* GetNearestEnterableCarFromCoord_444FA0(Fix16 x, Fix16 y, Fix16 z, Ped* pPed);
    Car_BC* DoGetNearestCarFromCoord_444FC0(Fix16 xpos,
                                            Fix16 ypos,
                                            Fix16 zpos,
                                            s32 bMatchDriverless,
                                            Ped* pPed,
                                            char_type bIgnorePedRestrictions);

    Car_BC* GetNearestFrontVehicle_445210(Sprite* pSprite, u8 k3);
    Car_BC* SpawnBusAtValidRoadPosition_4453E0(Fix16 x, Fix16 y, s32 side, const s32& car_model);
    Car_BC* SpawnCarOnRoadNetwork_4458B0(Fix16 xpos, Fix16 ypos, s32 road_direction, s32 car_model_type);
    Car_BC* SpawnCarAt_446230(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, s32 car_info_idx, Fix16 maybe_w_scale);
    Trailer* SpawnCabAndTrailer_446530(Fix16 xpos, Fix16 ypos, Ang16 rotation, s32 car_idx, s32 trailer_idx);
    void RemoveFromPoolAndCollision_446730(Car_BC* pCar);

    void DecrementAllocatedCarType_4466C0(s32 car_kind);
    void EnableFreeShoppingIfAllKFsPassed_446760();
    void CarsService_446790();
    bool CanAllocateCarOfKindWithSpare_446870(s32 type);
    bool CanAllocateCarOfKind_446930(s32 type);
    CarManager_6C();
    ~CarManager_6C();

    // 9.6f inlined
    inline Car_BC* SpawnCar_426E10(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, s32 car_info_idx)
    {
        return SpawnCarAt_446230(xpos, ypos, zpos, rotation, car_info_idx, kFpOne_6F77C4);
    }

    // SpawnCar_426E10 with the scale passed in: 10.5 loads a different Fix16(1) global per TU
    // (kFpOne_6F77C4 in miss2_0x11C, kOne_6FE614 in Player)
    inline Car_BC* SpawnCar_426E10(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, s32 car_info_idx, const Fix16& scale)
    {
        return SpawnCarAt_446230(xpos, ypos, zpos, rotation, car_info_idx, scale);
    }

    // 9.6f inlined
    inline Car_BC* SpawnCar_4764A0(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, s32 car_info_idx)
    {
        return SpawnCarAt_446230(xpos, ypos, zpos, rotation, car_info_idx, kFpHalf_6F7690);
    }

    inline Car_BC* SpawnCar_426E10_v2(Fix16 xpos, Fix16 ypos, Fix16 zpos, Ang16 rotation, s32 car_info_idx)
    {
        return SpawnCarAt_446230(xpos, ypos, zpos, rotation, car_info_idx, kFP16One_6777D0);
    }

    // unknown inlined function
    inline Car_BC* SpawnCar_shortened(s32 car_info_idx)
    {
        return SpawnCarAt_446230(kFpFive_6F77D4, kFpFive_6F77D4, kFpZero_6F77C0, kAngZero_6F804C, car_info_idx, kFpOne_6F77C4);
    }

    // As SpawnCarAtCorrectZ_426E40 with the scale passed in (PublicTransport_181C::SpawnTrainsFromStations_578860)
    inline Car_BC* SpawnCarAtCorrectZ_Scaled(Fix16 xpos, Fix16 ypos, const Ang16& rotation, s32 car_model, const Fix16& scale)
    {
        if (car_model == car_model_enum::TRAIN || car_model == car_model_enum::TRAINCAB || car_model == car_model_enum::TRAINFB ||
            car_model == car_model_enum::boxcar)
        {
            return SpawnCarAt_446230(xpos, ypos, gMap_0x370_6F6268->GetRailwayZCoordAtXY_4E6510(xpos, ypos), rotation, car_model, scale);
        }
        else
        {
            return SpawnCarAt_446230(xpos, ypos, gMap_0x370_6F6268->FindGroundZForCoord_4E5B60(xpos, ypos), rotation, car_model, scale);
        }
    }

    // As SpawnCarAtCorrectZ_426E40 but with the scale passed in (TrafficSpawner_14::SpawnTrafficCar_582480 passes its own kFpOne_6FF778)
    inline Car_BC* SpawnCarAtCorrectZ_WithScale(Fix16 xpos, Fix16 ypos, Ang16 rotation, s32 car_model, const Fix16& scale)
    {
        if (car_model == car_model_enum::TRAIN || car_model == car_model_enum::TRAINCAB || car_model == car_model_enum::TRAINFB ||
            car_model == car_model_enum::boxcar)
        {
            return SpawnCarAt_446230(xpos, ypos, gMap_0x370_6F6268->GetRailwayZCoordAtXY_4E6510(xpos, ypos), rotation, car_model, scale);
        }
        else
        {
            return SpawnCarAt_446230(xpos, ypos, gMap_0x370_6F6268->FindGroundZForCoord_4E5B60(xpos, ypos), rotation, car_model, scale);
        }
    }

    inline Car_BC* SpawnCarAtCorrectZ_426E40(Fix16 xpos, Fix16 ypos, Ang16 rotation, s32 car_model)
    {
        if (car_model == car_model_enum::TRAIN || car_model == car_model_enum::TRAINCAB || car_model == car_model_enum::TRAINFB ||
            car_model == car_model_enum::boxcar)
        {
            return SpawnCarAt_446230(xpos,
                                     ypos,
                                     gMap_0x370_6F6268->GetRailwayZCoordAtXY_4E6510(xpos, ypos),
                                     rotation,
                                     car_model,
                                     kFP16One_6777D0);
        }
        else
        {
            return SpawnCarAt_446230(xpos,
                                     ypos,
                                     gMap_0x370_6F6268->FindGroundZForCoord_4E5B60(xpos, ypos),
                                     rotation,
                                     car_model,
                                     kFP16One_6777D0);
        }
    }

    CarRemapRng_2 field_0;
    s16 field_2_unused;
    Ped* field_4_unused;
    char_type field_8_unused;
    u8 field_9_good_car_idx;
    u8 field_A_bad_car_idx;
    u8 field_B_average_car_idx;
    s32 field_C_suppressed_model;
    CarRemapRng_2 field_10_remap_rng;
    CarRemapRng_2 field_12_unused;
    s32 field_14_next_car_id;
    char_type field_18_unused;
    char_type field_19_unused;
    u8 field_1A_fire_effect_cycle;
    char_type field_1B_unused;
    s32 field_1C_explosion_offset_cycle;
    s32 field_20_light_state_a;
    s32 field_24_light_state_b;
    s32 field_28_recycled_cars;
    s32 field_2C_paramedic_cars;
    s32 field_30_firefighter_cars;
    s32 field_34_unit_cars;
    s32 field_38_roadblock_cars;
    s32 field_3C_mission_cars;
    s32 field_40_proto_recycled_cars;
    s32 field_44_parked_cars;
    s32 field_48_kind10_cars;
    Fix16_Point field_4C_tv_van_dir;
    u8 field_54_spawn_block;
    char_type field_55_visible_cars_count;
    char_type field_56_unused;
    char_type field_57_unused;
    s32 field_58_model_to_check_destroy;
    char_type field_5C_model_check_destroyed;
    char_type field_5D_unused;
    char_type field_5E_unused;
    char_type field_5F_unused;
    s32 field_60_impact_mode;
    Fix16 field_64_zpos;
    char_type field_68_object_hit_active;
    char_type field_69_do_free_shopping;
    char_type field_6A_unused;
    char_type field_6B_unused;
};

EXTERN_GLOBAL(CarManager_6C*, gCarManager_6C_677930);

class Trailer
{
  public:
    Ped* GetTruckCabDriver_407B80();
    Car_BC* GetCabOrLoadedCar_407B90(Car_BC* pCar);
    void SetTruckCabAndTrailerCar_407BB0(Car_BC* pTruckCab, Car_BC* pTrailerCar);
    Fix16_Point GetHitchPosition_407BD0();
    void UpdateTrailerAlignment_407CE0();
    char_type ServicePhysics_408140();
    void SetupCarPhysicsAndSpriteBinding_408190();
    void DeAllocateCarPhysics_4081B0();
    char_type ExplodeBothIfOneDestroyed_4081D0();
    s32 Update_408220();

    void PoolAllocate()
    {
    }

    void PoolDeallocate()
    {
    }

    // 9.6f 0x49EFB0
    inline char_type IsHitchAngleClamped_49EFB0()
    {
        return field_0_hitch_angle_clamped;
    }

    //Inlined in CarManager_6C constructor 9.6f -> 0x4212d0
    inline Trailer()
    {
        mpNext = nullptr;
        field_8_truck_cab = nullptr;
        field_C_pCarOnTrailer = nullptr;
        field_0_hitch_angle_clamped = 0;
    }

    char_type
        field_0_hitch_angle_clamped; // set by UpdateTrailerAlignment_407CE0 when the trailer angle hits its window, cleared on hitching
    char_type field_1_unused;
    char_type field_2_unused;
    char_type field_3_unused;
    Trailer* mpNext;
    Car_BC* field_8_truck_cab;
    Car_BC* field_C_pCarOnTrailer;
};

static inline bool IsArmyModel(s32 idx1)
{
    return idx1 == car_model_enum::TANK || idx1 == 22 || idx1 == 30 || idx1 == 3;
}

class Car_BC
{
  public:
    // 9.6f 0x42AC70
    inline void SetTurnRight_42AC70()
    {
        field_A6_bf.bTurnRight = 1;
    }

    // 9.6f 0x42AC80
    inline void SetTurnLeft_42AC80()
    {
        field_A6_bf.bTurnLeft = 1;
    }

    // 9.6f 0x42AC90
    inline void SetTurnRightReady_42AC90()
    {
        field_A6_bf.bTurnRightReady = 1;
    }

    // 9.6f 0x421530
    inline void SetTurnLeftReady_421530()
    {
        field_A6_bf.bTurnLeftReady = 1;
    }

    // 9.6f 0x421540
    inline void SetAIHalted_421540()
    {
        field_A6_bf.bAIHalted = 1;
    }

    // 9.6f 0x421550
    inline void ClearAIHalted_421550()
    {
        field_A6_bf.bAIHalted = 0;
    }

    // 9.6f 0x42ACA0
    inline void ClearTurnRight_42ACA0()
    {
        field_A6_bf.bTurnRight = 0;
    }

    // 9.6f 0x42ACB0
    inline void ClearTurnLeft_42ACB0()
    {
        field_A6_bf.bTurnLeft = 0;
    }

    // 9.6f 0x42AC60
    inline bool IsAIHalted_42AC60()
    {
        return field_A6_bf.bAIHalted == 1;
    }

    // 9.6f 0x42AC20
    inline bool IsTurnRight_42AC20()
    {
        return field_A6_bf.bTurnRight == 1;
    }

    // 9.6f 0x42AC30
    inline bool IsTurnLeft_42AC30()
    {
        return field_A6_bf.bTurnLeft == 1;
    }

    // 9.6f 0x42AC40
    inline bool IsTurnRightReady_42AC40()
    {
        return field_A6_bf.bTurnRightReady == 1;
    }

    // 9.6f 0x42AC50
    inline bool IsTurnLeftReady_42AC50()
    {
        return field_A6_bf.bTurnLeftReady == 1;
    }

    // 9.6f 0x4118C0
    inline bool IsEngineOn_4118C0()
    {
        return field_9C_engine_status == car_engine_status::on_3;
    }

    // 9.6f 0x421610
    inline bool IsTrain_421610()
    {
        return field_84_car_info_idx == car_model_enum::TRAIN;
    }

    // 9.6f 0x421950
    inline void SetWeaponKind_421950(s32 v)
    {
        field_B4_weapon_kind = v;
    }

    // 9.6f 0x4A51B0
    inline void SetDamage_4A51B0(s16 v)
    {
        field_74_damage = v;
    }

    bool CanBeEnteredByPed_4451E0(Ped* pPed);
    bool IsEnterable_445360();
    Ang16 GetCornerAngle_4403A0();

    s16 ApplyImpactDamage_43D5D0(Fix16 damage);

    bool IsPoliceCar_439EC0();
    Fix16 GetDamageFactorOnSpeed_439EE0();
    Fix16 GetMaxSpeed_439F30();
    wchar_t* GetCarStr_439F80();
    Fix16_Point GetHitchPoint_439FB0();
    Fix16_Point GetRearWheelOffset_43A0E0();
    Fix16 GetMass_43A120();
    Fix16 GetAntiStrength_43A1D0();
    bool IsBus_43A1F0();
    bool HasSpriteZoom_43A230();
    Fix16 GetCarLinearSpeed_43A240();
    bool IsCarInAir_43A3C0();
    Ang16 GetOrientationAngle_43A3E0();
    Fix16_Point GetLinearVelocity_43A450();
    Fix16 GetVelocity_43A4C0();
    Fix16 GetMomentOfInertia_43A590();
    Fix16 GetMinDimension_43A5B0();
    void RemoveAllDamage();
    bool AllowResprayOrPlates();
    bool IsNotCurrentRemap(u8 remap);
    bool IsNotCurrentRemapOfCarAndTrailerCar(u8 remap);
    void SetCarRemap(u8 remap);
    void AssignRandomRemap_43A7D0();
    char_type GetCarModelForPhysics_43A850();
    void DoBrake_43A950();
    void DoBrakeAndHandbrake_43A970();
    void SetDriver(Ped* pNewDriver);
    void ApplyVisualDamage_43A9F0();
    void SetDummyControl_43AA20();
    void StartLightFlashing_43CAC0();
    void Deactivate_43AA60();
    char_type IsThreatToSearchingPed_43AAE0();
    char_type CanCarCollideWithSprite_43AAF0(Sprite* pSprite);
    void ProcessCarToCarImpact_43ADC0(Sprite* pSprite);
    bool CanExitCar_43AF10();
    void ResumeAIDriving_43AF40();
    void HaltAIDriving_43AF60();
    char_type IsDoorAccessible_43AFE0(u8 target_door);
    bool IsStoppedWithPavementAtDoor_43B140(u8 target_car_door);
    bool IsDoorLockedForPed_43B2B0(Ped* pPed);
    Car_Door_10* GetDoor(u8 door_idx);
    char_type GetRemap();
    void OpenAllDoors_43B380();
    void CloseAllDoors_43B3D0();
    void GetDoorWorldPos_43B420(u8 door_idx, Fix16* pOutX, Fix16* pOutY);
    bool IsDoorOffsetLarge_43B540(u8 targetDoor);
    void GetDoorWorldPosition_43B5A0(u8 targetDoor, Fix16* pOutX, Fix16* pOutY);
    char_type IsOnScreenForAnyPlayer_43B730();
    char_type IsVisibleToAnyPlayer_43B750();
    void AssignKillerToOccupants_43B770();
    void AssignDriverBlameForExplosion_43B7B0(Car_BC* pCar);
    bool IsImmuneToExplosionType_43B850(s32 explosion_type);
    void SpawnDamageFireEffect_43B870(s32 fire_size, const Fix16_Point* pPos);
    // Called without this in SpawnDamageFireEffect_43B870: a static
    static s32 GetFireExplosionType_43BB90(u8 fire_level);
    void SpawnFire_43BBC0();
    void SetupCarPhysicsAndSpriteBinding_43BC30();
    void SetupCarPhysicsAndSpriteBinding_43BCA0();
    void DeAllocateCarPhysics_43BD00();
    void FlashHeadlights_43BD40();
    void BrakeLightsOn_43BF10();
    void BrakeLightsOff_43BF70();
    void HeadlightsOn_43BFE0();
    void HeadlightsOff_43C0C0();
    void PrepareForExplosion_43C1C0();
    void UpdateTopRightRoofLight_43C260();
    void ResetTopRightRoofLight_43C310();
    void UpdateTopLeftRoofLight_43C3C0();
    void ResetTopLeftRoofLight_43C470();
    void UpdateRoofLights_43C500();
    void ResetRoofLights_43C650();
    void UpdateBottomLeftRoofLight_43C700();
    void ResetBottomLeftRoofLight_43C840();
    void ActivateEmergencyLights_43C920();
    void DeactivateEmergencyLights_43C9D0();
    void SyncEmergencyLightState_43CA80();
    void StopLightFlashing_43CBE0();
    void SetCarGraphic_43CDF0(char_type graphic_idx);
    void DamageArea_43CF30(s32 damage_area);
    bool IsAreaDamaged_43D1C0(s32 damage_area);
    void TryDamageArea_43D2C0(u8 damage_area, s32 damageAmount);
    void RepairDamage_43D400();
    void EmitExplosion_43D690(s32 type, Fix16 x, Fix16 y);
    void TriggerExplosion_43D7B0(s32 type);
    void HandleCarExplosion_43D840(s32 type);
    s16 AccumulateDamage_43DA90(s16 damage, const Fix16_Point* pVec);
    // HandleCarHitByObject_43F130 passes the operator temporaries straight in (push %eax of the result)
    inline s16 AccumulateDamage_43DA90(s16 damage, const Fix16_Point& vec)
    {
        return AccumulateDamage_43DA90(damage, &vec);
    }
    void KillContainedPeds_43DB80();
    void StopMovement_43DBD0();
    bool IsSpriteShrunk_43DC00();
    bool CarShrinkSprite_43DC80(s32 xoff, s32 yoff);
    bool IsBeingCrushed_43DD50();
    void DestroyWhileSinking_43DD60();
    char_type ManageDrowning_43E560();
    Car_BC* GetCabOrSelf_43E8D0();
    Ped* GetEffectiveDriver_43E990();
    char_type OnObjectTouched_43EA60(Object_2C* pObj);
    char_type HandleCarHitByObject_43F130(Object_2C* pObj);
    Fix16 GetMaxTurnRate_440510();
    char_type GetPassengersCount_440570();
    void InitCarAIControl_440590();
    void SpawnDriverPed();
    void AddGangDriver_440630(Gang_144* pGang);
    void AttachGangIcon_440660(u8 arrow_colour);
    void ShowCarName_4406B0(Ped* pPed);
    void AssignDriver_4406E0(Ped* pPed);
    void ClearDriver_4407F0();
    Sprite* GetSprite_440840();
    void PutWaterCannonOnRoof_440AC0();
    void PutTankCannonOnRoof_440B10();
    void PutMachineGunOnRoof_440B60();
    void PutTV_Antenna_440BB0();
    char_type RotateRoofObjectTowardTarget_440C10(Ang16 targetAngle);
    char_type HandleRoofTurretRotation_440D90(char_type left_on);
    void FireCarBomb_440F90(char_type instant_bomb);
    void GoToBlockTest_441030(u8 x, u8 y, u8 z, s32 direction);
    void GotoBlock_441080(u8 x, u8 y, u8 z, s32 direction);
    char_type CountConsecutiveArrowBlocks_4410D0(Ang16 ang, s8* pOutCount, Fix16 sprite_x, Fix16 sprite_y);
    Fix16 GetZPos_441330();
    void CountDownToWreck_441360();
    void TurnToWreck_4436A0();
    void StartWreckCountdown_441380();
    void UpdateTrainCarriagesOnTrack_4413B0(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    void UpdateEngineStatus_441520();
    void UpdateBrakeLights_4415C0();
    Fix16_Rect GetOrientedHalfExtentsRect_441600();
    void TryHonkHorn_4416D0(s32 horn_mode);
    void UpdateHorn_4417D0();
    void HonkHorn_4417F0();
    void HandleSpecialInput_441800(char_type now_special_pressed);
    void DoDetachTrailer_4418A0();
    void DetachTrailerAndUpdateDamage_4418B0();
    void HandleUserInput_4418D0(char_type forward_gas_on,
                                char_type foot_brake_on,
                                char_type left_on,
                                char_type right_on,
                                char_type hand_brake_on,
                                char_type now_special_pressed,
                                char_type was_special_pressed,
                                char_type attack);
    void AllocCarPhysics_4419E0();
    void DeAllocateCarPhysics_441A10();
    char_type AreAllDoorsClosed_441A40();
    void InitDoors_441A70();
    void ServiceDoors_441B00();
    void UpdateTaxiLight_441B20();
    void UpdateRoofLightFlasher_441B50();
    void UpdatePopupHeadlights_441C00();
    void UpdateFbiPopupSiren_441D40();
    void UpdateLights_441E70();
    bool IsCarInConditionsToDespawn_442170();
    void ServicePhysics_442190();
    char_type IsMissionRelated_4421B0();
    bool CanDespawn_442200();
    void ManageDespawning_442310();
    char_type UpdateCarDespawnStatus_4424C0();
    Ang16 GetRadioTowerAngle_442520();
    void ManageTVAntenna_4425D0();
    void ServiceDoorsLightsAndHorn_4426D0();
    void DetachTrailer_442760();
    void AttachTrailer_4427A0(Car_BC* pToFind);
    void TryHitchTrailer_442810();
    void LightUpdate_442D10();
    char_type TrainUpdate_442D70();
    char_type TrailerUpdate_443130();
    char_type PoolUpdate();
    void UpdateOnCraneHook_443330();
    bool UpdateAttachedToSprite_443360(Sprite* pSprite, Fix16 x, Fix16 y, Ang16 rot);
    void InitDoors_4435A0();
    s32 GetCrashSoundCategory_4435B0();
    void UpdateSpriteNum_4435F0();
    void ApplyExplosionImpulse_443710(Fix16_Point* xy);
    static s32 GetCarWeaponCost_443A50(s32 weapon_kind);
    void BuyCarWeapon_4438C0(s32 weapon_kind);
    static void ShowCantAffordMessage_443AB0(Player* pPlayer, s32 weapon_cost);
    void ResprayOrChangePlates(u8 remap);
    void ResprayOrCleanPlates(u8 remap);
    void HandleShops_443C40(Object_2C* pObj);
    void SetPosition_443D00(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    void IncrementCarStats_443D70(s32 car_kind);
    void IncrementAllocatedCarType_443DA0(s32 car_kind);
    void MarkProtoRecycled_443E50();
    void MarkRecycled_443E80();
    void ReassignCarStats_443EB0(s32 car_kind);
    void ReassignAllocatedCarType_443EE0(s32 car_kind);
    void AttachLight_443F30(s32 object_type, s32 argb, s32 x_offset, s32 y_offset);
    void UpdateExploderTimer_444020();
    u32 GetEffectiveDriverPedId_444090();
    void SetSirens_4441B0();
    void PoolAllocate();
    void DeAllocateAI_4446E0();
    void PoolDeallocate();

    Car_BC(); // 0x444860
    ~Car_BC(); // 0x444960

    void ClearHeadlightDeltas_447360();
    bool IsDrivenByNonPlayer_564300();

    // 9.6f 0x411930
    inline bool IsFlagSet_411930(u16 flag)
    {
        if (field_78_flags & flag)
        {
            return true;
        }
        return false;
    }

    // 9.6f 0x425770
    inline bool IsFlameproof_425770()
    {
        return IsFlagSet_411930(car_flag::Flameproof);
    }

    // FUNCTION: 96f 0x4118d0
    s32 IsDrivenByPlayer_4118D0() const
    {
        if (field_54_driver != nullptr && field_54_driver->field_15C_player != nullptr)
        {
            return true;
        }
        return false;
    }

    // Inlined 0x421720
    bool IsOnTrailer_421720()
    {
        // We are on the trailer
        return field_64_pTrailer && field_64_pTrailer->field_C_pCarOnTrailer == this;
    }

    Ped* GetDriver_4118B0()
    {
        return field_54_driver;
    }

    // Inlined 0x41E460
    bool IsTrailerCab_41E460()
    {
        // We are the cab of the trailer
        return field_64_pTrailer && field_64_pTrailer->field_8_truck_cab == this;
    }

    // Inlined 0x475E60
    bool HasOtherCarOnTrailer_475E60(Car_BC* pExcludeCar)
    {
        // A car is on the trailer but must not be pExcludeCar
        return field_64_pTrailer && field_64_pTrailer->field_C_pCarOnTrailer == pExcludeCar && this != pExcludeCar;
    }

    bool IsCargoCarOf_4BA390(Car_BC* pOther) const
    {
        return field_64_pTrailer && field_64_pTrailer->field_8_truck_cab == pOther && this != pOther;
    }

    inline bool IsWithinArea(SCR_Rect_f* rect)
    {
        Fix16 x_pos = field_50_car_sprite->field_14_xy.x;
        Fix16 width = rect->field_C_size.field_0_x;
        Fix16 y_pos, z_pos;
        Fix16 height;
        Fix16 z_target;
        return (
            x_pos >= rect->field_0_pos.field_0_x - width && x_pos <= rect->field_0_pos.field_0_x + width &&
            (y_pos = field_50_car_sprite->field_14_xy.y,
             height = rect->field_C_size.field_4_y,
             y_pos >= rect->field_0_pos.field_4_y - height) &&
            field_50_car_sprite->field_14_xy.y <= rect->field_0_pos.field_4_y + height &&
            (z_pos = field_50_car_sprite->field_1C_zpos, z_target = rect->field_0_pos.field_8_z, z_pos.ToUInt8() == z_target.ToUInt8()));
    }

    inline bool IsWithinBlock(SCR_XYZ_f* pos)
    {
        Sprite* pSprite = field_50_car_sprite;
        Fix16 car_z_pos;
        return (pSprite->field_14_xy.x >= pos->field_0_x.ToUInt8() && pSprite->field_14_xy.x < (pos->field_0_x.ToUInt8() + 1)

                && pSprite->field_14_xy.y >= pos->field_4_y.ToUInt8() && pSprite->field_14_xy.y < (pos->field_4_y.ToUInt8() + 1)

                && (car_z_pos = pSprite->field_1C_zpos, car_z_pos.ToUInt8() == pos->field_8_z.ToUInt8()));
    }

    bool IsTrainModel_403BA0()
    {
        return field_84_car_info_idx == car_model_enum::TRAIN || field_84_car_info_idx == car_model_enum::TRAINCAB ||
            field_84_car_info_idx == car_model_enum::TRAINFB || field_84_car_info_idx == car_model_enum::boxcar;
    }

    // 9.6f inline 0x421560
    void SetDriverPedType_421560(s32 driver_ped_type)
    {
        field_7C_ped_type = driver_ped_type;
        field_76_last_seen_timer = 0;
    }

    // TODO: Find 9.6f addr
    inline bool HasEmergencyInfoFlag()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::emergency) ==
            car_info_flag::emergency;
    }

    bool IsMaxDamage_40F890()
    {
        return field_74_damage == 32001;
    }

    inline bool AreEmergencyLightsOn_414F80()
    {
        return HasEmergencyLights_414F20() && field_A4_bf.bEmergencyLights;
    }

    // 9.6f 0x4119D0
    inline s32 TakeWeaponKind_4119D0()
    {
        s32 ret = field_B4_weapon_kind;
        field_B4_weapon_kind = 0;
        return ret;
    }

    // 9.6f 0x411950
    inline s32 TakeSoundEvent_411950()
    {
        s32 ret = field_AC_sound_event;
        field_AC_sound_event = 0;
        return ret;
    }

    // FUNCTION: 96f 0x411940
    inline s32 GetCarInfoIdx_411940()
    {
        return field_84_car_info_idx;
    }

    bool HasEmergencyLights_414F20()
    {
        return HasEmergencyInfoFlag() || IsFbiCar_411920();
    }

    // 9.6f inline 0x421700
    inline bool HasPopupHeadlights_421700()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags_2 & car_info_flag2::popup_headlights) ==
            car_info_flag2::popup_headlights;
    }

    // 9.6f inline 0x421660
    inline bool HasRoofLight_421660()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::roof_light) ==
            car_info_flag::roof_light;
    }

    inline bool IsTrailerInfo_421640()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::trailer) ==
            car_info_flag::trailer;
    }

    // 9.6f inline 0x4216C0
    inline bool HasTaxiLight_4216C0()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::taxi_light) ==
            car_info_flag::taxi_light;
    }

    // 9.6f inline 0x421680
    inline bool HasBottomLeftRoofLight_421680()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::bottom_roof_light) ==
            car_info_flag::bottom_roof_light;
    }

    // 9.6f inline 0x4216A0
    inline bool HasTopRoofLights_4216A0()
    {
        return (gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->info_flags & car_info_flag::top_roof_lights) ==
            car_info_flag::top_roof_lights;
    }

    void SetBottomLeftRoofLightDelta_421810()
    {
        field_8_damaged_areas.set_bit(CarDeltaBitsEnum::BottomLeftRoofLight_15);
    }

    void SetBulletproof_4218C0()
    {
        AddFlags_421890(car_flag::Bulletproof);
    }

    void SetFlameproof_4218D0()
    {
        AddFlags_421890(car_flag::Flameproof);
    }

    // 9.6f 0x44A3E0
    void SetNoCollide_44A3E0()
    {
        AddFlags_421890(car_flag::NoCollide);
    }

    // 9.6f 0x475C10
    inline void LockUnlessPermanent_475C10()
    {
        if (field_98_door_lock != Car_Door_Lock::locked_permanently_4)
        {
            field_98_door_lock = Car_Door_Lock::locked_1;
        }
    }

    // 9.6f 0x476360
    inline void SetCarThreadPending_476360()
    {
        field_8D_car_thread_flags |= 1;
    }

    // 9.6f 0x475C40
    inline void LockoutThiefUnlessPermanent_475C40()
    {
        if (field_98_door_lock != Car_Door_Lock::locked_permanently_4)
        {
            field_98_door_lock = Car_Door_Lock::lockout_thief_2;
        }
    }

    // 9.6f 0x476270
    inline void SetFlag4_476270()
    {
        AddFlags_421890(car_flag::Flag4);
    }

    // 9.6f 0x475C80
    inline void UnlockUnlessPermanent_475C80()
    {
        if (field_98_door_lock != Car_Door_Lock::locked_permanently_4)
        {
            field_98_door_lock = Car_Door_Lock::unlocked_3;
        }
    }

    // 9.6f 0x475C30
    inline void LockPermanently_475C30()
    {
        field_98_door_lock = Car_Door_Lock::locked_permanently_4;
    }

    // 9.6f 0x421490
    inline void SetDespawn3IfNot5_421490()
    {
        if (field_88_despawn_status != car_despawn_status::despawning_5)
        {
            field_88_despawn_status = car_despawn_status::despawn_soon_3;
        }
    }

    void SetFlag3_4218A0()
    {
        AddFlags_421890(car_flag::Flag3);
    }

    void SetFlag6_4218B0()
    {
        AddFlags_421890(car_flag::Flag6);
    }

    inline s32 GetCarKind_4343B0()
    {
        return field_A0_car_kind;
    }

    // 9.6f inline 0x425650
    inline void ClearHeadlightDeltas_425650()
    {
        if (HasPopupHeadlights_421700())
        {
            // clear the popup headlight door frames
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopRightDoor1_11);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopRightDoor2_12);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopRightDoor3_13);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopRightDoor4_14);

            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopLeftDoor1_28);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopLeftDoor2_29);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopLeftDoor3_30);
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::TopLeftDoor4_31);
        }
        field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::FrontRightHeadlight_6); // clear left front headlight
        field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::FrontLeftHeadlight_23); // clear right front headlight
        if (HasRoofLight_421660())
        {
            field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::BottomLeftRoofLight_15); // clear right siren light or roof light
        }
    }

    // 9.6f inline 0x421430
    inline void ClearBrakeLightDeltas_421430()
    {
        field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::BackRightBrakeLight_5); // clear left rear brake light
        field_8_damaged_areas.clear_bit(CarDeltaBitsEnum::BackLeftBrakeLight_22); // clear right rear brake light
    }

    // 9.6f inline 0x4213D0
    inline void SetBrakeLightDeltas_4213D0()
    {
        if (!field_8_damaged_areas.mask_bit(CarDeltaBitsEnum::TopRightDamage_1)) // if not rear left damage
        {
            field_8_damaged_areas.set_bit(CarDeltaBitsEnum::BackRightBrakeLight_5); // set left rear brake light
        }
        if (!field_8_damaged_areas.mask_bit(CarDeltaBitsEnum::TopLeftDamage_0)) // if not rear right damage
        {
            field_8_damaged_areas.set_bit(CarDeltaBitsEnum::BackLeftBrakeLight_22); // set right rear brake light
        }
    }

    // 9.6f inline 0x425590
    inline void SetHeadlightDeltas_425590()
    {
        if (!field_8_damaged_areas.mask_bit(CarDeltaBitsEnum::BottomRightDamage_2)) // if not front left damage
        {
            if (HasPopupHeadlights_421700())
            {
                field_8_damaged_areas.set_bit(CarDeltaBitsEnum::TopRightDoor1_11); // set left rear door open/close frame 1 ????
            }
            else
            {
                field_8_damaged_areas.set_bit(CarDeltaBitsEnum::FrontRightHeadlight_6); // set left front headlight
            }
        }

        if (!field_8_damaged_areas.mask_bit(CarDeltaBitsEnum::BottomLeftDamage_3)) // if not front right damage
        {
            if (HasPopupHeadlights_421700())
            {
                field_8_damaged_areas.set_bit(CarDeltaBitsEnum::TopLeftDoor1_28); // set right rear door open/close frame 1 ????
            }
            else
            {
                field_8_damaged_areas.set_bit(CarDeltaBitsEnum::FrontLeftHeadlight_23); // set right front headlight
            }
        }
        if (HasRoofLight_421660())
        {
            field_8_damaged_areas.set_bit(CarDeltaBitsEnum::BottomLeftRoofLight_15); // set right siren light or roof light
        }
    }

    // 0x48A930
    Fix16 GetCarWidth_48A930()
    {
        return field_50_car_sprite->GetWidth_420740();
    }

    // 0x48A950
    Fix16 GetCarHeight_48A950()
    {
        return field_50_car_sprite->GetHeight_447E70();
    }

    bool IsMarkedForDespawn_4214B0()
    {
        return field_88_despawn_status == car_despawn_status::despawn_pending_2 ||
            field_88_despawn_status == car_despawn_status::marked_for_despawn_4 ||
            field_88_despawn_status == car_despawn_status::despawn_soon_3;
    }

    // 9.6f 0x421870
    inline Player* GetDriverPlayer_421870()
    {
        return field_54_driver->field_15C_player;
    }

    // 9.6f 0x421460
    inline void ClearHorn_421460()
    {
        field_A7_horn = 0;
    }

    // 9.6f 0x4215F0
    inline void CancelDespawn_4215F0()
    {
        if (field_88_despawn_status == car_despawn_status::despawn_pending_2 ||
            field_88_despawn_status == car_despawn_status::marked_for_despawn_4 ||
            field_88_despawn_status == car_despawn_status::despawn_soon_3)
        {
            field_88_despawn_status = car_despawn_status::active_1;
        }
    }

    // 9.6f 0x4218F0
    inline s32 IsCarThreadPending_4218F0()
    {
        return field_8D_car_thread_flags & 1;
    }

    // 9.6f 0x4218E0
    inline void ClearCarThreadPending_4218E0()
    {
        field_8D_car_thread_flags &= ~1u;
    }

    // 9.6f 0x421570
    inline void ResetEngineStatus_421570()
    {
        if (field_9C_engine_status == car_engine_status::destroyed_6 || field_9C_engine_status == car_engine_status::unknown_5)
        {
            field_9C_engine_status = car_engine_status::off_1;
        }
    }

    // 9.6f 0x421590
    inline void SetEngineDamaged_421590()
    {
        if (field_9C_engine_status != car_engine_status::destroyed_6)
        {
            field_9C_engine_status = car_engine_status::unknown_5;
        }
    }

    // 9.6f 0x421490
    inline void DespawnSoon_421490()
    {
        if (field_88_despawn_status != car_despawn_status::despawning_5)
        {
            field_88_despawn_status = car_despawn_status::despawn_soon_3;
        }
    }

    void MarkForDespawn_421470()
    {
        if (field_88_despawn_status != car_despawn_status::despawning_5 &&
            field_88_despawn_status != car_despawn_status::despawn_pending_2 &&
            field_88_despawn_status != car_despawn_status::despawn_soon_3)
        {
            field_88_despawn_status = car_despawn_status::marked_for_despawn_4;
        }
    }

    bool IsDeactivated_4214D0()
    {
        return field_88_despawn_status == car_despawn_status::deactivated_7;
    }

    void SetDeactivatedStatus_4214E0()
    {
        field_88_despawn_status = car_despawn_status::deactivated_7;
    }

    // FUNCTION: 96f 0x447ea0
    void ResetDespawnStatus_447EA0()
    {
        field_88_despawn_status = car_despawn_status::active_1;
    }

    bool IsGunJeep_411910()
    {
        return field_84_car_info_idx == car_model_enum::GUNJEEP;
    }

    bool IsTank_411900()
    {
        return field_84_car_info_idx == car_model_enum::TANK;
    }

    bool IsSwatVan_4217A0()
    {
        return field_84_car_info_idx == car_model_enum::SWATVAN;
    }

    // 9.6f 0x403BC0
    inline bool IsSwatVanOrBankVan_403BC0()
    {
        return field_84_car_info_idx == car_model_enum::SWATVAN || field_84_car_info_idx == car_model_enum::bank_van;
    }

    // 9.6f 0x4119F0
    inline char_type IsTurretRotated_4119F0()
    {
        return field_B8_turret_rotated;
    }

    bool IsFireTruck_4118F0()
    {
        return field_84_car_info_idx == car_model_enum::FIRETRUK;
    }

    // 9.6f 0x421780
    bool IsMediCar_421780()
    {
        return field_84_car_info_idx == car_model_enum::MEDICAR;
    }

    bool IsCopCar_421790()
    {
        return field_84_car_info_idx == car_model_enum::COPCAR;
    }

    bool IsFbiCar_411920()
    {
        return field_84_car_info_idx == car_model_enum::EDSELFBI;
    }

    bool IsGt24640_4217D0()
    {
        return field_84_car_info_idx == car_model_enum::GT24640;
    }

    bool IsTvVan_4217E0()
    {
        return this->field_84_car_info_idx == car_model_enum::TVVAN;
    }

    bool IsTruckTransport_447EC0()
    {
        return this->field_84_car_info_idx == car_model_enum::TRUKTRNS;
    }

    bool HasPlayerDriver_4215C0()
    {
        return field_54_driver && field_54_driver->field_15C_player &&
            field_54_driver->get_occupation_403980() != ped_ocupation_enum::empty;
    }

    bool IsEngineCrushed_447EB0()
    {
        return this->field_9C_engine_status == car_engine_status::crushed_7;
    }

    Fix16 ApplyScale_421910(Fix16 value)
    {
        if (field_68_scale != kFP16One_6777D0)
        {
            return value * field_68_scale;
        }
        else
        {
            return value;
        }
    }

    Fix16 GetX_41E430()
    {
        return field_50_car_sprite->field_14_xy.x;
    }

    Fix16 GetY_41E440()
    {
        return field_50_car_sprite->field_14_xy.y;
    }

    Fix16 GetZ_41E450()
    {
        return field_50_car_sprite->field_1C_zpos;
    }

    inline bool IsEmittingHorn_411970()
    {
        return field_A7_horn > 0 && field_A7_horn <= 0xF8u;
    }

    inline bool IsEmittingHorn_411990()
    {
        // Not sure what is going on here, seems to same as 0x411970 ??
        return field_A7_horn <= 0xF8u && field_A7_horn > 0;
    }

    s16 GetLastSeenTimer_4A9AD0()
    {
        return field_76_last_seen_timer;
    }

    // FUNCTION: 96f 0x4215b0
    bool IsDespawning_4215B0()
    {
        return field_88_despawn_status == car_despawn_status::despawning_5;
    }

    bool IsTruckCab_421620()
    {
        car_info* v1 = gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx);
        return v1->IsTruckCab_41FEA0();
    }

    bool IsLongerThanOneBlock_447ED0()
    {
        return (field_50_car_sprite->GetHeight_447E70() > kFpOne_679E74) ? true : false;
    }

    bool CanBeLiftedByCrane_447F00()
    {
        return !IsTruckCab_421620() && !gGtx_0x106C_703DD4->get_car_info_5AA3B0(field_84_car_info_idx)->IsTrailer() &&
            !IsTrainModel_403BA0() && !IsLongerThanOneBlock_447ED0();
    }

    void StartEngine_426E00()
    {
        field_9C_engine_status = car_engine_status::on_3;
        HeadlightsOn_43BFE0();
    }

    // 9.6f 0x416B80
    inline void GetXYZ_416B80(Fix16* pX, Fix16* pY, Fix16* pZ)
    {
        *pX = field_50_car_sprite->field_14_xy.x;
        *pY = field_50_car_sprite->field_14_xy.y;
        *pZ = field_50_car_sprite->field_1C_zpos;
    }

    // 9.6f 0x416BB0
    inline Ang16 GetCarRotation_416BB0()
    {
        return field_50_car_sprite->field_0_rotation;
    }

    inline bool CanCollideOver_4216E0()
    {
        car_info* pCarInfo = gGtx_0x106C_703DD4->get_car_info_4BF1F0(field_84_car_info_idx);
        return pCarInfo->IsHeavy_41FF00();
    }

    inline bool HasNonPlayerDriver_4214F0()
    {
        return field_54_driver && !field_54_driver->PedTypeIs_45EDE0(ped_type::player_2);
    }

    inline bool CanCrushCars_49EFE0()
    {
        return CanCollideOver_4216E0() && (IsTank_411900() || !HasNonPlayerDriver_4214F0());
    }

    inline void AddFlags_421890(u16 flag)
    {
        field_78_flags |= flag;
    }

    inline void SetCrushFlag_49EFC0()
    {
        AddFlags_421890(car_flag::Crushing);
    }

    char SnapCarToGreenArrow_444E40(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    char TrySnapCarToNearestDrivableRoadAndDriveForward_445EC0(Fix16 xpos, Fix16 ypos, s32 direction);

    SpriteList_4 field_0_attachments;
    Ped_List_4 field_4_passengers_list;
    DamageAreaFlags field_8_damaged_areas; // TODO: check if it's a bitset
    Car_Door_10 field_C_doors[4];
    Car_BC* mpNext;
    Sprite* field_50_car_sprite;
    Ped* field_54_driver;
    CarPhysics_B0* field_58_physics;
    CarAI_78* field_5C_AI;
    CarChaseTask_40* field_60_pChaseTask;
    Trailer* field_64_pTrailer;
    Fix16 field_68_scale;
    s32 field_6C_car_id;
    s32 field_70_exploder_ped_id;
    s16 field_74_damage;
    s16 field_76_last_seen_timer;
    u16 field_78_flags; // car_flag masks
    s16 field_7A_pad;
    s32 field_7C_ped_type;
    char_type field_80_abandoned;
    char_type field_81_pad;
    char_type field_82_pad;
    char_type field_83_pad;
    s32 field_84_car_info_idx;
    s32 field_88_despawn_status; // car_despawn_status
    u8 field_8C_damage_level;
    char_type field_8D_car_thread_flags;
    char_type field_8E_flash_count;
    char_type field_8F_pad;
    s32 field_90_death_cause;
    u8 field_94_exploder_timer;
    u8 field_95_player_ped_id;
    char_type field_96_pad;
    char_type field_97_pad;
    s32 field_98_door_lock;
    s32 field_9C_engine_status;
    s32 field_A0_car_kind; // police car, fire engine, mission car etc
    union
    {
        char_type field_A4_light_flags;
        CarLightFlags8 field_A4_bf;
    };
    char_type field_A5_flash_phase_counter;
    union
    {
        char_type field_A6_turn_flags;
        CarTurnFlags8 field_A6_bf;
    };
    u8 field_A7_horn;
    u8 field_A8_horn_cooldown;
    char_type field_A9_wreck_countdown;
    char_type field_AA_pad;
    char_type field_AB_pad;
    s32 field_AC_sound_event;
    s32 field_B0_emitter_status;
    s32 field_B4_weapon_kind;
    char_type field_B8_turret_rotated;
    char_type field_B9_pad;
    char_type field_BA_pad;
    char_type field_BB_pad;
};

struct Car_BC_Pool
{
    //Inlined in CarManager_6C constructor 9.6f -> 0x426db0
    Car_BC_Pool() = default;

    ~Car_BC_Pool() = default;

    // TODO: 9.6f addr/check
    void Remove(Car_BC* pCar)
    {
        field_0_pool.DeAllocate(pCar);
    }

    // 9.6f 0x45AD30
    inline s16 get_cars_count_45AD30()
    {
        return field_0_pool.field_X_count;
    }

    // 9.6f 0x420E50
    inline Car_BC* GetFirstCar_420E50()
    {
        return field_0_pool.field_4_pPrev;
    }

    // FUNCTION: 96f 0x420f20
    void UpdateNextPrev(Car_BC* pItem)
    {
        field_0_pool.UpdateNextPrev(pItem);
    }

    Pool<Car_BC, 306> field_0_pool;
};

class TrailerPool
{
  public:
    //Inlined in CarManager_6C constructor 9.6f -> 0x425500
    TrailerPool() = default;

    ~TrailerPool()
    {
        field_0_pool.field_0_pHead = nullptr;
    }

    // 9.6f 0x425580
    inline void Remove_425580(Trailer* pTrailer)
    {
        field_0_pool.DeAllocate(pTrailer);
    }

    PoolBasic<Trailer, 10> field_0_pool;
};

struct TrafficSpawner_14
{
    TrafficSpawner_14();
    ~TrafficSpawner_14();
    char IsInsideCameraEdge_582360(int direction, Fix16 x, Fix16 y);
    void MakeTrafficForCurrCamera_5832C0();
    char_type SpawnTrafficCarFacing_583260(s32 angle_face);
    void GenerateTraffic_583670();

    // 9.6f 0x4B3230. A thiscall member: SpawnTrafficCar_582480 sets ecx = this before calling it
    Fix16 GetRandomTrafficSpeed_583750(Fix16 max_speed, u8* pOut);

    char_type SpawnTrafficCar_582480(s32 spawn_type, s32 arrow_direction, s32 blocked_direction);

    char_type IsFlatRoadAt_583870(Fix16 xpos, Fix16 ypos);

    s32 GetLaneOffset_5838E0(u8 lane_idx);
    char_type ResolveSpawnDirection_583930(char_type a1, char_type a2);

    Camera_0xBC* field_0_cam;
    Fix16_Rect* field_4_rect;
    s8 field_8_spawn_side;
    s8 field_9_traffic_car_model;
    s8 field_A_unused;
    s8 field_B_unused;
    Player* field_C_player;
    gmp_zone_info* field_10_zone;
};

EXTERN_GLOBAL(Car_BC_Pool*, gCar_BC_Pool_67792C);

EXTERN_GLOBAL(Sprite*, gSprite_6F61E8);

EXTERN_GLOBAL(TrailerPool*, gTrailerPool_66AC80);

EXTERN_GLOBAL(TrafficSpawner_14*, gTrafficSpawner_14_677934);

EXTERN_GLOBAL(char_type, gbRngRemapTableDone_679C0A);

// TODO: move
s32 GetDamageMultiplier_45CF90(Ped* pPed);
s32 GetDeathCauseForObjectModel_48E780(s32 model);
char_type IsNoKillCreditObject_48E720(s32 model);