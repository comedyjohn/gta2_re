#pragma once

#include "SoundObject_10.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"
#include <windows.h>

class root_sound
{
  public:
    SoundObject_10* field_0_pFreeList;
    SoundObject_10 field_4_pool[999 + 1];

    SoundObject_10* DestroySoundObj_40FE60(SoundObject_10* pSoundObj) // inline
    {
        pSoundObj->Release_40EF20();
        SoundObject_10* result = field_0_pFreeList;
        pSoundObj->field_C_pAny.pNextFree = field_0_pFreeList;
        field_0_pFreeList = pSoundObj;
        return result;
    }

    SoundObject_10* PopFree_410730()
    {
        SoundObject_10* pCurrent = field_0_pFreeList;
        field_0_pFreeList = pCurrent->field_C_pAny.pNextFree;
        pCurrent->field_8_sound_entry = 0; // TODO: 9.6f calls SoundObject_10::sub_4106D0 (not dumped)
        return pCurrent;
    }

    SoundObject_10* CreateSoundObject_40EF40(void* pObject, s32 objectType);

    void Init_40EF80();

    void Service_40EFA0();

    s32 AddSoundObject_40EFB0(SoundObject_10* pSoundObj);

    void FreeSoundEntry_40EFD0(s32 sound_entry);

    char_type LoadStyle_40EFF0(const char_type* pStyleName);

    void InitMusicAndCopRadio_40F010();

    void DeInitVocals_40F020();

    void DeclareRadioStation_40F030(s32 station_idx, Fix16 xpos, Fix16 ypos);

    void RemoveSound_40F050(Fix16 sound_id, Fix16 emitter);

    void CycleRadioStation_40F070(char_type direction);

    void PlayVoice_40F090(s32 state);

    void SetSfxVol_40F0B0(u8 sfxVol);

    void SetCDVol_40F0F0(u8 cdVol);

    u8 GetCDVol_40F120();

    void Release_40F130();

    void Reacquire_40F140();

    char_type GetAudioDriveLetter_40F150();

    char_type Set3DSound_40F160(char_type b3dSound);

    char_type Get3DSound_40F180();

    root_sound();

    ~root_sound();

    void DeinitializeAudio();
};

EXTERN_GLOBAL(root_sound, gRoot_sound_66B038);