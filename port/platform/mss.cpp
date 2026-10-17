// The Miles Sound System functions the game calls (AIL_*), over SDL audio: the 2D samples it plays
// its sound effects on, the streams it plays music and voices from, and the digital driver they mix
// into. Replaces the VC6 build's stubs (Source/3rdParty/mss.cpp) in the native build.
//
// Miles' semantics, as the game relies on them:
// - volume and pan are 0..127 (pan 64 is the centre), the master volume scales everything;
// - a loop count of 0 loops forever, n plays n times; a loop block repeats [start, end) in bytes, an
//   end of -1 (or anything not past the start) meaning the end of the data;
// - status is SMP_PLAYING while playing, SMP_DONE once finished or ended, SMP_STOPPED before a start;
// - AIL_delay counts 60ths of a second.
//
// No 3D providers are offered, so the game keeps to its 2D samples (cSampleManager::Init3DProvider_58D820
// looks for "Microsoft DirectSound3D hardware support" and gives up without it).
#include "path.h"
#include "platform.h"

#include "3rdParty/mss.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

bool port_audio_manual; // --audiotest: no device; the test pulls frames with port_audio_mix

// The audio event log (see port_audio_log_open): what the game's sound code asked the mixer to do,
// hashed into a running value the replay tests compare, and optionally written out as text.
long port_audio_frame; // the update the replay is at, for the text log
static uint64_t gAudioHash = 14695981039346656037ULL; // FNV-1a basis
static uint64_t gAudioEvents;
static FILE* gAudioText;

namespace
{
const int kMaxSamples = 64;
const int kMaxStreams = 8;

// A 2D sample: the game's own PCM data, played in place
struct Sample
{
    bool mAllocated;
    U32 mStatus;
    const U8* mData;
    U32 mLen; // bytes
    S32 mFormat; // DIG_F_*
    U32 mFlags; // DIG_PCM_SIGN for signed 8-bit data
    S32 mRate;
    S32 mVolume;
    S32 mPan;
    S32 mLoopCount;
    S32 mLoopStart; // bytes
    S32 mLoopEnd;
    S32 mLoopsLeft; // passes still to play after this one, -1 forever
    double mPos; // frames
};

// A stream: a WAV file, decoded when it is opened
struct Stream
{
    bool mAllocated;
    U32 mStatus;
    plat::Wav mWav;
    S32 mRate;
    S32 mVolume;
    S32 mPan;
    S32 mLoopCount;
    S32 mLoopsLeft;
    double mPos;
};

struct Driver
{
    bool mOpen;
    bool mDevice; // an SDL device is open (not in --audiotest)
    int mOutRate;
    S32 mMasterVolume;
    Sample mSamples[kMaxSamples];
    Stream mStreams[kMaxStreams];
};

Driver gDriver;

struct Lock
{
    Lock()
    {
        plat::LockAudio();
    }
    ~Lock()
    {
        plat::UnlockAudio();
    }
};

enum AudioEvent
{
    EV_SET_ADDRESS = 1,
    EV_START_SAMPLE,
    EV_END_SAMPLE,
    EV_SAMPLE_RATE,
    EV_SAMPLE_VOLUME,
    EV_SAMPLE_PAN,
    EV_SAMPLE_LOOP_COUNT,
    EV_SAMPLE_LOOP_BLOCK,
    EV_MASTER_VOLUME,
    EV_OPEN_STREAM,
    EV_CLOSE_STREAM,
    EV_START_STREAM,
    EV_STREAM_VOLUME,
    EV_STREAM_RATE,
    EV_STREAM_LOOP_COUNT,
    EV_STREAM_POSITION,
};

const char* const kAudioEventNames[] = {"",
                                        "set_address",
                                        "start_sample",
                                        "end_sample",
                                        "sample_rate",
                                        "sample_volume",
                                        "sample_pan",
                                        "sample_loop_count",
                                        "sample_loop_block",
                                        "master_volume",
                                        "open_stream",
                                        "close_stream",
                                        "start_stream",
                                        "stream_volume",
                                        "stream_rate",
                                        "stream_loop_count",
                                        "stream_position"};

void HashValue(uint64_t v)
{
    for (int i = 0; i < 8; i++)
    {
        gAudioHash = (gAudioHash ^ ((v >> (8 * i)) & 0xFF)) * 1099511628211ULL;
    }
}

// Handles are slots of the driver's tables, so the log is the same from run to run (never addresses)
void LogAudio(AudioEvent ev, int slot, int64_t a = 0, int64_t b = 0)
{
    gAudioEvents++;
    HashValue(ev);
    HashValue(static_cast<uint64_t>(slot));
    HashValue(static_cast<uint64_t>(a));
    HashValue(static_cast<uint64_t>(b));
    if (gAudioText)
    {
        fprintf(gAudioText,
                "%ld %s %d %lld %lld\n",
                port_audio_frame,
                kAudioEventNames[ev],
                slot,
                static_cast<long long>(a),
                static_cast<long long>(b));
    }
}

int SlotOf(HSAMPLE s)
{
    return static_cast<int>(reinterpret_cast<Sample*>(s) - gDriver.mSamples);
}

int SlotOf(HSTREAM s)
{
    return static_cast<int>(reinterpret_cast<Stream*>(s) - gDriver.mStreams);
}

Sample* ToSample(HSAMPLE s)
{
    return reinterpret_cast<Sample*>(s);
}

Stream* ToStream(HSTREAM s)
{
    return reinterpret_cast<Stream*>(s);
}

void PanGains(S32 pan, float volume, float& left, float& right)
{
    const float p = static_cast<float>(pan < 0 ? 0 : pan > 127 ? 127 : pan);
    left = volume * (p <= 64.0F ? 1.0F : (127.0F - p) / 63.0F);
    right = volume * (p >= 64.0F ? 1.0F : p / 64.0F);
}

float Volume(S32 v)
{
    return static_cast<float>(v < 0 ? 0 : v > 127 ? 127 : v) / 127.0F;
}

int SampleChannels(const Sample& s)
{
    return (s.mFormat & DIG_F_STEREO_MASK) ? 2 : 1;
}

int SampleBytesPerFrame(const Sample& s)
{
    return SampleChannels(s) * ((s.mFormat & DIG_F_16BITS_MASK) ? 2 : 1);
}

// One frame of a sample as floats in -1..1
void SampleFrame(const Sample& s, U32 frame, float& l, float& r)
{
    const int channels = SampleChannels(s);
    if (s.mFormat & DIG_F_16BITS_MASK)
    {
        const U8* p = s.mData + frame * 2U * channels;
        l = static_cast<float>(static_cast<S16>(p[0] | (p[1] << 8))) / 32768.0F;
        r = channels == 2 ? static_cast<float>(static_cast<S16>(p[2] | (p[3] << 8))) / 32768.0F : l;
    }
    else
    {
        const U8* p = s.mData + frame * channels;
        const bool sign = (s.mFlags & DIG_PCM_SIGN) != 0;
        l = sign ? static_cast<float>(static_cast<signed char>(p[0])) / 128.0F : (static_cast<float>(p[0]) - 128.0F) / 128.0F;
        r = l;
        if (channels == 2)
        {
            r = sign ? static_cast<float>(static_cast<signed char>(p[1])) / 128.0F : (static_cast<float>(p[1]) - 128.0F) / 128.0F;
        }
    }
}

void StreamFrame(const Stream& s, U32 frame, float& l, float& r)
{
    const S16* p = s.mWav.samples + static_cast<size_t>(frame) * s.mWav.channels;
    l = static_cast<float>(p[0]) / 32768.0F;
    r = s.mWav.channels >= 2 ? static_cast<float>(p[1]) / 32768.0F : l;
}

// Mixes a voice into `mix`, following its loops; returns false once it has finished. `frames` is the
// voice's length, [loopStart, loopEnd) the part it repeats, `frameAt` reads one frame.
template<class Voice, class FrameAt>
bool MixVoice(Voice& v,
              U32 frames,
              U32 loopStart,
              U32 loopEnd,
              float gainL,
              float gainR,
              int outRate,
              float* mix,
              int count,
              FrameAt frameAt)
{
    if (frames == 0 || v.mRate <= 0)
    {
        return false;
    }
    const double step = static_cast<double>(v.mRate) / outRate;
    for (int i = 0; i < count; i++)
    {
        // A pass ends at the loop's end while there are passes left after it, else at the data's end
        const U32 end = v.mLoopsLeft != 0 ? loopEnd : frames;
        if (v.mPos >= end)
        {
            if (v.mLoopsLeft == 0)
            {
                return false;
            }
            if (v.mLoopsLeft > 0)
            {
                v.mLoopsLeft--;
            }
            v.mPos = loopStart + (v.mPos - end);
            if (v.mPos >= loopEnd)
            {
                v.mPos = loopStart;
            }
        }
        const U32 f0 = static_cast<U32>(v.mPos);
        U32 f1 = f0 + 1;
        if (f1 >= (v.mLoopsLeft != 0 ? loopEnd : frames))
        {
            f1 = v.mLoopsLeft != 0 ? loopStart : f0;
        }
        const float t = static_cast<float>(v.mPos - f0);
        float l0, r0, l1, r1;
        frameAt(f0, l0, r0);
        frameAt(f1, l1, r1);
        mix[i * 2] += (l0 + (l1 - l0) * t) * gainL;
        mix[i * 2 + 1] += (r0 + (r1 - r0) * t) * gainR;
        v.mPos += step;
    }
    return true;
}

void MixInto(int16_t* out, int count)
{
    std::vector<float> mix(static_cast<size_t>(count) * 2, 0.0F);
    const float master = Volume(gDriver.mMasterVolume);

    for (Sample& s : gDriver.mSamples)
    {
        if (!s.mAllocated || s.mStatus != SMP_PLAYING || !s.mData)
        {
            continue;
        }
        const int bpf = SampleBytesPerFrame(s);
        const U32 frames = s.mLen / bpf;
        U32 loopStart = s.mLoopStart > 0 ? static_cast<U32>(s.mLoopStart) / bpf : 0;
        U32 loopEnd = s.mLoopEnd > 0 ? static_cast<U32>(s.mLoopEnd) / bpf : frames;
        if (loopEnd > frames || loopEnd <= loopStart)
        {
            loopEnd = frames;
        }
        if (loopStart >= loopEnd)
        {
            loopStart = 0;
        }
        float gl, gr;
        PanGains(s.mPan, Volume(s.mVolume) * master, gl, gr);
        if (!MixVoice(s,
                      frames,
                      loopStart,
                      loopEnd,
                      gl,
                      gr,
                      gDriver.mOutRate,
                      mix.data(),
                      count,
                      [&s](U32 f, float& l, float& r)
                      {
                          SampleFrame(s, f, l, r);
                      }))
        {
            s.mStatus = SMP_DONE;
        }
    }

    for (Stream& s : gDriver.mStreams)
    {
        if (!s.mAllocated || s.mStatus != SMP_PLAYING)
        {
            continue;
        }
        float gl, gr;
        PanGains(s.mPan, Volume(s.mVolume) * master, gl, gr);
        if (!MixVoice(s,
                      s.mWav.frames,
                      0,
                      s.mWav.frames,
                      gl,
                      gr,
                      gDriver.mOutRate,
                      mix.data(),
                      count,
                      [&s](U32 f, float& l, float& r)
                      {
                          StreamFrame(s, f, l, r);
                      }))
        {
            s.mStatus = SMP_DONE;
        }
    }

    for (size_t i = 0; i < mix.size(); i++)
    {
        const float v = mix[i] * 32767.0F;
        out[i] = static_cast<int16_t>(v > 32767.0F ? 32767 : v < -32768.0F ? -32768 : lrintf(v));
    }
}

void AudioCallback(int16_t* out, int frames, void*)
{
    MixInto(out, frames);
}

S32 LoopsLeft(S32 loopCount)
{
    return loopCount <= 0 ? -1 : loopCount - 1;
}

U32 StreamLengthMs(const Stream& s)
{
    return s.mWav.rate > 0 ? static_cast<U32>(static_cast<unsigned long long>(s.mWav.frames) * 1000 / s.mWav.rate) : 0;
}
} // namespace

// --audiotest: mixes `frames` frames of the current state, as the device would pull them
void port_audio_mix(int16_t* out, int frames)
{
    Lock lock;
    MixInto(out, frames);
}

int port_audio_rate()
{
    return gDriver.mOutRate;
}

DXDEC S32 AILCALL AIL_enumerate_3D_providers(HPROENUM FAR* next, HPROVIDER FAR* dest, C8 FAR* FAR* name)
{
    return 0;
}

DXDEC void FAR* AILCALL AIL_mem_alloc_lock(U32 size)
{
    return malloc(size);
}

DXDEC void AILCALL AIL_mem_free_lock(void FAR* ptr)
{
    free(ptr);
}

DXDEC S32 AILCALL AIL_startup(void)
{
    return 1;
}

DXDEC S32 AILCALL AIL_get_preference(U32 number)
{
    return 0;
}

DXDEC void AILCALL AIL_shutdown(void)
{
    AIL_waveOutClose(reinterpret_cast<HDIGDRIVER>(&gDriver));
}

DXDEC S32 AILCALL AIL_set_preference(U32 number, S32 value)
{
    return 0;
}

DXDEC void AILCALL AIL_delay(S32 intervals)
{
    if (intervals > 0)
    {
        Sleep(static_cast<DWORD>(intervals) * 1000 / 60);
    }
}

// The game opens 22050 Hz 16-bit stereo; the device may run at another rate, voices are resampled
DXDEC S32 AILCALL AIL_waveOutOpen(HDIGDRIVER FAR* drvr, LPHWAVEOUT FAR* lphWaveOut, S32 wDeviceID, LPWAVEFORMAT lpFormat)
{
    if (gDriver.mOpen)
    {
        AIL_waveOutClose(reinterpret_cast<HDIGDRIVER>(&gDriver));
    }
    const int rate = lpFormat && lpFormat->nSamplesPerSec ? static_cast<int>(lpFormat->nSamplesPerSec) : 22050;
    gDriver.mMasterVolume = 127;
    if (port_audio_manual)
    {
        gDriver.mOutRate = rate;
        gDriver.mDevice = false;
    }
    else
    {
        int actual = rate;
        if (!plat::OpenAudio(rate, AudioCallback, nullptr, actual))
        {
            return 1; // the game carries on without sound
        }
        gDriver.mOutRate = actual;
        gDriver.mDevice = true;
    }
    gDriver.mOpen = true;
    *drvr = reinterpret_cast<HDIGDRIVER>(&gDriver);
    return 0;
}

DXDEC void AILCALL AIL_waveOutClose(HDIGDRIVER drvr)
{
    if (!gDriver.mOpen)
    {
        return;
    }
    if (gDriver.mDevice)
    {
        plat::CloseAudio(); // stops the callback first
        gDriver.mDevice = false;
    }
    for (Stream& s : gDriver.mStreams)
    {
        if (s.mAllocated)
        {
            plat::FreeWav(s.mWav);
            s.mAllocated = false;
        }
    }
    // Sample handles stay allocated: cSampleManager::OpenDigitalDriver_58D720 keeps field_98_hSample
    // across a close and a reopen, so a handle has to stay usable rather than alias a new one
    for (Sample& s : gDriver.mSamples)
    {
        s.mStatus = s.mAllocated ? SMP_DONE : SMP_FREE;
    }
    gDriver.mOpen = false;
}

DXDEC S32 AILCALL AIL_digital_handle_release(HDIGDRIVER drvr)
{
    plat::PauseAudio(true);
    return 1;
}

DXDEC S32 AILCALL AIL_digital_handle_reacquire(HDIGDRIVER drvr)
{
    plat::PauseAudio(false);
    return 1;
}

DXDEC HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER dig)
{
    if (!gDriver.mOpen)
    {
        return 0;
    }
    Lock lock;
    for (Sample& s : gDriver.mSamples)
    {
        if (!s.mAllocated)
        {
            memset(&s, 0, sizeof(s));
            s.mAllocated = true;
            s.mStatus = SMP_DONE;
            return reinterpret_cast<HSAMPLE>(&s);
        }
    }
    return 0;
}

DXDEC void AILCALL AIL_release_sample_handle(HSAMPLE S)
{
    if (S)
    {
        Lock lock;
        ToSample(S)->mAllocated = false;
        ToSample(S)->mStatus = SMP_FREE;
    }
}

DXDEC void AILCALL AIL_init_sample(HSAMPLE S)
{
    if (S)
    {
        Lock lock;
        Sample& s = *ToSample(S);
        s.mStatus = SMP_DONE;
        s.mData = 0;
        s.mLen = 0;
        s.mFormat = DIG_F_MONO_16;
        s.mFlags = 0;
        s.mRate = 22050;
        s.mVolume = 127;
        s.mPan = 64;
        s.mLoopCount = 1;
        s.mLoopStart = 0;
        s.mLoopEnd = -1;
        s.mPos = 0;
    }
}

DXDEC void AILCALL AIL_set_sample_address(HSAMPLE S, void FAR* start, U32 len)
{
    if (S)
    {
        Lock lock;
        Sample& s = *ToSample(S);
        LogAudio(EV_SET_ADDRESS, SlotOf(S), len);
        s.mStatus = SMP_DONE; // a new address stops what was playing
        s.mData = static_cast<const U8*>(start);
        s.mLen = len;
        s.mPos = 0;
    }
}

DXDEC void AILCALL AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags)
{
    if (S)
    {
        Lock lock;
        ToSample(S)->mFormat = format;
        ToSample(S)->mFlags = flags;
    }
}

DXDEC void AILCALL AIL_start_sample(HSAMPLE S)
{
    if (S)
    {
        Lock lock;
        Sample& s = *ToSample(S);
        LogAudio(EV_START_SAMPLE, SlotOf(S), s.mLen, s.mRate);
        s.mPos = 0;
        s.mLoopsLeft = LoopsLeft(s.mLoopCount);
        s.mStatus = s.mData && s.mLen ? SMP_PLAYING : SMP_DONE;
    }
}

DXDEC void AILCALL AIL_end_sample(HSAMPLE S)
{
    if (S)
    {
        Lock lock;
        LogAudio(EV_END_SAMPLE, SlotOf(S));
        ToSample(S)->mStatus = SMP_DONE;
    }
}

DXDEC void AILCALL AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate)
{
    if (S)
    {
        Lock lock;
        LogAudio(EV_SAMPLE_RATE, SlotOf(S), playback_rate);
        ToSample(S)->mRate = playback_rate;
    }
}

DXDEC void AILCALL AIL_set_sample_volume(HSAMPLE S, S32 volume)
{
    if (S)
    {
        Lock lock;
        LogAudio(EV_SAMPLE_VOLUME, SlotOf(S), volume);
        ToSample(S)->mVolume = volume;
    }
}

DXDEC void AILCALL AIL_set_sample_pan(HSAMPLE S, S32 pan)
{
    if (S)
    {
        Lock lock;
        LogAudio(EV_SAMPLE_PAN, SlotOf(S), pan);
        ToSample(S)->mPan = pan;
    }
}

DXDEC void AILCALL AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count)
{
    if (S)
    {
        Lock lock;
        Sample& s = *ToSample(S);
        LogAudio(EV_SAMPLE_LOOP_COUNT, SlotOf(S), loop_count);
        s.mLoopCount = loop_count;
        if (s.mStatus == SMP_PLAYING)
        {
            s.mLoopsLeft = LoopsLeft(loop_count); // e.g. a looping sound told to finish its current pass
        }
    }
}

DXDEC void AILCALL AIL_set_sample_loop_block(HSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
    if (S)
    {
        Lock lock;
        LogAudio(EV_SAMPLE_LOOP_BLOCK, SlotOf(S), loop_start_offset, loop_end_offset);
        ToSample(S)->mLoopStart = loop_start_offset;
        ToSample(S)->mLoopEnd = loop_end_offset;
    }
}

DXDEC U32 AILCALL AIL_sample_status(HSAMPLE S)
{
    if (!S)
    {
        return SMP_DONE;
    }
    Lock lock;
    return ToSample(S)->mStatus;
}

DXDEC void AILCALL AIL_set_digital_master_volume(HDIGDRIVER dig, S32 master_volume)
{
    Lock lock;
    LogAudio(EV_MASTER_VOLUME, 0, master_volume);
    gDriver.mMasterVolume = master_volume;
}

DXDEC HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char FAR* filename, S32 stream_mem)
{
    if (!gDriver.mOpen || !filename)
    {
        return 0;
    }
    FILE* f = fopen(port_resolve_path(filename).c_str(), "rb");
    if (!f)
    {
        return 0;
    }
    std::vector<U8> bytes;
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    rewind(f);
    if (size > 0)
    {
        bytes.resize(static_cast<size_t>(size));
        bytes.resize(fread(bytes.data(), 1, bytes.size(), f));
    }
    fclose(f);

    plat::Wav wav;
    if (bytes.empty() || !plat::DecodeWav(bytes.data(), static_cast<uint32_t>(bytes.size()), wav))
    {
        return 0;
    }

    Lock lock;
    for (Stream& s : gDriver.mStreams)
    {
        if (!s.mAllocated)
        {
            memset(&s, 0, sizeof(s));
            s.mAllocated = true;
            s.mStatus = SMP_STOPPED;
            s.mWav = wav;
            s.mRate = wav.rate;
            s.mVolume = 127;
            s.mPan = 64;
            s.mLoopCount = 1;
            uint64_t nameHash = 14695981039346656037ULL; // which file, by its name's characters
            for (const char* c = filename; *c; c++)
            {
                nameHash = (nameHash ^ static_cast<unsigned char>(*c)) * 1099511628211ULL;
            }
            LogAudio(EV_OPEN_STREAM, SlotOf(reinterpret_cast<HSTREAM>(&s)), static_cast<int64_t>(nameHash), wav.frames);
            return reinterpret_cast<HSTREAM>(&s);
        }
    }
    plat::FreeWav(wav);
    return 0;
}

DXDEC void AILCALL AIL_close_stream(HSTREAM stream)
{
    if (stream)
    {
        plat::Wav wav;
        {
            Lock lock;
            Stream& s = *ToStream(stream);
            LogAudio(EV_CLOSE_STREAM, SlotOf(stream));
            wav = s.mWav;
            s.mAllocated = false;
            s.mStatus = SMP_FREE;
            s.mWav.samples = nullptr;
        }
        plat::FreeWav(wav);
    }
}

DXDEC void AILCALL AIL_start_stream(HSTREAM stream)
{
    if (stream)
    {
        Lock lock;
        Stream& s = *ToStream(stream);
        LogAudio(EV_START_STREAM, SlotOf(stream), s.mWav.frames);
        s.mPos = 0;
        s.mLoopsLeft = LoopsLeft(s.mLoopCount);
        s.mStatus = SMP_PLAYING;
    }
}

DXDEC void AILCALL AIL_set_stream_volume(HSTREAM stream, S32 volume)
{
    if (stream)
    {
        Lock lock;
        LogAudio(EV_STREAM_VOLUME, SlotOf(stream), volume);
        ToStream(stream)->mVolume = volume;
    }
}

DXDEC S32 AILCALL AIL_stream_volume(HSTREAM stream)
{
    if (!stream)
    {
        return 0;
    }
    Lock lock;
    return ToStream(stream)->mVolume;
}

DXDEC void AILCALL AIL_set_stream_playback_rate(HSTREAM stream, S32 rate)
{
    if (stream)
    {
        Lock lock;
        LogAudio(EV_STREAM_RATE, SlotOf(stream), rate);
        ToStream(stream)->mRate = rate;
    }
}

DXDEC S32 AILCALL AIL_stream_playback_rate(HSTREAM stream)
{
    if (!stream)
    {
        return 0;
    }
    Lock lock;
    return ToStream(stream)->mRate;
}

DXDEC void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count)
{
    if (stream)
    {
        Lock lock;
        Stream& s = *ToStream(stream);
        LogAudio(EV_STREAM_LOOP_COUNT, SlotOf(stream), count);
        s.mLoopCount = count;
        if (s.mStatus == SMP_PLAYING)
        {
            s.mLoopsLeft = LoopsLeft(count);
        }
    }
}

DXDEC S32 AILCALL AIL_stream_status(HSTREAM stream)
{
    if (!stream)
    {
        return SMP_DONE;
    }
    Lock lock;
    return ToStream(stream)->mStatus;
}

DXDEC void AILCALL AIL_set_stream_ms_position(HSTREAM S, S32 milliseconds)
{
    if (S)
    {
        Lock lock;
        Stream& s = *ToStream(S);
        LogAudio(EV_STREAM_POSITION, SlotOf(S), milliseconds);
        const double frame = static_cast<double>(milliseconds < 0 ? 0 : milliseconds) * s.mWav.rate / 1000.0;
        s.mPos = frame < s.mWav.frames ? frame : s.mWav.frames;
    }
}

DXDEC void AILCALL AIL_stream_ms_position(HSTREAM S, //)
                                          S32 FAR* total_milliseconds,
                                          S32 FAR* current_milliseconds)
{
    if (!S)
    {
        return;
    }
    Lock lock;
    const Stream& s = *ToStream(S);
    if (total_milliseconds)
    {
        *total_milliseconds = static_cast<S32>(StreamLengthMs(s));
    }
    if (current_milliseconds)
    {
        *current_milliseconds = s.mWav.rate > 0 ? static_cast<S32>(s.mPos * 1000.0 / s.mWav.rate) : 0;
    }
}

DXDEC M3DRESULT AILCALL AIL_open_3D_provider(HPROVIDER lib)
{
    return 0;
}

DXDEC void AILCALL AIL_close_3D_provider(HPROVIDER lib)
{
}

DXDEC void AILCALL AIL_3D_provider_attribute(HPROVIDER lib, const C8 FAR* name, void FAR* val)
{
}

DXDEC void AILCALL AIL_set_3D_provider_preference(HPROVIDER lib, const C8 FAR* name, void FAR* val)
{
}

DXDEC H3DSAMPLE AILCALL AIL_allocate_3D_sample_handle(HPROVIDER lib)
{
    return 0;
}

DXDEC void AILCALL AIL_release_3D_sample_handle(H3DSAMPLE S)
{
}

DXDEC void AILCALL AIL_start_3D_sample(H3DSAMPLE S)
{
}

DXDEC void AILCALL AIL_end_3D_sample(H3DSAMPLE S)
{
}

DXDEC S32 AILCALL AIL_set_3D_sample_info(H3DSAMPLE S, AILSOUNDINFO FAR* info)
{
    return 0;
}

DXDEC void AILCALL AIL_set_3D_sample_volume(H3DSAMPLE S, S32 volume)
{
}

DXDEC void AILCALL AIL_set_3D_sample_playback_rate(H3DSAMPLE S, S32 playback_rate)
{
}

DXDEC void AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE S, U32 loops)
{
}

DXDEC void AILCALL AIL_set_3D_sample_loop_block(H3DSAMPLE S, S32 loop_start_offset, S32 loop_end_offset)
{
}

DXDEC U32 AILCALL AIL_3D_sample_status(H3DSAMPLE S)
{
    return 0;
}

DXDEC void AILCALL
AIL_3D_sample_float_distances(H3DSAMPLE S, F32 FAR* max_front_dist, F32 FAR* min_front_dist, F32 FAR* max_back_dist, F32 FAR* min_back_dist)
{
}

DXDEC void AILCALL AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z)
{
}

DXDEC void AILCALL
AIL_set_3D_sample_float_distances(H3DSAMPLE S, F32 max_front_dist, F32 min_front_dist, F32 max_back_dist, F32 min_back_dist)
{
}
// The audio event log: a running hash of every call the game's sound code made to the mixer, and how
// many there were. Hashed values only, so it is the same in every build; sample completion (which
// depends on the mixer thread's timing) is deliberately not part of it, only what the game asked for.
uint64_t port_audio_event_hash(uint64_t* events)
{
    if (events)
    {
        *events = gAudioEvents;
    }
    return gAudioHash;
}

// Also writes every call as a line of text: "update event slot arg1 arg2"
void port_audio_log_open(FILE* f)
{
    gAudioText = f;
}
