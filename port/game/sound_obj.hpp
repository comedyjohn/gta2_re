#pragma once

#include "Function.hpp"
#include "ang16.hpp"
#include "fix16.hpp"
#include "root_sound.hpp" // for SoundObject_10
#include <windows.h>

class Car_BC;
class Object_2C;
class gmp_map_zone;
class CollisionEvent_28;
class SoundObject_10;

class SoundPos_C
{
  public:
    // Compiler-generated constructor at 0x419DF0
    Fix16 field_0_x;
    Fix16 field_4_y;
    Fix16 field_8_z;
};

struct SoundPosEntry_C
{
    // 9.6f 0x416C00
    SoundPosEntry_C()
    {
    }

    Fix16 field_0_x;
    u32 field_4;
    //char_type field_4;
    //char_type field_5;
    //char_type field_6;
    //char_type field_7;
    s16 field_8;
    u8 field_A;
    char_type field_B_unused;
};

struct AudioEntity_C
{
    char_type field_0_bUsed;
    u8 field_1_age;
    char_type field_2_unused;
    char_type field_3;
    SoundObject_10* field_4_pObj;
    SoundPosEntry_C* field_8_pAlloc;
};

struct SoundEntry_1C
{
    char_type field_0;
    char_type field_1_unused;
    char_type field_2_unused;
    char_type field_3;
    s32 field_4_fp;
    SoundObject_10 field_8;
    s32 field_18;
};

struct sound_f16_pos_0x1C
{
    u8 field_0_bUsed;
    u8 field_1_unused;
    u8 field_2_unused;
    u8 field_3;
    
    Fix16 field_4_xpos;
    Fix16 field_8_ypos;

    s32 field_C;
    s16 field_10;
    s16 field_12;
    s32 field_14;
    s32 field_18;
};

struct sound_0x68
{
    s32 field_0_EntityIndex;
    char_type field_4_SampleIndex;
    char_type field_5_unused;
    char_type field_6_unused;
    char_type field_7_unused;
    SoundPos_C field_8_obj;
    u32 field_14_samp_idx;
    char_type field_18_bIs2D;
    char_type field_19_unused;
    char_type field_1A_unused;
    char_type field_1B_unused;
    u32 field_1C_ReleasingVolumeModificator;
    u32 field_20_rate;
    u8 field_24_nVolume;
    char_type field_25_unused;
    char_type field_26_unused;
    char_type field_27_unused;
    Fix16 field_28_distance;
    char_type field_2C_bIsBeingPlayed;
    char_type field_2D_bIsPlayingFinished;
    char_type field_2E_unused;
    char_type field_2F_unused;
    u32 field_30_loop_count;
    s32 field_34_loop_start;
    s32 field_38_loop_end;
    s32 field_3C_speed_multiplier;
    char_type field_40_pan;
    char_type field_41;
    char_type field_42_unused;
    char_type field_43_unused;
    s32 field_44_frames_to_play;
    u32 field_48_nCalculatedVolume;
    u32 field_4C_releasing_volume_divider;
    u8 field_50_volume_change;
    char_type field_51_unused;
    char_type field_52_unused;
    char_type field_53_unused;
    Fix16 field_54_sound_intensity;
    s32 field_58_type;
    sound_0x68* field_5C;
    u8 field_60_nEmittingVolume;
    char_type field_61_unused;
    char_type field_62_unused;
    char_type field_63_unused;
    s32 field_64_max_distance;
};

struct Sound_Params_8
{
    Sprite* field_0_pObj;
    char_type field_4_bDrivenByPlayer;
    char_type field_5_bHasSolidAbove;
    // pad?
    char_type field_6_unused;
    char_type field_7_unused;
};

class sound_obj
{
  public:
    char_type field_0_bSoundInitialized;
    char_type field_1_isPaused;
    char_type field_2_service_enabled;
    char_type field_3;
    s32 field_4_speed_of_sound;
    u8 field_8_frames_per_second;
    char_type field_9_unused;
    char_type field_A;
    char_type field_B_unused;
    s32 field_C;
    u8 field_10_nActiveSamples;
    char_type field_11_unused;
    char_type field_12;
    char_type field_13_unused;
    s32 field_14_sample_rate;
    char_type field_18;
    char_type field_19_unused;
    char_type field_1A_bDoubleVolume;
    char_type field_1B_unused;
    u8 field_1C_samp_count;
    char_type field_1D_b3d_sound;
    char_type field_1E_unused;
    char_type field_1F_unused;
    s32 field_20;
    u8 field_24_sfx_vol;
    u8 field_25_cdVol;
    char_type field_26_unused;
    char_type field_27_unused;
    Fix16 field_28_dist_related;
    char_type field_2C_distCalculated;
    char_type field_2D_unused;
    char_type field_2E_unused;
    char_type field_2F_unused;
    sound_0x68 field_30_sQueueSample;
    u8 field_98_nActiveSampleQueue;
    char_type field_99_pad;
    char_type field_9A_pad;
    char_type field_9B_pad;
    sound_0x68 field_9C_asSamples[2][16];
    u8 field_D9C_abSampleQueueIndexTable[2][16];
    u8 field_DBC_SampleRequestQueuesStatus[2];
    s16 field_DBE_unused;
    sound_0x68 field_DC0_ActiveSamples[16];
    s32 field_1440_unused;
    char_type field_1444_unused;
    char_type field_1445_unused;
    char_type field_1446_unused;
    char_type field_1447_unused;
    s32 field_1448_unused;
    s32 field_144C_unused;
    char_type field_1450_unused;
    char_type field_1451_unused;
    char_type field_1452_unused;
    char_type field_1453_unused;
    u32 field_1454_anRandomTable[5];
    Fix16 field_1468_listener_x;
    Fix16 field_146C_listener_y;
    Fix16 field_1470_listener_z;
    Ang16 field_1474_rotation;
    char_type field_1476_unused;
    char_type field_1477_unused;
    s32 field_1478_type5Idx;
    AudioEntity_C field_147C_audio_entities[1020];
    u32 field_444C_AudioEntityOrderList[1020];
    u32 field_543C_444C_nAudioEntitiesCount;
    s32 field_5440;
    s32 field_5444_unused;
    u32 field_5448_m_FrameCounter;
    SoundEntry_1C field_544C[5]; // sound_f16_pos_0x1C instead of SoundEntry_1C ?

    // Radio emitters use indices 1..5
    inline sound_f16_pos_0x1C& RadioEmitter(s32 idx)
    {
        return (reinterpret_cast<sound_f16_pos_0x1C*>(field_544C))[idx];
    }

    char_type field_54D8_unused[5];
    char_type field_54DD_unused;
    char_type field_54DE_unused;
    char_type field_54DF_unused;
    char_type field_54E0_unused;
    char_type field_54E1_unused;
    char_type field_54E2_unused;
    char_type field_54E3_unused;
    char_type field_54E4_unused;
    char_type field_54E5_unused;
    char_type field_54E6_unused;
    char_type field_54E7_unused;
    s16 field_54E8[5];
    u8 field_54F2[5];
    u8 field_54F7[5];
    s32 field_54FC;
    s32 field_5500;
    char_type field_5504_radio_station_change_mode;
    char_type field_5505_unused;
    char_type field_5506;
    char_type field_5507_unused;
    s32 field_5508_radio_entity_idx;
    SoundObject_10 field_550C_radio_entity;
    s32 field_551C;
    u8 field_5520_bCanPlay;
    u8 field_5521_radio_word;
    s16 field_5522_wanted_star_count;
    s32 field_5524_radio_word;
    u8 field_5528_idx15_cur;
    u8 field_5529_idx15;
    char_type field_552A_pad;
    char_type field_552B_pad;
    s32 field_552C_15array[15];
    char_type field_5568_crime_xpos;
    char_type field_5569_crime_ypos;
    s16 field_556A_erv_en_route_timer;
    s16 field_556C_timer;
    s16 field_556E_timer;
    s16 field_5570_timer;
    s16 field_5572_unused; // pad?
    s32 field_5574_car_info_idx;

    sound_obj();

    // TODO: Ordering
    void Reacquire_41A2A0();
    char_type GetAudioDriveLetter_41A2E0();
    char_type Get3dSound_41A390();
    void Service_419EF0();
    void Release_41A290();
    u8 GetCDVol_41A280();
    void Init_419E10();
    s32 get_samp_idx_for_car_417D70(Car_BC* pCar, bool bDoorOpening, bool bTrainOrBus);
    s32 samp_idx_for_model_417AC0(s32 car_model);
    s32 GetSirenSampleIdx_417B80(s32 car_model, bool bHornOn);

    void null_412240();
    void null_412250();
    char_type InitSampleForType_412260(sound_0x68* pObj);
    void ProcessEntity_4123A0(s32 idx);
    void ProcessType2_412490(s32 idx);
    void ProcessType1_Sprite_412740(s32 idx);
    void ProcessType8_Crane_412820(s32 idx);
    void ProcessType9_Crusher_412A60(s32 idx);
    void ProcessCar_412B80(Sound_Params_8* pType2Entity);
    char_type Type6_412C90(CollisionEvent_28* pEvent, u8 base_sample);
    char_type SelectObjectImpactSound_1_10_412D30(CollisionEvent_28* pEvent);
    char_type Type6_2_412D40(u8 roll);
    void Type6_Play_412D90(s32 model);
    char_type Type6_3_413000(CollisionEvent_28* pEvent);
    char_type Type6_4_413040(u8 roll);
    char_type Type6_5_413090(u8 roll);
    char_type Type6_7_4130E0(CollisionEvent_28* pEvent);
    char_type SelectObjectImpactSound_413120(CollisionEvent_28* pEvent, s32 interactionType);
    char_type Type6_9_413540(CollisionEvent_28* pEvent);
    char_type Type6_12_4136D0(CollisionEvent_28* pEvent);
    void ProcessType6_CollisionSoundQueue_413760(s32 idx);
    char_type Type6_413A10(CollisionEvent_28* pEvent);
    void ProcessTrainCab_413B90(Sound_Params_8* pParams, SoundPosEntry_C* pAlloc);
    void ProcessTrain_413BE0(Sound_Params_8* pParams);
    void ProcessTank_413BF0(Sound_Params_8* pParams, SoundPosEntry_C* pAlloc);
    void ProcessOtherCarTypes_413C50(Sound_Params_8* pParams, SoundPosEntry_C* pAlloc);
    void HandleAICarHornBeep_413D10(Sound_Params_8* pParams);
    void HandleTrainEngineSound_4140C0(Sound_Params_8* pParams);

    char_type Type_16_414320(sound_0x68* pObj);
    void HandleTrainCabRollingFrictionSound_4143A0(Sound_Params_8* pParams);
    bool GetCar_4145E0(s32 idx, Car_BC** ppOut);
    char_type Type_17_414690(sound_0x68* pObj);
    void TrainCab_414710(Sound_Params_8* pParams);
    char_type Type_19_4149D0(sound_0x68* pObj);
    void Tank_414A50(Sound_Params_8* pParams);
    char_type Type_12_414C90(sound_0x68* pSound);
    void Tank_414D30(Sound_Params_8* pParams);
    char_type Type_11_414EE0(sound_0x68* pSound);
    void HandleCarBurningSound_414F90(Sound_Params_8* pParams);
    char_type Type_15_415100(sound_0x68* pSound);
    void Tank_415190(Sound_Params_8* pParams);
    char_type Type_13_4153F0(sound_0x68* pObj);
    void HandleCarWeaponHitSound_415480(Sound_Params_8* pParams);
    void HandleCarAlarmSound_415570(Sound_Params_8* pParams, SoundPosEntry_C* pAlloc);
    char_type Type_5_InitEngineSoundProfile_415730(sound_0x68* pSound);
    void HandleCarEngineSound_4157C0(Sound_Params_8* pParams);
    char_type Type_1_6_416260(sound_0x68* pSound);
    void HandleCarHornSound_417060(Sound_Params_8* pParams);
    char_type Type_3_HandleCarImpactSound_4174C0(sound_0x68* pSound);
    void HandleCarDamageSound_4177D0(Sound_Params_8* pParams);
    void HandleSirenActivationSound_4178C0(Sound_Params_8* pParams);
    char_type Type_4_417A00(sound_0x68* pSound);
    s32 GetVehicleAudioClass_417BA0(s32 car_model);
    s32 ConvertToPlayBackRate_417C60(s32 car_model);
    void HandleHeavyVehicleStopSound_417E30(Sound_Params_8* pParams, SoundPosEntry_C* pAlloc);
    s32 Type_7_417EF0(sound_0x68* pObj);
    char_type IsHeavyTruckOrBus_417F40(s32 car_model);
    void HandleTruckCorneringAudio_417FD0(Sound_Params_8* pParams);
    char_type Type_8_418130(sound_0x68* pObj);
    void HandleAICarEngineSound_418190(Sound_Params_8* pParams);
    char_type Type_2_4182A0(sound_0x68* pObj);
    void HandleCarDoorSounds_4182E0(Sound_Params_8* pParams);
    void UpdateCarSurfaceAudio_418610(Sound_Params_8* pParams);
    char_type Type_9_4186D0(sound_0x68* pObj);
    void HandleCarTireScrubSound_418720(Sound_Params_8* pParams);
    char_type Type_10_HandleCarSkidSound_418940(sound_0x68* pSound);
    void ProcessType11_HudPager_418B60(s32 entity_idx);
    void AddVocalsEntity_418C20();
    void RemoveVocalsEntity_418C60();
    void PlayVocal_418C80(s32 vocal_idx);
    void ProcessType10_Vocals_418CA0();
    char_type CalculateDistance_419020(Fix16 max_dist);
    bool VolCalc_419070(u8 base_volume, Fix16 max_dist, char_type bFlag);
    Fix16 ComputeEmitterDistanceSquared_4190B0();
    s32 AddSoundObject_419FA0(SoundObject_10* pTuring);

    void FreeSoundEntry_41A090(u32 idx);
    char_type LoadStyle_41A1B0(const char_type* pStyleName);
    const char_type* GetFileName_41A1E0(const char_type* Str);
    void SetSfxVol_41A250(u8 sfxVol);
    void SetCDVol_41A270(u8 cdVol);
    char_type Set3DSound_41A2F0(char_type b3dSound);
    void ServiceSoundEffects_41A3A0();
    char_type CalcVolume_41A3F0(u8 base_volume, Fix16 max_dist, Fix16 dist);
    u8 ComputePan_41A4A0(Fix16 a1, Fix16 a2);
    s32 AdjustPlaybackRate_41A580(s32 snd_rate, Fix16 xpos, Fix16 ypos, Fix16 zpos);
    s32 RandomDisplacement_41A650(u32 seed);
    void ResetEntry_41A6C0(s32 idx);
    void IncrementAudioEntitiesAge_41A6F0();
    void InterrogateAudioEntities_41A730();
    void AddSampleToRequestedQueue_41A850();
    void AddDetailsToRequestedOrderList_41A910(u8 sample);
    void AddReleasingSounds_41A9D0();
    void ProcessActiveQueues_41AB80();

    void UpdateFramesToPlay_41B490(sound_0x68* pObj);
    void VecDiff_41B4E0(SoundPos_C* pVec, SoundPos_C* pRet);
    void FromFix16_41B520(Fix16 value, f32* pOut);
    void AdjustSamplesVolume_41B540();
    char_type ComputeEmittingVolume_41B660(u8 emittingVolume, u32 maxDistance, u32 distance);
    void ClearRequestedQueue_41B700();
    void ClearActivateSamples_41B7A0();
    void GenerateIntegerRandomNumberTable_41BA90();

    void ProcessObject_41E820(Sound_Params_8* pEntity);
    void ProcessObject_Type12_41E850(Sound_Params_8* pParams);
    void ProcessOtherObjects_41F520(Sound_Params_8* pParams);
    void UpdateRadioChatterLoop_41FCA0();

    void ProcessPed_422B70(Sound_Params_8* pType3Entity);
    void HandlePedVoiceEvent_423080(Sound_Params_8* pParams);

    void ResetRadioMessageState_426750();
    void PoliceRadioMessageGeneration_426790();
    void EnqueueRadioLocationPhrase_426E10(u8 xpos, u8 ypos);
    void GenerateRadioVehicleDescription_426F20(Car_BC* pCar);

    void Init_RadioMessageQueue_427180();
    void EnqueueRadioWord_4271B0(u32 val);
    void ProcessPoliceRadioWordsPlayback_427220();
    u8 GetQueuedRadioWordCount_427310();
    void EnqueueRadioCrimeCallout_427340(s32 crime_word, u8 loc_a, u8 loc_b);
    void AppendRadioMessageSuffix_4273B0();
    void nullsub_4();
    u32 GetCopRadioZoneIndex_427400(u8 x, u8 y, gmp_map_zone** ppZone);

    void ProcessType7_Weapon_42A500(s32 idx);

    void ProcessType3_CopRadioAndMusic_57DD50();
    void HandleVocalStreamSwitching_57DF10(char_type bFastForward);
    void UpdateCarEngineAudio_57E220();
    void UpdateVocalStream_57E510();
    void Type3_CopRadioReport_57E680();
    void ChooseRadioEmitterForVehicle_57E6C0();
    void InitMusicAndCopRadio_57E960();
    void DeInitVocals_57EA10();
    void UpdateActiveRadioEmitterVolume_57EA90();
    char_type ComputeRadioEmitterVolume_57EB90(u8 emitterIndex, s32 bUseFarRadius);
    void DeclareRadioStation_57ECB0(s32 station_idx, Fix16 xpos, Fix16 ypos);
    void sub_57EDB0(sound_f16_pos_0x1C* pEmitter, s32 type);
    void RemoveSound_57EE30(Fix16 xpos, Fix16 ypos);
    void CycleRadioStation_57EEE0(char_type bPrev);
    void SelectBestRadioEmitter_57EF60();
    u8 FindEmitterByStatus_57F050(s32 status);
    bool IsPoliceOrServiceVehicle_57F090(Car_BC* pCar);
    bool IsTrainOrBoxcar_57F120(Car_BC* pCar);
};

EXTERN_GLOBAL(sound_obj, gSound_obj_66F680);
