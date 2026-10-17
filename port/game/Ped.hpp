#pragma once

#include "PatrolRoutePool_1D7E.hpp"
#include "char.hpp"
#include "core/ang16.hpp"
#include "core/bit_set32.hpp"
#include "core/enums.hpp"
#include "core/fix16.hpp"
#include "core/fix16_point.hpp"
#include "core/function.hpp"
#include "core/rng.hpp"
#include "miss2_xyz.hpp"
#include "ped_flags.hpp"
#include "sprite.hpp"
#include <cstdio>

class Player;
class gmp_map_zone;
class PedGroup;
class Weapon_30;
class Gang_144;
class Sprite;
class Char_8;
class Char_B4;
class PatrolRoute_96;
class Object_2C;
class Car_BC;
class TrainStation_34;

class Ped
{
  public:
    bool Is_occupation_elvis_433C90()
    {
        return field_240_occupation == PedOcupation::kElvis;
    }

    void set_enter_car_as_passenger_4039B0(s32 v)
    {
        field_248_enter_car_as_passenger = v;
    }

    s32 get_enter_car_as_passenger_4039C0()
    {
        return field_248_enter_car_as_passenger;
    }

    char_type GetInternalObjectiveStatus_4039D0()
    {
        return field_226_internal_objective_status;
    }

    Fix16 GetGameObjectVelocity_433C20()
    {
        return field_168_game_object->get_velocity_41B080();
    }

    // 9.6f 0x4AF860 (0x4AF880 is an identical copy)
    void SetTrainStation_4AF860(TrainStation_34* pStation)
    {
        if (pStation != field_13C_pTrainStation)
        {
            field_13C_pTrainStation = pStation;
        }
    }

    // 9.6f 0x4117D0 (0x4117F0 is an identical copy)
    TrainStation_34* PopTrainStation_4117D0()
    {
        TrainStation_34* pStation = field_13C_pTrainStation;
        field_13C_pTrainStation = nullptr;
        return pStation;
    }

    void set_target_car_door_403A70(u8 v)
    {
        field_24C_target_car_door = v;
    }

    void set_objective_target_ped_403AC0(Ped* v)
    {
        field_148_objective_target_ped = v;
    }

    Ped* get_objective_target_ped_403AD0()
    {
        return field_148_objective_target_ped;
    }

    void set_target_to_enter_403B00(Car_BC* v)
    {
        field_154_target_to_enter = v;
    }

    u16 get_car_state_timer_403B20()
    {
        return field_21A_car_state_timer;
    }

    u16 get_objective_timer_403B30()
    {
        return field_218_objective_timer;
    }

    void set_objective_status_403B40(u8 v)
    {
        field_225_objective_status = v;
    }

    void SetInternalObjectiveStatus_403B50(char_type v)
    {
        field_226_internal_objective_status = v;
    }

    void SetOffscreenCounter_433B80(u16 v)
    {
        field_20E_offscreen_counter = v;
    }

    void set_objective_timer_433C80(u16 v)
    {
        field_218_objective_timer = v;
    }

    void clear_last_char_punched_475B40()
    {
        field_188_last_char_punched = nullptr;
    }

    Ped* get_last_char_punched_475B10()
    {
        return field_188_last_char_punched;
    }

    Char_B4* get_game_object_4A5020()
    {
        return field_168_game_object;
    }

    Ped(); // 45AE70
    ~Ped(); // 45AF00
    void Reset_45AFC0();
    void PoolAllocate();
    char_type IsLawEnforcement_45B4E0();
    Fix16Point GetVelocityVector_45B520();
    void SetRecentCrimeTimer_45B550();
    void SetPlayer_45B560(Player* pPlayer, char_type bIsAuxPed);
    bool IsEmergencyOccupation_45B590();
    void CopyStatsFromPed_45B5B0(Ped* pSrc);
    Car_BC* GetCarBeingEnteredOrExited_45BBF0();
    void TeleportToCoord_45BC10(Fix16 xpos, Fix16 ypos);
    void ManageShocking_45BC70();
    bool CloseDoorIfCarApproaching_45BD20(Car_BC* pCar);
    s32 GetBulletSpriteOffset_45BE30();
    void SetOnFire();
    void PutOutFire();
    void ManageBurning_45BEC0();
    void DrawFlamesAndStartScreamTimer();
    void SetInvisible();
    void SetVisible();
    void SetSpriteSemiTransIfInvisible();
    void SetInvulnerable();
    void ClearInvulnerable_45C050();
    void SetSpriteFlagIfInvulnerable_45C070();
    void RestoreCarOrPedHealth();
    void SpawnCharInZone_45C0C0(gmp_map_zone* pZone);
    void PoolDeallocate();
    void RespawnPed_45C350(gmp_map_zone* pZone);
    void ResetForPlayerRespawn_45C410();
    void UpdatePositionFromCar_45C4B0();
    void ChangeNextPedState1_45C500(s32 new_state);
    void ChangeNextPedState2_45C540(s32 new_state);
    void RestorePreviousPedState_45C5A0();
    void CancelEnterCarObjective_45C5C0();
    void SpawnDriverRunAway_45C650(Car_BC* pCar, Ped* pPed);
    void SpawnPedInCar_45C730(Car_BC* pCar);
    void EnterCarAsDriver(Car_BC* pCar);
    void EnterCarAsPassenger_45C7F0(Car_BC* pCar);
    char_type AllocCharB4_45C830(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    Ang16 GetTurnSpeed_45C900();
    Fix16 GetPedVelocity_45C920();
    Ang16 GetRotation();
    Fix16 GetMoveDirection_45C9B0();
    Ang16 ComputeAimAngle_45C9D0();
    void HandleClosePedInteraction_45CAA0();
    void TakeDamage(s16 damage);
    void HandlePedCrossingTrigger_45CF20(Object_2C* pObj);
    char_type HandlePedHitByObject_45D000(Object_2C* pObj);
    char_type AddWeaponWithAmmo_45DD30(s32 weapon_kind, char_type ammo);
    char_type HandlePickupCollision_45DE80(Object_2C* pPickUp);
    void SpawnWeaponOnDeath_45E080();
    void StartCrossingRoad_45E4A0();
    void DeallocateWithGroupCleanup_45EA00();
    void Deallocate_45EB60();
    char_type IsActivePlayerPed_45EDC0();
    bool PedTypeIs_45EDE0(s32 type);
    void SetOccupation_45EE00(u32 occupation);
    void EnterPublicTransport_45EE70();
    void Mugger_AI_45F360();
    void CarThief_AI_45FF60();
    void TaxiCustomer_AI_460820();
    void BusCustomer_AI_461290();
    void TrainCustomer_AI_461530();
    void RobbedDriver_AI_461630();
    void RoadBlockTank_AI_4619F0();
    void UpdateFacingAngle_461A60();
    void Occupation_AI_461F20();
    void UpdateAI_462280();
    void ReleaseGroupSpritesAndWeapons_4624A0();
    void RemovePedWeapons_462510();
    void RemoveSecondaryWeapon_462550();
    void ForceDoNothing_462590();
    void UpdateOnScreenCounter_462620();
    char_type StateMachineTick_4626B0();
    void UpdateCharB4_462B80();
    bool PoolUpdate();
    void ProcessObjective_4632E0();
    void ChangePedStatesByMode_463300(u8 mode);
    void SetStatesForObjective_4633E0(char_type bMainObj);
    void SetObjective(s32 objective, s16 objective_timer = 9999); // 9999: never times out
    void SetObjective2_463830(s32 internal_objective, s16 internal_objective_timer = 9999);
    void ProcessOnFootObjective_463AA0();
    void ProcessInCarObjective_463FB0();
    void CalcApproachPointNearTargetPed_4645B0();
    void Threat_Reaction_AI_465270();
    void ReactToAttacker_465B20();
    bool IsAttackingTargetPed_465CD0();
    bool IsPedAThreat_465D00(Ped* pTargetPed);
    char_type IsInvisibleOnFoot_466B70();
    char_type IsThreatToSearchingPed_4661F0();
    Ped* FindBestTargetPed_Mode1_466B90(s32 max_x_check);
    Ped* FindBestTargetPed_Mode4_466BB0(s32 max_x_check);
    Ped* FindBestTargetPed_Mode5_466BD0(s32 max_x_check);
    Ped* FindBestTargetPed_466BF0(s32 max_x_check);
    Ped* FindNearestPed_Mode4_466F40(u8 max_x_check);
    Ped* FindNearestPed_466F60(u8 max_x_check);
    Ped* FindNearbyPed_466FB0();
    Ped* GetLastProcessedPedOnFoot_467070();
    char_type FindUsableCarDoor_467090();
    Sprite* ResetAnimAndFindNearestSprite_467280();
    void UpdateMovementTowardsTarget_4672E0(Fix16 distance, u8 type);
    void FleeOnFootTillSafe_4678E0();
    void FleeCharOnFootTillSafe_467960();
    void FleeFromCharOnFootAlways_467A20();
    void FleeCharAlwaysOnceCarStopped_467AD0();
    void EnterCarOrFlee_467BD0();
    void KillCharOnFoot_467CA0();
    void KillCharAnyMeans_467E20();
    void KillFrenzy_467FB0();
    void PunchChar_467FD0();
    void ProcessAirborneMovement_468040();
    void TimeWaitedInCar_4682A0();
    void GotoAreaInCar_468310();
    void EnterTargetObjectiveCar_4686C0();
    void LeaveTargetObjectiveCar_468820();
    void EnterTrain_468930();
    void LeaveTrain_468A00();
    void AbortEnterCarObjective_468BD0();
    void PatrolOnFoot_468C70();
    void GotoAreaOnFoot_468DE0();
    void UpdateFollowPedObjective_468E80();
    s32 GetValueByIdParity_469010();
    void UpdateKillerIdTimer_469030();
    void GotoAreaByAnyMeans_469060();
    void UpdateGroupMemberFollow_469BD0();
    void GuardSpot_469BF0();
    void GuardArea_469D60();
    void FailWhenObjectiveTimerExpired_469E10();
    void SetCarAiSpeedOne_469E30();
    void SetupCarFollowTargetPed_469E50();
    void FollowPedInCar_469F30();
    void WaitInCurrentCar_469FC0();
    void CallPoliceCar_469FE0();
    void PullDriverOutOfCar_46A1F0();
    void FollowCarInCurrCar_46A290();
    void FollowCarOnFootWithOffset_46A350();
    void FireAtObject_46A530();
    void FireAtPlayer_46A5E0();
    void AimVehicleTurretStateMachine_46A6D0();
    void DestroyTargetObject_46A7C0();
    void DestroyTargetCar_46A850();
    void FleeOnFootTillSafe_46A8F0();
    void FleeFromPedTillSafe_46A9C0();
    void FleeFromPedAlways_46AAE0();
    void CheckFollowTargetPedStillValid_46AB50();
    void FollowTargetStateMachine_46AC20();
    void ChaseTargetStateMachine_46B170();
    void PullDriverOutOfCarStateMachine_46B2F0();
    void MeleeAttackStateMachine_46B670();
    void WaitOnFoot_46BD30();
    char_type IsOtherPedEnteringAsDriver_46BD50(Car_BC* pCar);
    void EnterCarStateMachine_46BDC0();
    void ExitCarStateMachine_46C250();
    void WalkToTargetPoint_46C770();
    void GotoAreaOnFoot_46C7E0();
    void WalkToTargetThenStop_46C8A0();
    void FollowPathPoints_46C910();
    void CrossRoad_46C9B0();
    void EmptyState14_46CA60();
    void FollowPedInCar_46CA70();
    void StartPedCrossingAtTrafficLight_Y_Backward_46CB30();
    void StartPedCrossingAtTrafficLight_X_Forwards_46CC70();
    void StartPedCrossingAtTrafficLight_Y_Forwards_46CDB0();
    void StartPedCrossingAtTrafficLight_X_Backwards_46CEF0();
    void WaitForTrain_46D030();
    void WaitForCarStateTimer_46D0B0();
    void EnterTrainStateMachine_46D0D0();
    void ExitTrainStateMachine_46D240();
    void FollowCarOnFoot_46D300();
    void AttackTargetStateMachine_46D460(u8 targetType);
    void AttackPed_46DB60();
    void AttackCar_46DB70();
    void AttackObject_46DB80();
    Sprite* GetSprite_46DF50();
    void SetupFollower_46DF70(Ped* pToFollow, s32 weaponIdx);
    bool CanBeRecruitedToGroup_46E020(PedGroup* pGroup);
    void RecruitNearbyPeds_46E080(s32 desiredCount, Fix16 searchRadius);
    void SpawnPedGroupFollowers_46E200(u8 total);
    u8 get_wanted_star_count_46EF00();
    void set_wanted_level_46EF40(u16 wanted);
    void IncreaseWantedLevelFromDebugKeys_46EFD0();
    void set_wanted_star_count_46F070(u8 star_count);
    bool WantedStartCountLessThan_46F100(u8 star_count);
    Weapon_30* GetWeaponFromPed_46F110();
    void ApplyAimJitter_46F1E0(Weapon_30* pWeapon);
    void ManageWeapon_46F390();
    Weapon_30* ChooseAttackWeapon_46F490();
    void ForceWeapon_46F600(s32 weapon_kind);
    void GiveWeapon_46F650(s32 weapon_kind);
    void ApplyGangRespectForKill_46F680(Ped* pPed);
    void UpdateStatsForKiller_46F720();
    void Kill_46F9D0();
    void AddThreateningPedToList_46FC70();
    void HandleShootingAtCar_46FC90(Car_BC* pCar, s32 model);
    void ProcessWeaponHitResponse_46FE20(Object_2C* pObj);
    void NotifyWeaponHit_46FF00(Fix16 xpos, Fix16 ypos, s32 model);
    void HandleWeaponFireEnd_46FFF0(s32 model);
    void AimRoofGun_470050();
    void add_wanted_points_470160(s16 wanted_amount);
    bool IsNearestSpriteACar_4701D0();
    void StartPedWalking_470200(Fix16 xpos, Fix16 ypos, Fix16 zpos);
    void BecomeLeaderOfGroup_4702D0(Ped* pPed);
    void BecomeDummyOnPlayerDisconnect_470300();
    void PushPatrolPoint_4702A0(s8 x, s8 y, s8 z);
    s32 IsInTrain_470F00();

    void FleeCharAnyMeansTillSafe_Nullsub();
    void FleeCharAnyMeansAlways_Nullsub();
    void KillCar_Nullsub();
    void Objective50_Nullsub();
    void State30_Nullsub();

    u8 get_ped_ref_idx_420B50()
    {
        return field_267_ped_ref_idx;
    }

    void ClearGroupAndGroupIdx_403A30()
    {
        this->field_164_ped_group = nullptr;
        this->field_23C_group_idx = 0;
    }

    s32 GetPedType_420B70()
    {
        return field_238_ped_type;
    }

    void ClearAttacking()
    {
        // There was no way to match this without using a bit field
        field_21C_bf.bAttacking = 0;
    }

    void SetElectrofingers_4A5060()
    {
        field_21C_bf.bElectroFingers = true;
    }

    void SetFullHealth_4A5050()
    {
        set_health_4039A0(100);
    }

    void ClearElectrofingers_482080()
    {
        field_21C_bf.bElectroFingers = false;
    }

    bool IsActive_433B40()
    {
        return field_21C_bf.bActive != 0;
    }

    bool IsAttacking()
    {
        return field_21C_bf.bAttacking != 0;
    }

    // 9.6f inline 0x450CB0
    u8 GetObjectiveStatus_450CB0()
    {
        return field_225_objective_status;
    }

    void reset_ped_group()
    {
        field_164_ped_group = nullptr;
        field_23C_group_idx = 0;
    }

    void set_ped_group(PedGroup* ptr)
    {
        field_164_ped_group = ptr;
    }

    void set_ped_group_id(s8 group_idx)
    {
        field_23C_group_idx = group_idx;
    }

    u16 GetOffscreenCounter_4039F0() const
    {
        return field_20E_offscreen_counter;
    }

    bool has_car_403B80() const
    {
        return field_16C_car != nullptr;
    }

    s32 GetPedState_403990() const
    {
        return field_278_ped_state_1;
    }

    void SetStolenCar_492CB0(Car_BC* pCar)
    {
        field_140_stolen_car = pCar;
    }

    Car_BC* GetStolenCar_49EF40()
    {
        return field_140_stolen_car;
    }

    // 9.6f inline 0x403AE0
    void SetInternalTargetPed_403AE0(Ped* pSrc)
    {
        field_14C_internal_target_ped = pSrc;
    }

    // 9.6f inline 0x403950
    void SetPanicking_403950()
    {
        field_21C_bf.bPanicking = true;
    }

    bool IsPanicking()
    {
        return field_21C_bf.bPanicking;
    }

    void ClearPanicking_403960()
    {
        field_21C_bf.bPanicking = false;
    }

    void SetTargetObjectiveCar(Car_BC* ptr)
    {
        field_150_target_objective_car = ptr;
    }

    Car_BC* get_target_objective_car_403AB0()
    {
        return field_150_target_objective_car;
    }

    void SetPedType_403920(s32 type)
    {
        field_238_ped_type = type;
    }

    Fix16 get_cam_x()
    {
        return field_1AC_cam.x;
    }

    Fix16 get_cam_y()
    {
        return field_1AC_cam.y;
    }

    Fix16 get_cam_z()
    {
        return field_1AC_cam.z;
    }

    bool IsWithinArea(SCR_Rect_f* rect)
    {
        Fix16 x_pos = field_1AC_cam.x;
        Fix16 width = rect->field_C_size.field_0_x;
        Fix16 y_pos = field_1AC_cam.y;
        Fix16 height = rect->field_C_size.field_4_y;
        Fix16 z_pos = field_1AC_cam.z;
        Fix16 z_target = rect->field_0_pos.field_8_z;
        return (x_pos >= rect->field_0_pos.field_0_x - width && x_pos <= rect->field_0_pos.field_0_x + width &&
                y_pos >= rect->field_0_pos.field_4_y - height && field_1AC_cam.y <= rect->field_0_pos.field_4_y + height &&
                z_pos.ToUInt8() == z_target.ToUInt8());
    }

    s16 get_wanted_points_433DC0()
    {
        return field_20A_wanted_points;
    }

    bool not_enter_car_as_passenger_4A5040()
    {
        return field_248_enter_car_as_passenger != 1;
    }

    Car_BC* get_car_416B60()
    {
        return field_16C_car;
    }

    // 9.6f 0x420B60, 0x4C4F20
    s32 get_id() const
    {
        return field_200_id;
    }

    Fix16 get_max_speed_1F0()
    {
        return field_1F0_max_speed;
    }

    void SetStateInitPending_433B50()
    {
        field_168_game_object->field_16_state_init_pending = 1;
    }

    s8 get_remap_433BA0()
    {
        return field_244_remap;
    }

    bool is_player_41B0A0()
    {
        return field_15C_player != nullptr;
    }

    void DoJump_433C40()
    {
        if (field_168_game_object)
        {
            field_168_game_object->DoJump_5454D0();
        }
    }

    s32 TakeVoiceEvent_41B0B0()
    {
        s32 ret = field_250_voice_event; // sound id the ped says next
        field_250_voice_event = 0;
        return ret;
    }

    void SetVoiceEvent_IfBit24Clear_433DD0(s32 voice_event)
    {
        // TODO: Check if (HIBYTE(this->field_21C) & 1)
        // is correct
        if (field_21C_bf.bOnFire == 0)
        {
            field_250_voice_event = voice_event;
        }
    }

    u8 get_target_car_door_403A60()
    {
        return field_24C_target_car_door;
    }

    // 9.6f inline 0x433B90
    void set_remap_433B90(u8 remap)
    {
        field_244_remap = remap;
    }

    // 9.6f inline 0x4039A0
    void set_health_4039A0(s16 health)
    {
        field_216_health = health;
    }

    // 9.6f inline 0x433B70
    s32 get_health_433B70()
    {
        return field_216_health;
    }

    void set_occupation_403970(s32 occupation)
    {
        field_240_occupation = occupation;
    }

    s32 get_occupation_403980()
    {
        return field_240_occupation;
    }

    void SetMoveTargetX_433C50(Fix16 x)
    {
        this->field_1C4_move_target_x = x;
    }

    void SetMoveTargetY_433C60(Fix16 y)
    {
        this->field_1C8_move_target_y = y;
    }

    void SetMoveTargetZ_433C70(Fix16 z)
    {
        this->field_1CC_move_target_z = z;
    }

    Fix16 GetMoveTargetX_492CE0()
    {
        return field_1C4_move_target_x;
    }

    Fix16 GetMoveTargetY_492CF0()
    {
        return field_1C8_move_target_y;
    }

    // TODO: to use this inline we need to fix a circular dependency issue
    inline s32 get_car_model();

    // 9.6f 0x433C10: a thin wrapper around Char_B4::SetRemap. The remap is taken by const reference:
    // by value, VC6 loads the argument before the Char_B4 and 9 callers stop matching.
    void SetRemap_433C10(const u8& remap)
    {
        Char_B4* p_B4 = field_168_game_object; // local necessary to match Ped::SetupFollower_46DF70
        p_B4->SetRemap_46DD50(remap);
    }

    void TriggerVoiceEventRateLimited_433E50()
    {
        if (static_cast<u32>(gAmbientRng->GetCurRng() - field_220_last_voice_rng) > 5)
        {
            SetVoiceEvent_IfBit24Clear_433DD0(25);
            field_220_last_voice_rng = gAmbientRng->GetCurRng();
        }
    }

    void SetJumpOverMode_433BB0(s32 value)
    {
        field_230_jump_over_mode = value;
    }

    Ang16 GetTargetFacingAngle_492CC0()
    {
        return field_130_target_facing_angle;
    }

    s32 GetJumpOverMode_492C20()
    {
        return field_230_jump_over_mode;
    }

    void SetPedClass_433BC0(s32 value)
    {
        field_22C_ped_class = value;
    }

    void ClearHitCount_403A20()
    {
        field_228_hit_count = 0;
    }

    void ClearWantedPoints_420B80()
    {
        field_20A_wanted_points = 0;
    }

    bool IsGroupLeader_472FD0()
    {
        return field_23C_group_idx == 99;
    }

    void ClearAttacker_433BE0()
    {
        field_144_attacker = nullptr;
    }

    void SetRotation_433C00(Ang16 rotation)
    {
        field_168_game_object->set_rotation_433A30(rotation);
    }

    s32 GetPedState2_433B60()
    {
        return field_27C_ped_state_2;
    }

    // Same body as the out-of-line copy IsInvisibleOnFoot_466B70: this form's front-end size (47) is what
    // makes the inlined IsPedAThreat in IsThreatToSearchingPed_4661F0 call the copy (nested budget 42)
    bool IsInvisibleOnFoot_433DA0()
    {
        if (field_21C_bf.bInvisible && field_168_game_object != nullptr)
        {
            return true;
        }

        return false;
    }

    bool bHasGameObject_403B70()
    {
        return field_168_game_object != nullptr;
    }

    u8 GetPanicking()
    {
        return field_21C_bf.bPanicking;
    }

    u8 GetAttacking_433CA0()
    {
        return field_21C_bf.bAttacking;
    }

    u8 IsOnFire_475B50()
    {
        return field_21C_bf.bOnFire;
    }

    u8 HasSpottedPlayer()
    {
        return field_21C_bf.bSpottedPlayer;
    }

    s32 get_objective_403A80()
    {
        return field_258_objective;
    }

    Car_BC* get_target_to_enter_403B10()
    {
        return field_154_target_to_enter;
    }

    void SetAttacking_4A5010()
    {
        field_21C_bf.bAttacking = true;
    }

    void ClearAttacking_403A40()
    {
        field_21C_bf.bAttacking = false;
    }

    bool IsPedGoingToEnterCar_492FD0()
    {
        return field_258_objective == Objectives::kEnterCarAsDriver || field_25C_internal_objective == Objectives::kEnterCarAsDriver;
    }

    Ang16 GetAimAngle_4CCA90()
    {
        return field_12E_aim_angle;
    }

    bool isDead_403B60()
    {
        return this->field_278_ped_state_1 == PedState1::kDead;
    }

    PedGroup* GetGroup_475AF0()
    {
        return this->field_164_ped_group;
    }

    s32 GetInternalObjective_403A90()
    {
        return field_25C_internal_objective;
    }

    Ped* GetInternalTargetPed_403AF0()
    {
        return field_14C_internal_target_ped;
    }

    void SetAttacker_433BF0(Ped* pPed)
    {
        field_144_attacker = pPed;
    }

    void IncrementAttackersCount_433BD0()
    {
        ++field_262_attackers_count;
    }

    PatrolPoint_3 field_0_patrol_points[100];
    Ang16 field_12C_facing_angle;
    Ang16 field_12E_aim_angle;
    Ang16 field_130_target_facing_angle;
    Ang16 field_132_follow_car_offset_angle;
    Ang16 field_134_rotation;
    s16 field_136_unused;
    sptr field_138_collided_char_b4; // Char_B4*
    TrainStation_34* field_13C_pTrainStation;
    Car_BC* field_140_stolen_car;
    Ped* field_144_attacker;
    Ped* field_148_objective_target_ped;
    Ped* field_14C_internal_target_ped;
    Car_BC* field_150_target_objective_car;
    Car_BC* field_154_target_to_enter;
    Car_BC* field_158_escape_car;
    Player* field_15C_player;
    Ped* mpNext;
    PedGroup* field_164_ped_group;
    Char_B4* field_168_game_object;
    Car_BC* field_16C_car;
    Weapon_30* field_170_selected_weapon;
    Weapon_30* field_174_pWeapon;
    Weapon_30* field_178_car_weapon;
    Gang_144* field_17C_pGang;
    Ped* field_180_car_thief;
    Object_2C* field_184_pObj2C;
    Ped* field_188_last_char_punched;
    PatrolPoint_3* field_18C_current_path_point;
    PatrolRoute_96* field_190_patrol_route;
    PatrolPoint_3* field_194_current_patrol_point;
    Ped* field_198_hit_target_ped; // ped last hit by this ped's weapon / electrobaton
    Gang_144* field_19C_dummy_gang; // gang of a dummy ped
    Object_2C* field_1A0_objective_target_object;
    Object_2C* field_1A4_internal_target_object;
    Ped* field_1A8_ped_killer;
    Fix16Vec field_1AC_cam;
    Fix16 field_1B8_target_x;
    Fix16 field_1BC_target_y;
    Fix16 field_1C0_target_z;
    Fix16 field_1C4_move_target_x;
    Fix16 field_1C8_move_target_y;
    Fix16 field_1CC_move_target_z;
    Fix16 field_1D0_internal_target_x;
    Fix16 field_1D4_internal_target_y;
    Fix16 field_1D8_internal_target_z;
    Fix16 field_1DC_objective_target_x;
    Fix16 field_1E0_objective_target_y;
    Fix16 field_1E4_objective_target_z;
    Fix16 field_1E8_target_distance_limit;
    s32 field_1EC_unused;
    Fix16 field_1F0_max_speed;
    Fix16 field_1F4_walk_speed;
    Fix16 field_1F8_run_speed;
    Fix16 field_1FC_follow_car_offset_distance;
    s32 field_200_id;
    s32 field_204_killer_id;
    u16 field_208_invulnerability;
    s16 field_20A_wanted_points;
    u16 field_20C_target_unseen_counter; // frames the target was not seen; gives up after 15
    u16 field_20E_offscreen_counter;
    u16 field_210_shock_counter;
    u16 field_212_electrocution_threshold;
    s16 field_214_unused;
    s16 field_216_health;
    u16 field_218_objective_timer;
    u16 field_21A_car_state_timer;

    PedFlags32 field_21C_bf;

    s32 field_220_last_voice_rng;
    PedFlags8 field_224_bf;
    u8 field_225_objective_status; // it uses objective_status enum
    char_type field_226_internal_objective_status;
    char_type field_227_unused;
    char_type field_228_hit_count; // times hit by the attacker
    u8 field_229_mug_count; // times mugged while mugging a player
    s32 field_22C_ped_class; // ped_class
    s32 field_230_jump_over_mode; // ped_jump_over_mode
    char_type field_234_lifetime_timer; // ped_lifetime
    s32 field_238_ped_type;
    u8 field_23C_group_idx;
    s32 field_240_occupation;
    char_type field_244_remap;
    s32 field_248_enter_car_as_passenger;
    u8 field_24C_target_car_door;
    s32 field_250_voice_event;
    s32 field_254_block_spec;
    //char_type field_255;
    //char_type field_256;
    //char_type field_257;
    s32 field_258_objective;
    s32 field_25C_internal_objective;
    u8 field_260_wait_counter; // frames waited on the target (field_224_bf.bWaitingOnTarget)
    char_type field_261_unused;
    u8 field_262_attackers_count;
    u8 field_263_prev_attackers_count;
    u8 field_264_killer_id_timer;
    u8 field_265_patrol_list_idx;
    u8 field_266_path_fail_count;
    u8 field_267_ped_ref_idx;
    char_type field_268_electrocution_timer;
    char_type field_269_unused;
    u8 field_26A_recent_crime_timer;
    s32 field_26C_graphic_type;
    s32 field_270_fire_mode; // ped_fire_mode
    s32 field_274_gang_car_model;
    s32 field_278_ped_state_1;
    s32 field_27C_ped_state_2;
    s32 field_280_stored_ped_state_1;
    s32 field_284_stored_ped_state_2;
    s32 field_288_threat_search;
    s32 field_28C_threat_reaction;
    s32 field_290_death_cause; // ped_death_cause
};

void CarDoorAlignmentSolver_545AF0(u8 a1, Car_BC* a2, u8 a3, Fix16& a4, Fix16& a5, Ang16& a6);

EXTERN_GLOBAL(s32, gPedId_61A89C);

EXTERN_GLOBAL(u8, gNumberMuggersSpawned_6787CA);

EXTERN_GLOBAL(u8, gNumberCarThiefsSpawned_6787CB);

EXTERN_GLOBAL(u8, gNumberElvisLeadersSpawned_6787CC);

EXTERN_GLOBAL(u8, gNumberWalkingCopsSpawned_6787CD);

EXTERN_GLOBAL(u8, bThreateningPedAdded_6787EF);

EXTERN_GLOBAL(Fix16, kFpOne256th_678620);
EXTERN_GLOBAL(Fix16, kFpFour_678670);
EXTERN_GLOBAL(Fix16, kFpOneSixteenth_678448);
EXTERN_GLOBAL(Fix16, kFpPoint1_6FD824);
