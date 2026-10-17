#pragma once

#include "CarInfo_808.hpp"
#include "Car_BC.hpp"
#include "core/ang16.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"

class Sprite;
class Car_BC;
class Trailer;
class SpriteShape_4C;
class Ped;
class ModelPhysics_48;
class CarAI_78;

EXTERN_GLOBAL(Fix16, dword_6FD120);
EXTERN_GLOBAL(ModelPhysics_48*, gCarInfo_48_6FE258);
EXTERN_GLOBAL(Ang16, kAng90_6FE00C);
EXTERN_GLOBAL(Ang16, kAng270_6FE154);
EXTERN_GLOBAL(CarInfo_2C*, gCarInfo_2C_6FE0E4);
EXTERN_GLOBAL(Fix16, gDamageSpeedFactor_6FE348);
EXTERN_GLOBAL(Fix16, kDrowningMaxSpeed_677794);
EXTERN_GLOBAL(Fix16Point, stru_6FDF50);
EXTERN_GLOBAL(Fix16, dword_6FE0B0);
EXTERN_GLOBAL(Fix16Point, CollisionIntersectionPoint_6FE1A0);

Fix16Point ComputeLineLineIntersection_55F3B0(Fix16 OwnerMass,
                                              Fix16 TargetMass,
                                              const Fix16Point& RelativeVelocity,
                                              Fix16Point& DistToCollision_ByRef,
                                              Fix16Point& CollisionIntersectPoint,
                                              const Fix16Point& CoM_related,
                                              Fix16Point& a8,
                                              Fix16 OwnerMomOfInertia,
                                              Fix16 TargetMomOfInertia,
                                              Fix16 offset);

namespace car_turn_direction
{
enum
{
    clockwise_m1 = -1,
    none_0 = 0,
    anticlockwise_1 = 1,
};
} // namespace car_turn_direction

class CarPhysics_B0
{
  public:
    Fix16 get_cp3_40F800()
    {
        return field_6C_cp3;
    }

    void set_cp3_40F810(Fix16 v)
    {
        field_6C_cp3 = v;
    }

    Ang16 get_theta_40F820()
    {
        return field_58_theta;
    }

    void set_theta_40F830(Ang16 v)
    {
        field_58_theta = v;
    }

    void set_cp1_40F7E0(const Fix16Point& v)
    {
        field_38_cp1.x = v.x;
        field_38_cp1.y = v.y;
    }

    void set_linvel_421170(const Fix16Point& v)
    {
        field_40_linvel_1.x = v.x;
        field_40_linvel_1.y = v.y;
    }

    void StopMoving_4895D0()
    {
        field_74_ang_vel_rad = dword_6FD120;
        field_40_linvel_1.Clear();
    }

    Ang16 GetLinvelAngle_4211E0()
    {
        return field_40_linvel_1.Atan2();
    }

    bool IsRngBelowDamage_421130()
    {
        return static_cast<u32>(gAmbientRng->GetCurRng()) < field_8_total_damage_q;
    }

    void SetHandBrakeOff_421260()
    {
        field_92_is_hand_brake_on = 0;
    }

    void SetHandBrakeOn_421250()
    {
        field_92_is_hand_brake_on = 1;
    }

    char_type get_is_hand_brake_on_421270()
    {
        return field_92_is_hand_brake_on;
    }

    // TODO: Ordering
    Fix16 ComputeZPosition_559E90();

    Fix16Point get_cp1_40B560();
    ~CarPhysics_B0() = default; // 0x446ee0
    void ShowPhysicsDebug_559430();
    void ShowSpeedRevsDamage_5597B0();
    bool IsNotMoving_5599D0();
    void UpdateTrailerPhysicsFromTowingCar_559A40();
    void UpdateTrailerAlignment_559B40();
    void EnforceTrailerControlLimits_559B50();
    void set_field_A0_559B90(const s32& spin_dir);
    void SpinOutOnOil_559BA0();
    void ScarePedsOnDrivingFast_559C30();
    void ApplyForcedSteering_559DD0();
    void ApplyObjectImpact_559E20(Object_2C* pObj);
    Fix16Point ComputeCombinedCenterOfMass_559EC0();
    Fix16 CalculateMass_559FF0();
    Fix16 GetEffectiveMomentOfInertia_55A050();
    u8 IsInAir_55A0B0();
    Fix16 GetTrailerAwareTurnRatio_55A100();
    // Inlined in UpdateSteeringAngle_562560. The scale is computed before the turn ratio call, which
    // needs it as a by-value argument ahead of the turn direction (arguments are evaluated right to left)
    Fix16 GetScaledTurnRatio(Fix16 scale, s32 turn_direction)
    {
        return GetTrailerAwareTurnRatio_55A100() * turn_direction * scale;
    }
    char_type IsFootBrakeOn_55A150();
    char_type IsAccelerationOrReverseOn_55A180();
    void SetVelocityTowardTarget_55A1D0(Fix16 targetX, Fix16 targetY, Fix16 targetAngle, s32* rotationMode);
    void restore_saved_physics_state_55A400();
    void save_physics_state_55A4B0();
    void restore_state_55A550();
    void save_state_55A600();
    Fix16 ComputeRequiredSweepSteps_55A6A0();
    void ResetForceAccumulators_55A840();
    void HandleUserInputs_55A860(char_type bForwardGasOn,
                                 char_type bFootBrakeOn,
                                 char_type bLeftOn,
                                 char_type bRightOn,
                                 char_type bHandBrakeOn);
    void HandleGravityOnSlope_55AA00();
    Fix16 ComputeSlopeCorrection_55AB50(Fix16* pOutY);
    void UpdateZPhysics_55AD90(Fix16 dt);
    void SyncZWithTrailer_55B3F0(Fix16 dt);
    void UpdateZPosition_55B4F0(Fix16 dt);
    void UpdateZPosition_55B7B0(Fix16 dt);
    void EmitImpactParticles_55B7E0(u8 apply_to_corners_mask);
    char_type ProcessGroundCollisionAndSurfaceType_55B970(char_type* check_mask);
    void ProcessGroundCollisionAndEmitImpactParticles_55BFE0();
    char_type TestCollision_55C150();
    char_type SweepTestMovementForCollision_55C3B0(Fix16* outHitStep, Fix16* outNoHitStep);
    void BinarySearchCollisionTime_55C560(Fix16& low, Fix16& high);
    void HandleMapBoundaryCollisionY_55C5C0(Fix16Point& pPoint, Ang16 angle);
    void HandleMapBoundaryCollisionX_55C820(Fix16Point& pPoint, Ang16 angle);
    void DispatchCollision_55CA70(Fix16Point& point, Ang16 angle);
    void ReplayAndDispatchCollision_55CBB0(Fix16 step_a, Fix16 step_b);
    void SpawnSkidSegment_55D200(s32 box_idx, Fix16Point arg_4, s32 surface);
    void UpdateWheelSkidEffects_55DC00();
    void DoSkidmarks_55E260();
    char_type StepMovementAndCollisions_55E470();
    char_type CheckAndHandleCarAndTrailerCollisions_55EB80();
    void ApplyForwardEngineForce_55EC30();
    void ApplyReverseEngineForce_55EF20();
    void ApplyTurningForce_55F020();
    char_type ApplyMovementCommand_55F240();
    char_type ProcessCollisionAndClampVelocity_55F280();
    void StepPhysics_55F330();
    char_type CheckPendingCollision_55F360();
    void ApplyForceWithTrailerRedirect_55F740(Fix16Point* pPoint, Fix16Point* pForce);
    void ApplyForceAndIntegrate_55F7A0(Fix16Point* pPoint, Fix16Point force);
    void ApplyForceAtPoint_55F800(Fix16Point* pPoint, Fix16Point* pForce, s32 bRotate);
    void AccumulateImpulse_55F930(Fix16Point* pPoint);
    void ApplyAngularImpulse_55F970(Fix16 impulse);
    void ApplyForceScaledByMass_55F9A0(const Fix16Point& pForce);
    void ApplyImpulseWithTrailerRedirect_55FA10(Fix16Point* pPoint);
    Fix16 ApplyImpactForcesAndDamage_55FA60(Fix16Point& PointOfForce, const Fix16Point& Impulse, s32 base_dmg);
    void AccumulateImpulse_55FC30(const Fix16Point& a2, s32 base_dmg);
    void HandleWorldCollision_55FD00(Fix16Point& pHitPoint);
    void HandleCarCollision_55FF20(Car_BC* pOtherCar);
    void HandleObjectCollision_5606C0(Object_2C* p2C, char_type damage_area);
    void ProcessPedImpact_560B40(Char_B4* pCharB4, u8 hitType);
    void UpdateLinearAndAngularAccel_560EB0();
    void ApplyMovementStep_560F20(Fix16 time_scale);
    void IntegrateAndClampVelocities_5610B0();
    Fix16Point ComputeRelativePointVelocity_561130(Fix16Point* pPoint);
    Fix16Point GetPointVelocity_561350(Fix16Point* pPoint);
    Fix16Point ComputePointVelocity_561380(Fix16Point& point);
    Fix16 ApplyDriveForce_5615D0(Fix16Point& wheel_point, Ang16 angle, Fix16Point& friction, Fix16 slip_speed);
    bool get_revs_561940();
    Fix16 ComputeEngineTorque_561970();
    Fix16 ComputeTorqueFromThrottle_561DD0();

    bool IsVelocityAlignedWithHeading_40F840()
    {
        Ang16 v14 = (field_40_linvel_1.Atan2() - field_58_theta);
        return v14 <= kAng90_6FE00C || v14 >= kAng270_6FE154;
    }

    Fix16 inline_ComputeTorqueFromThrottle_561DD0()
    {
        if (get_revs_561940() != 0)
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust +
                ((field_60_gas_pedal * ((gDamageSpeedFactor_6FE348 * gCarInfo_2C_6FE0E4->field_18_fith_thrust)))) * 2;
        }
        else
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust +
                ((field_60_gas_pedal * ((gDamageSpeedFactor_6FE348 * gCarInfo_2C_6FE0E4->field_18_fith_thrust))));
        }
    }

    // ComputeTorqueFromThrottle_561DD0 as inlined into ComputeEngineTorque_561970 (out of line operators)
    Fix16 inline_ComputeTorqueFromThrottle_561DD0_ool()
    {
        if (get_revs_561940() != 0)
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust.Add(
                gCarInfo_2C_6FE0E4->field_18_fith_thrust.Multiply(gDamageSpeedFactor_6FE348).Multiply(field_60_gas_pedal).MultiplyInt(2));
        }
        else
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust.Add(
                gCarInfo_2C_6FE0E4->field_18_fith_thrust.Multiply(gDamageSpeedFactor_6FE348).Multiply(field_60_gas_pedal));
        }
    }

    Fix16 ComputeTorqueUnknown_49E8E0()
    {
        if (get_revs_561940())
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust + (gCarInfo_2C_6FE0E4->field_18_fith_thrust * this->field_60_gas_pedal) * 2;
        }
        else
        {
            return gCarInfo_2C_6FE0E4->field_14_half_thrust + gCarInfo_2C_6FE0E4->field_18_fith_thrust * this->field_60_gas_pedal;
        }
    }

    bool IsDrowning_421100()
    {
        if (field_98_surface_type == 8)
        {
            if (field_40_linvel_1.GetLength() <= kDrowningMaxSpeed_677794)
            {
                return true;
            }
        }
        return false;
    }

    Fix16 CalculateFrontWheelForce_561E50();
    Fix16 CalculateRearWheelForce_5620D0();
    // 0x62450 moved to Sprite
    void ApplyThrottleInput_562480();
    void ApplyBrakePhysics_5624F0();
    void UpdateSteeringAngle_562560();
    bool IsGasPedalPressedEnough_5626A0();
    Fix16 MinGasPedalPressure_5626C0();
    void ApplyArrowSteerAssist_5626F0();
    void StabilizeVelocityAtSpeed_562910();
    void EnforceGearSensitiveMaxSpeed_562D00();
    void SetModelPhysicsGlobal_562EB0();
    void SetCarInfoGlobal_562ED0();
    void SetCurrentCarInfoAndModelPhysics_562EF0();

    void HandleObjectCollisionAt_482CC0(Object_2C* pObj, Fix16Point& point, char_type a3)
    {
        SetCurrentCarInfoAndModelPhysics_562EF0();
        CollisionIntersectionPoint_6FE1A0 = point;
        HandleObjectCollision_5606C0(pObj, a3);
    }
    void ApplyInputsAndIntegratePhysics_562F30();
    char_type UpdateLastMovementTimer_562FA0();
    bool ProcessCarPhysicsStateMachine_562FE0();
    void UpdateCp1FromCm1_563280();
    void UpdateCenterOfMassPoint_563350();
    void UpdateReferencePoint_563460();
    void SetSprite_563560(Sprite* pSprite);
    void SnapVelocityToSpriteDirection_563590(Sprite* pSprt);
    void UpdateSpriteFromPhysics_563670();
    void UpdateCarAndTrailerSpriteFromPhysics_5636C0();
    bool IsNearlyStopped_5636E0();
    void Init_5637A0();
    void PoolAllocate();

    Fix16Point get_linvel_447010();

    void PoolDeallocate()
    {
    }

    void SetCar_5638C0(Car_BC* pBC);
    CarPhysics_B0();

    Fix16 GetLinearSpeed_4211A0()
    {
        return field_40_linvel_1.GetLength();
    }

    // 9.6f 0x49EF50, defined in CarPhysics_B0.cpp
    inline void AddDamage_49EF50(s32 damage);

    void ClearHandBrake_421260()
    {
        field_92_is_hand_brake_on = 0;
    }

    char_type is_backward_gas_on_411810()
    {
        return field_94_is_backward_gas_on;
    }

    Fix16 get_car_velocity_4211C0()
    {
        return field_0_vel_read_only.GetLength();
    }

    void SetField8C_to_1()
    {
        field_8C_state = 1;
    }

    void SetField8C_to_2()
    {
        field_8C_state = 2;
    }

    bool IsStationary_49EF80()
    {
        return field_40_linvel_1.x == kFixZero && field_40_linvel_1.y == kFixZero && field_74_ang_vel_rad == kFixZero;
    }

    void Field74Subtract()
    {
        field_74_ang_vel_rad -= dword_6FE0B0;
    }

    void Field40Add()
    {
        field_40_linvel_1 += stru_6FDF50;
    }

    void Field40Subtract()
    {
        field_40_linvel_1 -= stru_6FDF50;
    }

    void ResetPoint6FDF50()
    {
        stru_6FDF50.Reset();
    }

    void Accelerate_421210()
    {
        field_93_is_forward_gas_on = 1;
        field_91_is_foot_brake_on = 0;
        field_94_is_backward_gas_on = 0;
        field_95 = 0;
    }

    void ForceNeutralInput_42ABC0()
    {
        field_95 = 1;
        field_91_is_foot_brake_on = 0;
        field_94_is_backward_gas_on = 0;
        field_93_is_forward_gas_on = 0;
    }

    void NeutralGear_42AC00()
    {
        field_91_is_foot_brake_on = 0;
        field_93_is_forward_gas_on = 0;
        field_94_is_backward_gas_on = 0;
        field_95 = 0;
    }

    // Unknown 9.6f inlined func
    void Reverse_Unk()
    {
        field_94_is_backward_gas_on = 1;
        field_91_is_foot_brake_on = 0;
        field_93_is_forward_gas_on = 0;
        field_95 = 0;
    }

    void TurnClockwise_42ABA0()
    {
        field_AD_turn_direction = car_turn_direction::clockwise_m1;
    }

    void TurnAntiClockwise_42AB90()
    {
        field_AD_turn_direction = car_turn_direction::anticlockwise_1;
    }

    void SetGoStraight_42ABB0()
    {
        field_AD_turn_direction = 0;
        field_78_pointing_ang_rad = kFixZero;
    }

    Fix16 vec_len_552DE0(); // Char_B4.cpp func

    void ClearDriverInputs_453F90();
    void ForceNeutralInput_453F50();
    void ForceForwardAcceleration_453F70();

    Fix16Point field_0_vel_read_only;
    u32 field_8_total_damage_q;
    CarPhysics_B0* mpNext;
    Fix16Point field_10_last_skid_pos[4];
    Fix16Point field_30_cm1;
    Fix16Point field_38_cp1;
    Fix16Point field_40_linvel_1;
    Fix16Point field_48_force_accum;
    Fix16Point field_50_linear_accel;
    Ang16 field_58_theta;
    s16 field_5A_unused;
    Car_BC* field_5C_pCar;
    Fix16 field_60_gas_pedal;
    Fix16 field_64_brake_pressure;
    Fix16 field_68_z_pos;
    Fix16 field_6C_cp3;
    Fix16 field_70_z_vel;
    Fix16 field_74_ang_vel_rad;
    Fix16 field_78_pointing_ang_rad;
    Fix16 field_7C_torque_accum;
    Fix16 field_80_angular_accel;
    Fix16 field_84_front_skid;
    Fix16 field_88_rear_skid;
    s32 field_8C_state;
    u8 field_90_timer_since_last_move;
    char_type field_91_is_foot_brake_on;
    char_type field_92_is_hand_brake_on;
    char_type field_93_is_forward_gas_on;
    char_type field_94_is_backward_gas_on;
    char_type field_95;
    char_type field_96_unused;
    char_type field_97_unused;
    s32 field_98_surface_type;
    s32 field_9C_block_spec;
    s32 field_A0_oil_spin_dir;
    char_type field_A4_oil_spin_timer;
    u8 field_A5_current_slope_length;
    char_type field_A6_current_slope_left_tiles;
    u8 field_A7_current_tile_z;
    char_type field_A8_hand_brake_force;
    char_type field_A9_car_model;
    char_type field_AA_sbw;
    char_type field_AB_tpa;
    u8 field_AC_drive_wheels_locked_q;
    char_type field_AD_turn_direction;
    char_type field_AE_unused;
    char_type field_AF_unused;
};

class CarPhyisicsPool : public PoolBasic<CarPhysics_B0, 306>
{
  public:
    //Inlined in CarManager_6C constructor 9.6f -> 0x420f8
    CarPhyisicsPool() = default;

    ~CarPhyisicsPool()
    {
        head_ = nullptr;
    }
};

EXTERN_GLOBAL(CarPhyisicsPool*, gCarPhysicsPool_6FE3E0);
