// gta2 --audiotest: plays samples and streams through the AIL_* functions the way cSampleManager
// does, mixes them without a device (port_audio_mix) and checks what comes out: volume, pan, master
// volume, loop counts and loop blocks, playback rates, 8-bit data, and WAV streams in PCM and IMA
// ADPCM (the format of the game's music). Needs no display, no sound card and no game data.
#include "3rdParty/mss.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

extern bool port_audio_manual;
void port_audio_mix(int16_t* out, int frames);

namespace
{
int gFailures = 0;

void Check(bool ok, const char* what, long expected, long got)
{
    gFailures += ok ? 0 : 1;
    printf("%s %-58s expected %6ld, got %6ld\n", ok ? "PASS" : "FAIL", what, expected, got);
}

void CheckNear(const char* what, long expected, long got, long tolerance = 2)
{
    Check(labs(expected - got) <= tolerance, what, expected, got);
}

std::vector<int16_t> Mix(int frames)
{
    std::vector<int16_t> out(static_cast<size_t>(frames) * 2);
    port_audio_mix(out.data(), frames);
    return out;
}

// Frames of a mix in which the left channel isn't silent
int Audible(const std::vector<int16_t>& mix)
{
    int n = 0;
    for (size_t i = 0; i < mix.size(); i += 2)
    {
        n += mix[i] != 0 || mix[i + 1] != 0 ? 1 : 0;
    }
    return n;
}

// Plays a sample until it is done (at most `limit` output frames); returns the frames it was audible
int PlayOut(HSAMPLE s, int limit = 100000)
{
    int audible = 0;
    for (int done = 0; done < limit && AIL_sample_status(s) == SMP_PLAYING; done += 64)
    {
        audible += Audible(Mix(64));
    }
    return audible;
}

void Put16(std::vector<uint8_t>& b, uint32_t v)
{
    b.push_back(v & 0xFF);
    b.push_back((v >> 8) & 0xFF);
}

void Put32(std::vector<uint8_t>& b, uint32_t v)
{
    Put16(b, v & 0xFFFF);
    Put16(b, v >> 16);
}

void PutTag(std::vector<uint8_t>& b, const char* tag)
{
    b.insert(b.end(), tag, tag + 4);
}

// A RIFF WAVE file from a fmt chunk body and the data
std::vector<uint8_t> Wave(const std::vector<uint8_t>& fmt, const std::vector<uint8_t>& data, uint32_t factFrames)
{
    std::vector<uint8_t> b;
    PutTag(b, "RIFF");
    Put32(b, 0);
    PutTag(b, "WAVE");
    PutTag(b, "fmt ");
    Put32(b, static_cast<uint32_t>(fmt.size()));
    b.insert(b.end(), fmt.begin(), fmt.end());
    if (factFrames)
    {
        PutTag(b, "fact");
        Put32(b, 4);
        Put32(b, factFrames);
    }
    PutTag(b, "data");
    Put32(b, static_cast<uint32_t>(data.size()));
    b.insert(b.end(), data.begin(), data.end());
    const uint32_t riff = static_cast<uint32_t>(b.size() - 8);
    memcpy(&b[4], &riff, 4);
    return b;
}

std::vector<uint8_t> PcmWave(int channels, int rate, const std::vector<int16_t>& samples)
{
    std::vector<uint8_t> fmt;
    Put16(fmt, 1);
    Put16(fmt, channels);
    Put32(fmt, rate);
    Put32(fmt, rate * channels * 2);
    Put16(fmt, channels * 2);
    Put16(fmt, 16);
    std::vector<uint8_t> data;
    for (int16_t s : samples)
    {
        Put16(data, static_cast<uint16_t>(s));
    }
    return Wave(fmt, data, 0);
}

// Mono IMA ADPCM whose every nibble is 0: at step index 0 that adds nothing, so each block decodes
// to its header's predictor, held
std::vector<uint8_t> ImaWave(int rate, int16_t value, int blocks)
{
    const int blockAlign = 256;
    const int perBlock = (blockAlign - 4) * 2 + 1;
    std::vector<uint8_t> fmt;
    Put16(fmt, 0x11);
    Put16(fmt, 1);
    Put32(fmt, rate);
    Put32(fmt, rate * blockAlign / perBlock);
    Put16(fmt, blockAlign);
    Put16(fmt, 4);
    Put16(fmt, 2);
    Put16(fmt, perBlock);
    std::vector<uint8_t> data;
    for (int i = 0; i < blocks; i++)
    {
        Put16(data, static_cast<uint16_t>(value));
        data.push_back(0);
        data.push_back(0);
        data.insert(data.end(), blockAlign - 4, 0);
    }
    return Wave(fmt, data, static_cast<uint32_t>(perBlock * blocks));
}

void WriteFile(const std::string& path, const std::vector<uint8_t>& bytes)
{
    FILE* f = fopen(path.c_str(), "wb");
    fwrite(bytes.data(), 1, bytes.size(), f);
    fclose(f);
}

HSAMPLE NewSample(HDIGDRIVER drv, const void* data, U32 len, S32 format, U32 flags, S32 rate)
{
    HSAMPLE s = AIL_allocate_sample_handle(drv);
    AIL_init_sample(s);
    AIL_set_sample_type(s, format, flags);
    AIL_set_sample_address(s, const_cast<void*>(data), len);
    AIL_set_sample_playback_rate(s, rate);
    return s;
}
} // namespace

int port_audiotest()
{
    port_audio_manual = true;
    AIL_startup();

    WAVEFORMAT format = {};
    format.wFormatTag = 1;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    HDIGDRIVER drv = 0;
    const bool opened = AIL_waveOutOpen(&drv, 0, -1, &format) == 0 && drv;
    Check(opened, "AIL_waveOutOpen", 1, opened);

    // 100 frames of 16-bit mono at half scale
    std::vector<int16_t> half(100, 16384);
    HSAMPLE s = NewSample(drv, half.data(), 200, DIG_F_MONO_16, DIG_PCM_SIGN, 22050);
    Check(AIL_sample_status(s) == SMP_DONE, "a new sample is done", SMP_DONE, AIL_sample_status(s));
    AIL_start_sample(s);
    Check(AIL_sample_status(s) == SMP_PLAYING, "started: playing", SMP_PLAYING, AIL_sample_status(s));
    std::vector<int16_t> m = Mix(50);
    CheckNear("full volume, centre: left", 16384, m[0]);
    CheckNear("full volume, centre: right", 16384, m[1]);
    Check(AIL_sample_status(s) == SMP_PLAYING, "half way: still playing", SMP_PLAYING, AIL_sample_status(s));
    m = Mix(60);
    Check(Audible(m) == 50, "plays its 100 frames, then stops", 50, Audible(m));
    Check(AIL_sample_status(s) == SMP_DONE, "then done", SMP_DONE, AIL_sample_status(s));

    AIL_set_sample_volume(s, 64);
    AIL_set_sample_pan(s, 0);
    AIL_start_sample(s);
    m = Mix(10);
    CheckNear("volume 64 of 127, pan 0: left", 16384 * 64 / 127, m[0]);
    Check(m[1] == 0, "pan 0: right silent", 0, m[1]);
    AIL_set_sample_pan(s, 127);
    m = Mix(10);
    Check(m[0] == 0, "pan 127: left silent", 0, m[0]);
    AIL_end_sample(s);
    Check(AIL_sample_status(s) == SMP_DONE, "AIL_end_sample: done", SMP_DONE, AIL_sample_status(s));
    Check(Audible(Mix(10)) == 0, "AIL_end_sample: silent", 0, Audible(Mix(10)));
    AIL_set_sample_volume(s, 127);
    AIL_set_sample_pan(s, 64);

    AIL_set_sample_loop_count(s, 3);
    AIL_start_sample(s);
    const int looped = PlayOut(s);
    Check(looped == 300, "loop count 3: 300 frames", 300, looped);
    AIL_set_sample_loop_count(s, 0);
    AIL_start_sample(s);
    Mix(5030);
    Check(AIL_sample_status(s) == SMP_PLAYING, "loop count 0: still playing after 5000", SMP_PLAYING, AIL_sample_status(s));
    AIL_set_sample_loop_count(s, 1);
    const int tail = PlayOut(s);
    Check(tail == 70, "then loop count 1: finishes the pass it is in", 70, tail);
    Check(AIL_sample_status(s) == SMP_DONE, "and is done", SMP_DONE, AIL_sample_status(s));

    AIL_set_sample_loop_count(s, 1);
    AIL_set_sample_playback_rate(s, 11025);
    AIL_start_sample(s);
    const int slow = PlayOut(s);
    Check(slow == 200, "half the rate: twice the frames", 200, slow);
    AIL_set_sample_playback_rate(s, 22050);

    // A loop block over the second half: the whole sample, then the block once more
    std::vector<int16_t> twoPart(100, 8000);
    for (int i = 50; i < 100; i++)
    {
        twoPart[i] = -8000;
    }
    AIL_set_sample_address(s, twoPart.data(), 200);
    AIL_set_sample_loop_block(s, 100, -1);
    AIL_set_sample_loop_count(s, 2);
    AIL_start_sample(s);
    m = Mix(160);
    Check(Audible(m) == 150, "loop block [50,end) twice: 150 frames", 150, Audible(m));
    Check(m[2 * 120] < 0, "the repeat is the block", -8000, m[2 * 120]);
    AIL_set_sample_loop_block(s, 0, -1);

    // 8-bit unsigned data, as AIL_set_sample_type(.., 0, 0) gives
    std::vector<uint8_t> u8(100, 192);
    HSAMPLE s8 = NewSample(drv, u8.data(), 100, DIG_F_MONO_8, 0, 22050);
    AIL_start_sample(s8);
    m = Mix(10);
    CheckNear("8-bit unsigned 192: half scale", 16384, m[0]);
    AIL_end_sample(s8);

    // Two at once add up, and the master volume scales both
    AIL_set_sample_address(s, half.data(), 200);
    AIL_start_sample(s);
    AIL_start_sample(s8);
    m = Mix(10);
    CheckNear("two samples mix", 32767, m[0]);
    AIL_set_digital_master_volume(drv, 0);
    m = Mix(10);
    Check(Audible(m) == 0, "master volume 0: silent", 0, Audible(m));
    AIL_set_digital_master_volume(drv, 127);
    AIL_end_sample(s);
    AIL_end_sample(s8);
    AIL_release_sample_handle(s8);

    // Streams, from files the way the game names them (backslashes, any case)
    char dir[] = "/tmp/gta2-audiotest-XXXXXX";
    if (!mkdtemp(dir))
    {
        printf("FAIL can't make a temporary directory\n");
        return 1;
    }
    char cwd[4096];
    if (!getcwd(cwd, sizeof(cwd)) || chdir(dir) != 0)
    {
        printf("FAIL can't enter %s\n", dir);
        return 1;
    }
    mkdir("audio", 0755);

    std::vector<int16_t> stereo;
    for (int i = 0; i < 1103; i++)
    {
        stereo.push_back(8192);
        stereo.push_back(-8192);
    }
    WriteFile("audio/pcm.wav", PcmWave(2, 11025, stereo));
    HSTREAM st = AIL_open_stream(drv, const_cast<char*>("AUDIO\\PCM.WAV"), 0);
    Check(st != 0, "AIL_open_stream: 16-bit stereo PCM", 1, st != 0);
    if (st)
    {
        S32 total = 0;
        AIL_stream_ms_position(st, &total, 0);
        CheckNear("its length in ms", 100, total, 1);
        Check(AIL_stream_playback_rate(st) == 11025, "its rate", 11025, AIL_stream_playback_rate(st));
        Check(AIL_stream_status(st) != SMP_PLAYING, "not playing before a start", 0, AIL_stream_status(st) == SMP_PLAYING);
        AIL_set_stream_loop_count(st, 1);
        AIL_start_stream(st);
        m = Mix(100);
        CheckNear("stream: left", 8192, m[100]);
        CheckNear("stream: right", -8192, m[101]);
        S32 current = 0;
        AIL_stream_ms_position(st, 0, &current);
        CheckNear("position after 100 frames at 22050", 4, current, 1);
        m = Mix(2300);
        const int audible = Audible(m) + 100;
        CheckNear("plays 1103 frames at 11025: 2206 at 22050", 2206, audible, 2);
        Check(AIL_stream_status(st) == SMP_DONE, "then done", SMP_DONE, AIL_stream_status(st));

        AIL_set_stream_loop_count(st, 0);
        AIL_start_stream(st);
        Mix(10000);
        Check(AIL_stream_status(st) == SMP_PLAYING, "loop count 0: still playing", SMP_PLAYING, AIL_stream_status(st));
        AIL_set_stream_volume(st, 32);
        Check(AIL_stream_volume(st) == 32, "AIL_stream_volume", 32, AIL_stream_volume(st));
        m = Mix(10);
        CheckNear("volume 32", 8192 * 32 / 127, m[0]);
        AIL_set_stream_ms_position(st, 50);
        AIL_stream_ms_position(st, 0, &current);
        CheckNear("AIL_set_stream_ms_position", 50, current, 1);
        AIL_close_stream(st);
    }

    WriteFile("audio/ima.wav", ImaWave(22050, 10000, 4));
    st = AIL_open_stream(drv, const_cast<char*>("audio\\ima.wav"), 0);
    Check(st != 0, "AIL_open_stream: IMA ADPCM", 1, st != 0);
    if (st)
    {
        AIL_set_stream_loop_count(st, 1);
        AIL_start_stream(st);
        m = Mix(100);
        CheckNear("IMA ADPCM decodes", 10000, m[100], 4);
        AIL_close_stream(st);
    }

    Check(AIL_open_stream(drv, const_cast<char*>("audio\\missing.wav"), 0) == 0, "a missing file: no stream", 0, 0);

    // The music, which the game looks for as X:\GTAUDIO\ on every drive: a folder in the game's folder
    mkdir("gtaudio", 0755);
    WriteFile("gtaudio/1.wav", PcmWave(2, 11025, stereo));
    st = AIL_open_stream(drv, const_cast<char*>("C:\\GTAUDIO\\1.wav"), 0);
    Check(st != 0, "C:\\GTAUDIO\\1.wav is gtaudio/1.wav in the game folder", 1, st != 0);
    AIL_close_stream(st);
    unlink("gtaudio/1.wav");
    rmdir("gtaudio");

    unlink("audio/pcm.wav");
    unlink("audio/ima.wav");
    rmdir("audio");
    if (chdir(cwd) != 0)
    {
        return 1;
    }
    rmdir(dir);

    AIL_release_sample_handle(s);
    AIL_waveOutClose(drv);

    // Through a real device, if there is one (SDL_AUDIODRIVER=dummy will do): a short sample
    // finishes, which means SDL's thread is pulling the mix
    port_audio_manual = false;
    if (AIL_waveOutOpen(&drv, 0, -1, &format) == 0)
    {
        s = NewSample(drv, half.data(), 200, DIG_F_MONO_16, DIG_PCM_SIGN, 22050);
        AIL_start_sample(s);
        for (int i = 0; i < 100 && AIL_sample_status(s) == SMP_PLAYING; i++)
        {
            usleep(10000);
        }
        Check(AIL_sample_status(s) == SMP_DONE, "a device: plays a sample to its end", SMP_DONE, AIL_sample_status(s));
        AIL_release_sample_handle(s);
        AIL_waveOutClose(drv);
    }
    else
    {
        printf("SKIP no audio device\n");
    }
    AIL_shutdown();

    printf("%s: %d failure(s)\n", gFailures ? "FAILED" : "OK", gFailures);
    return gFailures ? 1 : 0;
}
