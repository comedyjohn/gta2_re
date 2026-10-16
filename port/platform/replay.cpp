// gta2 --replay FILE: plays a recorded replay (test\replay.rep, see InputRecorder_67F8B0) without
// anyone at the keyboard and logs the state hash (statehash.h) after every simulation update. Two
// builds that log the same for the same replays compute the same game. port/tools/replaytest.sh runs
// a corpus of them; port/refactor/0-safety-net.md has the background.
//
// The game runs in a scratch copy of the game folder (links to the real files, with its own "test"
// and "player" folders) and with a scratch registry, so a replay run never touches the real settings,
// saves or replay. The debug settings it sets: play_replay, do_exit_after_replay, skip_frontend and
// (unless --draw) skip_draw, which updates without waiting for the clock and never draws.
//
// gta2 --record FILE [--map wil|ste|bil|NAME]: plays the game normally, straight into a level, and
// writes the replay of the session to FILE (debug setting do_release_replay). The registry is a
// copy of the real one (controls, screen), without its debug settings. With --hash-log it logs the
// hashes while playing: playing the replay back must log exactly the same.
#include "port_replay.h"
#include "statehash.h"

#include "Game_0x40.hpp"
#include "InputRecorder_67F8B0.hpp"
#include "rng.hpp"

#include <dirent.h>
#include <ftw.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>

// No std::filesystem: the prelude's _WIN32 gives it wchar_t paths

// The audio event log (platform/mss.cpp)
extern long port_audio_frame;
uint64_t port_audio_event_hash(uint64_t* events);
void port_audio_log_open(FILE* f);
extern bool port_audio_manual;
void port_audio_mix(int16_t* out, int frames);
int port_audio_rate();

namespace
{
struct Options
{
    std::string replay;
    std::string record; // absolute
    std::string map = "wil";
    std::string gameDir = ".";
    std::string hashLog; // empty: stdout when replaying, none when recording
    std::string audioLog; // every call the sound code makes to the mixer, as text (empty: none)
    long frames = -1; // stop after this many updates
    long hashEvery = 1;
    long breakAt = -1; // dump the state after this update, then stop
    std::vector<std::pair<long, std::string>> shots; // --screenshot N:FILE
    bool draw = false;
    bool keep = false; // keep the scratch folder
};

bool gActive;
Options gOptions;
FILE* gLog;
long gFrame; // simulation updates so far
std::string gScratch;
u32 gLastRecord; // replay records applied before this update
long gSyncMismatches;
size_t gShotsTaken; // screenshots written
bool gFinished; // the game returned normally
volatile sig_atomic_t gStopRequested; // Ctrl+C or SIGTERM: end the game at the next update

void OnStopSignal(int)
{
    gStopRequested = 1;
}

const char* const kUsage = "usage: gta2 --replay FILE [--game-dir DIR] [--hash-log FILE] [--frames N]\n"
                           "                         [--hash-every K] [--break-at FRAME] [--audio-log FILE] [--draw] [--keep]\n"
                           "                         [--screenshot N:FILE ...]  (with --draw: the picture after update N, as PPM)\n"
                           "       gta2 --record FILE [--map wil|ste|bil|NAME] [--game-dir DIR] [--hash-log FILE]\n";

bool ParseLong(const char* s, long& out)
{
    char* end;
    out = strtol(s, &end, 10);
    return *s && !*end;
}

bool Parse(int argc, char** argv, Options& o, std::string& error)
{
    for (int i = 1; i < argc; i++)
    {
        const std::string a = argv[i];
        const bool hasValue = i + 1 < argc;
        if (a == "--replay" && hasValue)
        {
            o.replay = argv[++i];
        }
        else if (a == "--record" && hasValue)
        {
            o.record = argv[++i];
            if (o.record[0] != '/')
            {
                char cwd[4096];
                o.record = std::string(getcwd(cwd, sizeof(cwd)) ? cwd : ".") + "/" + o.record;
            }
        }
        else if (a == "--map" && hasValue)
        {
            o.map = argv[++i];
        }
        else if (a == "--game-dir" && hasValue)
        {
            o.gameDir = argv[++i];
        }
        else if (a == "--hash-log" && hasValue)
        {
            o.hashLog = argv[++i];
        }
        else if (a == "--frames" && hasValue)
        {
            if (!ParseLong(argv[++i], o.frames))
            {
                error = "--frames needs a number";
                return false;
            }
        }
        else if (a == "--audio-log" && hasValue)
        {
            o.audioLog = argv[++i];
        }
        else if (a == "--hash-every" && hasValue)
        {
            if (!ParseLong(argv[++i], o.hashEvery) || o.hashEvery < 1)
            {
                error = "--hash-every needs a number of 1 or more";
                return false;
            }
        }
        else if (a == "--break-at" && hasValue)
        {
            if (!ParseLong(argv[++i], o.breakAt))
            {
                error = "--break-at needs a frame number";
                return false;
            }
        }
        else if (a == "--screenshot" && hasValue)
        {
            const std::string v = argv[++i];
            const size_t colon = v.find(':');
            long n = 0;
            if (colon == std::string::npos || !ParseLong(v.substr(0, colon).c_str(), n) || colon + 1 >= v.size())
            {
                error = "--screenshot needs N:FILE";
                return false;
            }
            o.shots.emplace_back(n, v.substr(colon + 1));
        }
        else if (a == "--draw")
        {
            o.draw = true;
        }
        else if (a == "--keep")
        {
            o.keep = true;
        }
        else
        {
            error = "unknown or incomplete option " + a;
            return false;
        }
    }
    if (o.replay.empty() == o.record.empty())
    {
        error = "give either --replay or --record";
        return false;
    }
    return true;
}

// The real registry file (platform/win32.cpp), before XDG_CONFIG_HOME is moved
std::string RealRegistryFile()
{
    const char* base = getenv("XDG_CONFIG_HOME");
    const char* home = getenv("HOME");
    return (base && *base ? std::string(base) : std::string(home ? home : ".") + "/.config") + "/gta2-native/registry.txt";
}

const char* const kDebugKey = "hkey_local_machine\\software\\dma design ltd\\gta2\\debug";

void WriteDword(FILE* f, const char* name)
{
    fprintf(f, "%s\t%s\t4\t01000000\n", kDebugKey, name);
}

// A string value (type 1), with its terminating zero
void WriteString(FILE* f, const char* name, const std::string& value)
{
    fprintf(f, "%s\t%s\t1\t", kDebugKey, name);
    for (char c : value)
    {
        fprintf(f, "%02x", static_cast<unsigned char>(c));
    }
    fprintf(f, "00\n");
}

// The registry file of win32.cpp: key, value name, type (4: DWORD, 1: string) and the value's bytes
// in hex
bool WriteRegistry(const std::string& file, const Options& o)
{
    FILE* f = fopen(file.c_str(), "w");
    if (!f)
    {
        return false;
    }
    if (!o.record.empty())
    {
        // The player's own settings, without debug settings that would change the game
        if (FILE* real = fopen(RealRegistryFile().c_str(), "r"))
        {
            char line[4096];
            while (fgets(line, sizeof(line), real))
            {
                if (strncmp(line, kDebugKey, strlen(kDebugKey)) != 0)
                {
                    fputs(line, f);
                }
            }
            fclose(real);
        }
        WriteDword(f, "do_release_replay");
        WriteDword(f, "skip_frontend");
        // A known level, or any map with a style and script of the same name
        std::string base = o.map;
        if (base == "1" || base == "downtown")
        {
            base = "wil";
        }
        else if (base == "2" || base == "residential")
        {
            base = "ste";
        }
        else if (base == "3" || base == "industrial")
        {
            base = "bil";
        }
        WriteString(f, "mapname", base + ".gmp");
        WriteString(f, "stylename", base + ".sty");
        WriteString(f, "scriptname", base + ".scr");
    }
    else
    {
        WriteDword(f, "play_replay");
        WriteDword(f, "do_exit_after_replay");
        WriteDword(f, "skip_frontend");
        if (!o.draw)
        {
            WriteDword(f, "skip_draw");
        }
    }
    fclose(f);
    return true;
}

std::string Absolute(const std::string& path)
{
    char* real = realpath(path.c_str(), nullptr);
    if (!real)
    {
        return "";
    }
    std::string result = real;
    free(real);
    return result;
}

bool CopyFile(const std::string& from, const std::string& to)
{
    FILE* in = fopen(from.c_str(), "rb");
    if (!in)
    {
        return false;
    }
    FILE* out = fopen(to.c_str(), "wb");
    bool ok = out != nullptr;
    char buffer[65536];
    size_t n;
    while (ok && (n = fread(buffer, 1, sizeof(buffer), in)) > 0)
    {
        ok = fwrite(buffer, 1, n, out) == n;
    }
    fclose(in);
    if (out)
    {
        ok = fclose(out) == 0 && ok;
    }
    return ok;
}

// The files of a folder (not its subfolders: the game's player folder has none)
bool CopyFolder(const std::string& from, const std::string& to)
{
    if (mkdir(to.c_str(), 0755) != 0)
    {
        return false;
    }
    DIR* dir = opendir(from.c_str());
    if (!dir)
    {
        return false;
    }
    bool ok = true;
    while (const dirent* e = readdir(dir))
    {
        const std::string src = from + "/" + e->d_name;
        struct stat st;
        if (stat(src.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        {
            ok = CopyFile(src, to + "/" + e->d_name) && ok;
        }
    }
    closedir(dir);
    return ok;
}

std::string Lower(std::string s)
{
    for (char& c : s)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

// A folder that plays the game in `gameDir` with the replay as test\replay.rep
bool MakeScratch(const Options& o, std::string& error)
{
    const std::string game = Absolute(o.gameDir);
    const std::string replay = o.replay.empty() ? "" : Absolute(o.replay);
    struct stat st;
    if (!o.replay.empty() && (replay.empty() || stat(replay.c_str(), &st) != 0 || !S_ISREG(st.st_mode)))
    {
        error = "no replay at " + o.replay;
        return false;
    }
    DIR* dir = game.empty() ? nullptr : opendir(game.c_str());
    if (!dir)
    {
        error = "can't read the game folder " + o.gameDir;
        return false;
    }

    const char* tmp = getenv("TMPDIR");
    std::string templ = std::string(tmp && *tmp ? tmp : "/tmp") + "/gta2-replay-XXXXXX";
    if (!mkdtemp(templ.data()))
    {
        error = "can't create a scratch folder in " + templ;
        closedir(dir);
        return false;
    }
    gScratch = templ;
    const std::string scratchGame = gScratch + "/game";
    mkdir(scratchGame.c_str(), 0755);
    mkdir((gScratch + "/config").c_str(), 0755);
    mkdir((gScratch + "/config/gta2-native").c_str(), 0755);

    bool hasData = false;
    bool ok = true;
    while (const dirent* e = readdir(dir))
    {
        const std::string name = e->d_name;
        const std::string lower = Lower(name);
        if (name == "." || name == ".." || lower == "test")
        {
            continue; // test: the replay's own
        }
        hasData |= lower == "data";
        if (lower == "player")
        {
            // Save games and high scores: the game writes them, so it gets copies
            ok = CopyFolder(game + "/" + name, scratchGame + "/" + name) && ok;
            continue;
        }
        ok = symlink((game + "/" + name).c_str(), (scratchGame + "/" + name).c_str()) == 0 && ok;
    }
    closedir(dir);
    if (!hasData)
    {
        error = "no game in " + game + " (it should have a data folder; see --game-dir)";
        return false;
    }
    mkdir((scratchGame + "/test").c_str(), 0755);
    if (!ok || (!replay.empty() && !CopyFile(replay, scratchGame + "/test/replay.rep")))
    {
        error = "can't set up the scratch folder " + gScratch;
        return false;
    }
    if (!WriteRegistry(gScratch + "/config/gta2-native/registry.txt", o))
    {
        error = "can't write the scratch registry";
        return false;
    }
    // A made-up pad (platform/controls.cpp) given relative to where gta2 was started
    if (const char* pad = getenv("GTA2_TEST_PAD"); pad && *pad && *pad != '/')
    {
        const std::string path = Absolute(pad);
        if (path.empty())
        {
            error = std::string("no GTA2_TEST_PAD file ") + pad;
            return false;
        }
        setenv("GTA2_TEST_PAD", path.c_str(), 1);
    }
    if (chdir(scratchGame.c_str()) != 0)
    {
        error = "can't enter " + scratchGame;
        return false;
    }
    return true;
}

int RemoveEntry(const char* path, const struct stat*, int, FTW*)
{
    return remove(path);
}

void Cleanup()
{
    if (gLog && gLog != stdout)
    {
        fclose(gLog);
        gLog = nullptr;
    }
    if (!gScratch.empty() && !gOptions.keep)
    {
        nftw(gScratch.c_str(), RemoveEntry, 16, FTW_DEPTH | FTW_PHYS);
        gScratch.clear();
    }
}

// The game ends a fatal error with exit(): report it as status 2
void AtExit()
{
    if (!gActive || gFinished)
    {
        return;
    }
    fprintf(stderr, "gta2 --replay: the game stopped with an error after %ld updates\n", gFrame);
    Cleanup();
    _exit(2);
}

void RequestExit()
{
    gGame_0x40_67E008->ExitGameNoBonus_4B8C00(0, GameExitType::CloseGameImmediately_1);
}
} // namespace

void port_registry_reload(); // win32.cpp

// Called from main() when the first argument is --replay. Sets everything up; the game then starts as
// usual. Returns false (and prints why) when it can't.
bool port_replay_setup(int argc, char** argv)
{
    std::string error;
    if (!Parse(argc, argv, gOptions, error))
    {
        fprintf(stderr, "gta2 --replay: %s\n%s", error.c_str(), kUsage);
        return false;
    }
    // Before MakeScratch moves into the scratch folder, so a relative path means the caller's folder
    const bool recording = !gOptions.record.empty();
    gLog = gOptions.hashLog.empty() ? (recording ? nullptr : stdout) : fopen(gOptions.hashLog.c_str(), "w");
    if (!gLog && !gOptions.hashLog.empty())
    {
        fprintf(stderr, "gta2 --replay: can't write %s\n", gOptions.hashLog.c_str());
        return false;
    }
    if (!gOptions.audioLog.empty())
    {
        FILE* audio = fopen(gOptions.audioLog.c_str(), "w");
        if (!audio)
        {
            fprintf(stderr, "gta2 --replay: can't write %s\n", gOptions.audioLog.c_str());
            return false;
        }
        port_audio_log_open(audio);
    }
    if (!MakeScratch(gOptions, error))
    {
        fprintf(stderr, "gta2 --replay: %s\n%s", error.c_str(), kUsage);
        Cleanup();
        return false;
    }
    setenv("XDG_CONFIG_HOME", (gScratch + "/config").c_str(), 1);
    port_registry_reload();
    if (!recording)
    {
        setenv("GTA2_NO_MESSAGE_BOX", "1", 1); // fatal errors go to stderr only
        setenv("SDL_AUDIODRIVER", "dummy", 0);
        // The mixer runs on the audio thread's clock, so when a sample finishes (which the sound code
        // polls, and which decides what it plays next) would depend on how fast the replay runs. A
        // replay mixes in step with the simulation instead: 33 ms of sound per update, in
        // port_replay_after_update.
        port_audio_manual = true;
    }
    if (gLog)
    {
        fprintf(gLog, "# frame rng_count total rng sprites peds cars objects players script police audio\n");
    }
    gActive = true;
    atexit(AtExit);
    // Ends the game as quitting it does, so a recording is saved
    signal(SIGINT, OnStopSignal);
    signal(SIGTERM, OnStopSignal);
    return true;
}

// Called from main() once the game has returned
int port_replay_finish(int result)
{
    gFinished = true;
    if (gSyncMismatches)
    {
        fprintf(stderr, "gta2 --replay: %ld replay records found a different random number than at recording\n", gSyncMismatches);
    }
    fprintf(stderr, "gta2 --replay: %ld updates\n", gFrame);
    if (!gOptions.record.empty())
    {
        if (CopyFile(gScratch + "/game/test/replay.rep", gOptions.record))
        {
            fprintf(stderr, "gta2 --record: wrote %s\n", gOptions.record.c_str());
        }
        else
        {
            fprintf(stderr, "gta2 --record: no replay was written (%s)\n", gOptions.record.c_str());
            result = result ? result : 1;
        }
    }
    Cleanup();
    return result;
}

void port_replay_before_update()
{
    if (!gActive || !gOptions.record.empty())
    {
        return;
    }
    // Each record applied since the last update was recorded together with the random number of
    // that moment (InputRecorder_67F8B0::save_replay_record_4CDE20). A different one here means this
    // build has already computed something else than the game that recorded the replay.
    const InputRecorder_67F8B0& rec = gInputRecorder_67F8B0;
    for (u32 i = gLastRecord; i < rec.field_75340_rec_buf_idx && i < InputRecorder_67F8B0::k_max_records; i++)
    {
        const InputRecord_C& r = rec.field_3C_rec_buff[i];
        if (r.field_8_rng_rnd != gpRng_67AB34->field_4_rnd || r.field_0_rng_idx != gpRng_67AB34->field_0_rng)
        {
            if (!gSyncMismatches)
            {
                fprintf(stderr,
                        "gta2 --replay: out of sync with the recording from update %ld (record %u: recorded at "
                        "count %d with random number %d, applied at count %d with %d)\n",
                        gFrame,
                        i,
                        r.field_0_rng_idx,
                        r.field_8_rng_rnd,
                        gpRng_67AB34->field_0_rng,
                        gpRng_67AB34->field_4_rnd);
            }
            gSyncMismatches++;
        }
    }
    gLastRecord = rec.field_75340_rec_buf_idx;
}

void port_replay_after_update()
{
    if (!gActive)
    {
        return;
    }
    gFrame++;
    port_audio_frame = gFrame;
    if (port_audio_manual)
    {
        static std::vector<int16_t> samples;
        const int frames = port_audio_rate() * 33 / 1000;
        samples.resize(static_cast<size_t>(frames) * 2);
        port_audio_mix(samples.data(), frames);
    }
    if (gLog && gFrame % gOptions.hashEvery == 0)
    {
        const StateHashes s = port_state_hash();
        fprintf(gLog,
                "%ld %d %016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx\n",
                gFrame,
                gpRng_67AB34->field_0_rng,
                static_cast<unsigned long long>(s.total),
                static_cast<unsigned long long>(s.rng),
                static_cast<unsigned long long>(s.sprites),
                static_cast<unsigned long long>(s.peds),
                static_cast<unsigned long long>(s.cars),
                static_cast<unsigned long long>(s.objects),
                static_cast<unsigned long long>(s.players),
                static_cast<unsigned long long>(s.script),
                static_cast<unsigned long long>(s.police),
                static_cast<unsigned long long>(port_audio_event_hash(nullptr)));
    }
    if (gFrame == gOptions.breakAt)
    {
        const std::string dump = port_state_dump();
        fprintf(gLog ? gLog : stdout, "# state after update %ld\n%s", gFrame, dump.c_str());
        RequestExit();
    }
    if (!gOptions.shots.empty() && gShotsTaken >= gOptions.shots.size())
    {
        RequestExit(); // the last screenshot was written
    }
    if (gFrame == gOptions.frames || gStopRequested)
    {
        RequestExit();
    }
}

bool port_replay_headless()
{
    return gActive && gOptions.record.empty() && !gOptions.draw;
}

const char* port_replay_capture_file()
{
    if (!gActive)
    {
        return nullptr;
    }
    static std::vector<bool> done;
    done.resize(gOptions.shots.size());
    for (size_t i = 0; i < gOptions.shots.size(); i++)
    {
        if (!done[i] && gOptions.shots[i].first == gFrame)
        {
            done[i] = true;
            gShotsTaken++;
            return gOptions.shots[i].second.c_str();
        }
    }
    return nullptr;
}
