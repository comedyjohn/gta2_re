// MSVC C runtime functions glibc lacks or spells differently (port/include/port_crt.h, direct.h, io.h),
// and the path resolution behind fopen and friends.
#include "path.h"

#include <direct.h>
#include <io.h>

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#undef fopen
#undef freopen

std::string port_resolve_path(const char* gamePath)
{
    std::string path(gamePath ? gamePath : "");
    for (char& c : path)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    // "C:/..." -> "...": the game only names a drive for the CD's music (cSampleManager looks for
    // X:\GTAUDIO\ on every drive), which is a folder in the game's own folder here
    if (path.size() >= 2 && path[1] == ':')
    {
        path.erase(0, path.size() >= 3 && path[2] == '/' ? 3 : 2);
    }
    if (path.empty() || access(path.c_str(), F_OK) == 0)
    {
        return path;
    }

    // Walk the components, replacing each with the directory entry that matches it ignoring case
    std::string resolved = path[0] == '/' ? "/" : "";
    size_t pos = path[0] == '/' ? 1 : 0;
    while (pos <= path.size())
    {
        size_t next = path.find('/', pos);
        if (next == std::string::npos)
        {
            next = path.size();
        }
        std::string part = path.substr(pos, next - pos);
        if (!part.empty())
        {
            std::string candidate = resolved + part;
            if (access(candidate.c_str(), F_OK) != 0)
            {
                DIR* dir = opendir(resolved.empty() ? "." : resolved.c_str());
                if (dir)
                {
                    while (dirent* entry = readdir(dir))
                    {
                        if (strcasecmp(entry->d_name, part.c_str()) == 0)
                        {
                            candidate = resolved + entry->d_name;
                            break;
                        }
                    }
                    closedir(dir);
                }
            }
            resolved = candidate;
            if (next < path.size())
            {
                resolved += '/';
            }
        }
        pos = next + 1;
    }
    return resolved;
}

extern "C" {

FILE* port_fopen(const char* path, const char* mode)
{
    return fopen(port_resolve_path(path).c_str(), mode);
}

FILE* port_freopen(const char* path, const char* mode, FILE* stream)
{
    return freopen(path ? port_resolve_path(path).c_str() : 0, mode, stream);
}

int port_stat(const char* path, struct _stat* buf)
{
    return stat(port_resolve_path(path).c_str(), buf);
}

int port_access(const char* path, int mode)
{
    return access(port_resolve_path(path).c_str(), mode);
}

int port_chdir(const char* path)
{
    return chdir(port_resolve_path(path).c_str());
}

char* port_getcwd(char* buf, int size)
{
    return getcwd(buf, size);
}

int port_mkdir(const char* path)
{
    return mkdir(port_resolve_path(path).c_str(), 0755);
}

// ---- _findfirst / _findnext / _findclose: a directory listing filtered by a wildcard
namespace
{
struct FindState
{
    std::vector<std::string> mNames;
    std::string mDir;
    size_t mNext;
};
std::map<long, FindState> gFinds;
long gNextFindHandle = 1;

bool FillFindData(FindState& state, _finddata_t* data)
{
    if (state.mNext >= state.mNames.size())
    {
        return false;
    }
    const std::string& name = state.mNames[state.mNext++];
    memset(data, 0, sizeof(*data));
    snprintf(data->name, sizeof(data->name), "%s", name.c_str());
    struct stat st;
    if (stat((state.mDir + name).c_str(), &st) == 0)
    {
        data->size = static_cast<unsigned long>(st.st_size);
        data->time_write = static_cast<long>(st.st_mtime);
        data->time_create = data->time_write;
        data->time_access = static_cast<long>(st.st_atime);
        data->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_ARCH;
    }
    return true;
}
} // namespace

long port_findfirst(const char* pattern, _finddata_t* data)
{
    std::string full(pattern);
    for (char& c : full)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    size_t slash = full.rfind('/');
    std::string dirPart = slash == std::string::npos ? "" : full.substr(0, slash + 1);
    std::string mask = slash == std::string::npos ? full : full.substr(slash + 1);

    // Bink movies can't be played yet (Source/3rdParty/bink.cpp is stubs) and the game treats a
    // movie it fails to open as fatal. It only plays one after finding it with _findfirst
    // (Frontend::intro_bik_exists_4B5FF0 and pre_intro_bik_exists_4B6030), so they are reported absent.
    if (mask.size() > 4 && strcasecmp(mask.c_str() + mask.size() - 4, ".bik") == 0)
    {
        return -1;
    }

    FindState state;
    state.mDir = dirPart.empty() ? "" : port_resolve_path(dirPart.c_str());
    if (!state.mDir.empty() && state.mDir.back() != '/')
    {
        state.mDir += '/';
    }
    state.mNext = 0;
    DIR* dir = opendir(state.mDir.empty() ? "." : state.mDir.c_str());
    if (!dir)
    {
        return -1;
    }
    while (dirent* entry = readdir(dir))
    {
        if (fnmatch(mask.c_str(), entry->d_name, FNM_CASEFOLD) == 0)
        {
            state.mNames.push_back(entry->d_name);
        }
    }
    closedir(dir);
    if (!FillFindData(state, data))
    {
        return -1;
    }
    long handle = gNextFindHandle++;
    gFinds[handle] = state;
    return handle;
}

int port_findnext(long handle, _finddata_t* data)
{
    auto it = gFinds.find(handle);
    if (it == gFinds.end() || !FillFindData(it->second, data))
    {
        return -1;
    }
    return 0;
}

int port_findclose(long handle)
{
    return gFinds.erase(handle) ? 0 : -1;
}

// ---- string helpers
char* port_strupr(char* s)
{
    for (char* p = s; *p; p++)
    {
        *p = static_cast<char>(toupper(static_cast<unsigned char>(*p)));
    }
    return s;
}

char* port_strlwr(char* s)
{
    for (char* p = s; *p; p++)
    {
        *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
    }
    return s;
}

char* port_itoa(int value, char* buf, int radix)
{
    if (radix == 10)
    {
        sprintf(buf, "%d", value);
        return buf;
    }
    unsigned int v = static_cast<unsigned int>(value);
    char tmp[40];
    char* p = tmp + sizeof(tmp) - 1;
    *p = 0;
    do
    {
        unsigned int digit = v % radix;
        *--p = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
        v /= radix;
    } while (v);
    strcpy(buf, p);
    return buf;
}

// MSVC's rand(): a linear congruential generator, 15 bits a draw, seeded with 1
static unsigned int gRandState = 1;

int port_rand(void)
{
    gRandState = gRandState * 214013u + 2531011u;
    return static_cast<int>((gRandState >> 16) & PORT_RAND_MAX);
}

void port_srand(unsigned int seed)
{
    gRandState = seed;
}

} // extern "C"
