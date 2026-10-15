#pragma once

#include "3rdParty/mss.h"
#include "Function.hpp"

struct sdt_entry_0x18
{
    s32 field_0_offset;
    s32 field_4_sample_length;
    s32 field_8_playBackRate;
    s32 field_C_random_displacement;
    s32 field_10_loop_start;
    s32 field_14_loop_end; // -1 = end of sample
};

class cSampleManager
{
  public:
    cSampleManager();

    char_type DisableA3DSplash_58D620();

    char_type SoundInit_58D6C0(s32* a2);

    char_type OpenDigitalDriver_58D720(char_type bFlag, char_type bStereo, s32 sampleRate);

    // todo: order

    char_type AllocSamples_58D9F0(s32 a2);

    void PlaySampleRange_58E8C0(u32 idx, u32 end_idx);

    char_type Init3DProvider_58D820(BYTE* pMaxSamples);

    void Reset3DSamples_58D960();

    void AllocSample_58DA80();

    void ReleaseSample_58DAC0();

    void Terminate_58DAE0();

    void Shutdown_58DB30();

    s32 GetPlayBackRateIdx_58DBF0(s32 idx);

    s32 GetRandomDisplacement_58DC10(s32 idx);

    s32 GetLoopStart_58DC30(s32 sample_idx);

    s32 GetLoopEnd_58DC50(s32 sample_idx);

    s32 GetSampleLength_maybe_58DC70(s32 sample_idx);

    void InitialiseChannel_58DC90(s32 channel, s32 sample_idx);

    void SetChannelVolume_58DCE0(s32 channel, s32 volume);

    void SetChannelPan_58DD00(s32 channel, s32 pan);

    void SetChannelFrequency_58DD20(s32 channel, s32 frequency);

    void SetChannelLoopPoints_58DD40(s32 channel, s32 startOffset, s32 endOffset);

    void SetChannelLoopCount_58DD60(s32 channel, s32 loopCount);

    bool GetChannelUsedFlag_58DD80(s32 channel);

    void StartChannel_58DDB0(s32 channel);

    void StopChannel_58DDD0(s32 channel);

    bool InitialiseChannel3D_58DDF0(s32 channel, s32 nSfx, s32 rate);

    void SetChannel3DVolume_58DE80(s32 idx, s32 vol);

    void SetChannel3DPosition_58DEA0(s32 channel, f32 x, f32 y, f32 z);

    void SetChannel3DDistances_58DED0(s32 channel, f32 maxDist, f32 minDist);

    void SetChannel3DFrequency_58DF00(s32 channel, s32 freq);

    void SetChannel3DLoopPoints_58DF20(s32 channel, s32 loop_start, s32 loop_end);

    void SetChannel3DLoopCount_58DF50(s32 channel, s32 loop_count);

    bool GetChannel3DUsedFlag_58DF70(s32 channel);

    void StartChannel3D_58DFA0(s32 samp_idx);

    void StopChannel3D_58DFC0(s32 samp_idx);

    u8 Get3DSampleCount_58E000();
    void SetEaxEnvironment_58E010(s32 env_idx);

    char_type Open3DProvider_58E140(s32 envIdx);

    // todo order
    void Close3DProvider_58E1C0();

    // todo order
    void Enum3DProviders_58E1F0();

    // order func

    bool StreamStatus_58E2C0();

    void StreamSetVolume_58E2F0(u8 vol);

    void OpenStream_58E320(u32 stream_idx);

    void CloseStream_58E460();

    void FadeOut_58E490();

    char_type MusicFileExists_58E500();

    void PlayVocal_58E510(s32 stream_idx, s32 voc_idx, char_type bAppendA);

    void CloseVocalStream_58E6A0(s32 stream_idx);

    void SetVocalVolume_58E6D0(s32 stream_idx, u8 vol);

    void SetVocalSpeed_58E700(s32 stream_idx, s32 playbackRate);

    s32 GetVocalSpeed_58E720(s32 stream_idx);

    void SetVocalPosMs_58E750(s32 stream_idx, s32 ms_pos);

    s32 GetVocalPosMs_58E770(long stream_idx);

    s32 GetVocalLengthMs_58E7A0(long stream_idx);

    void SetSampleVol_58E7D0(u8 vol);

    void PlayAtIdx_58E7F0(s32 idx);

    bool SampleNotDone_58E880();
    
    void EndSample_58E8A0();

    void EndSample_58E960();

    char_type LoadWavSdtData_58E980(const char_type* pRawOrSdtName);

    BYTE Reacquire_58E2A0();

    void Release_58E290();

    HDIGDRIVER field_0_hDriver;
    char_type field_4_gtaAudioDriveLetter;
    char_type field_5_str[80];
    bool field_55_bMusicLoaded;
    short field_56_pad;
    HSAMPLE field_58_hSamples[16];
    HSAMPLE field_98_hSample;
    HSTREAM field_9C_hStreams[2];
    char_type field_A4_bLoaded;
    char_type field_A5_pad;
    char_type field_A6_pad;
    char_type field_A7_pad;
    sdt_entry_0x18 field_A8_sdt_entries[320];
    void* field_1EA8_pAudioBuffer1;
    void* field_1EAC_pAudioBuffer2;
    u8 field_1EB0_count_samples;
    u8 field_1EB1_bytes_per_sample;
    u8 field_1EB2_3d_samp_count;
    char_type field_1EB3_pad;
    HPROVIDER field_1EB4_h3dProvider[256];
    char_type* field_22B4_str[256];
    HPROVIDER field_26B4_env_idx;
    s32 field_26B8_bEaxSupported;
    s32 field_26BC_eax_environment;
    s32 field_26C0_3d_provider;
    H3DSAMPLE field_26C4_3d_sample[16];
    f32 field_2704_float;
    f32 field_2708_float;
    f32 field_270C_float;
    s32 field_2710_3d_provider_count;
    char_type field_2714_bUnknown;
    char_type field_2715_pad;
    char_type field_2716_pad;
    char_type field_2717_pad;
};

EXTERN_GLOBAL(cSampleManager, gSampManager_6FFF00);