// <mmsystem.h> subset: timeGetTime, and the wave/MIDI types Miles' mss.h refers to.
// Packed to 1 byte, as the SDK header is.
#pragma once
#include <windows.h>

DECLARE_HANDLE(HWAVEOUT);
DECLARE_HANDLE(HWAVEIN);
DECLARE_HANDLE(HMIDIOUT);
DECLARE_HANDLE(HMIDIIN);
typedef HWAVEOUT* LPHWAVEOUT;
typedef HWAVEIN* LPHWAVEIN;
typedef HMIDIOUT* LPHMIDIOUT;
typedef UINT MMRESULT;

#pragma pack(push, 1)

typedef struct wavehdr_tag
{
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD dwUser;
    DWORD dwFlags;
    DWORD dwLoops;
    struct wavehdr_tag* lpNext;
    DWORD reserved;
} WAVEHDR, *PWAVEHDR, *LPWAVEHDR;

typedef struct midihdr_tag
{
    LPSTR lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD dwUser;
    DWORD dwFlags;
    struct midihdr_tag* lpNext;
    DWORD reserved;
    DWORD dwOffset;
    DWORD dwReserved[8];
} MIDIHDR, *PMIDIHDR, *LPMIDIHDR;

typedef struct waveformat_tag
{
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;

typedef struct pcmwaveformat_tag
{
    WAVEFORMAT wf;
    WORD wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, *LPPCMWAVEFORMAT;

typedef struct tWAVEFORMATEX
{
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX* LPCWAVEFORMATEX;

#pragma pack(pop)

#define WAVE_FORMAT_PCM 1
#define WAVE_MAPPER ((UINT)-1)
#define MMSYSERR_NOERROR 0
#define TIMERR_NOERROR 0

#ifdef __cplusplus
extern "C" {
#endif
DWORD timeGetTime(void);
MMRESULT timeBeginPeriod(UINT period);
MMRESULT timeEndPeriod(UINT period);
#ifdef __cplusplus
}
#endif
