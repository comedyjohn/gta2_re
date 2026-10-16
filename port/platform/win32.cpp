// The Win32 API as the game uses it (port/include/port_win32_api.h, mmsystem.h, commctrl.h).
//
// Windows are handles; the one real window is the SDL window the renderer opens, and platform events
// (port_pump_events) arrive as messages to the game's window procedure.
// The registry is real, since the game keeps its settings there: a key/value store saved to
// $XDG_CONFIG_HOME/gta2-native/registry.txt (or ~/.config/...).
#include "events.h"
#include "path.h"
#include "platform.h"
#include "renderer.h"

#include <commctrl.h>
#include <io.h>
#include <mmsystem.h>

#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace
{
uint32_t MonotonicMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint32_t>(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}

// Fake handles: distinct non-null values, never dereferenced
HANDLE MakeHandle()
{
    static uintptr_t next = 0x1000;
    next += 4;
    return reinterpret_cast<HANDLE>(next);
}

const HMODULE kRendererModule = reinterpret_cast<HMODULE>(0xD3D);
} // namespace

extern "C" {

// ---------------------------------------------------------------- kernel32
BOOL CloseHandle(HANDLE)
{
    return TRUE;
}

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR)
{
    return MakeHandle();
}

HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR)
{
    return MakeHandle();
}

HANDLE OpenMutexA(DWORD, BOOL, LPCSTR)
{
    return 0; // no other instance of the game is running
}

BOOL ReleaseMutex(HANDLE)
{
    return TRUE;
}

BOOL SetEvent(HANDLE)
{
    return TRUE;
}

BOOL ResetEvent(HANDLE)
{
    return TRUE;
}

DWORD WaitForSingleObject(HANDLE, DWORD)
{
    return 0;
}

void Sleep(DWORD ms)
{
    usleep(static_cast<useconds_t>(ms) * 1000);
}

DWORD GetTickCount(void)
{
    return MonotonicMs();
}

DWORD timeGetTime(void)
{
    return MonotonicMs();
}

MMRESULT timeBeginPeriod(UINT)
{
    return TIMERR_NOERROR;
}

MMRESULT timeEndPeriod(UINT)
{
    return TIMERR_NOERROR;
}

BOOL QueryPerformanceCounter(LARGE_INTEGER* count)
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    count->QuadPart = static_cast<LONGLONG>(ts.tv_sec) * 1000000000ll + ts.tv_nsec;
    return TRUE;
}

BOOL QueryPerformanceFrequency(LARGE_INTEGER* freq)
{
    freq->QuadPart = 1000000000ll;
    return TRUE;
}

HMODULE LoadLibraryA(LPCSTR name)
{
    if (name && port_is_renderer_library(name))
    {
        return kRendererModule;
    }
    return 0;
}

BOOL FreeLibrary(HMODULE)
{
    return TRUE;
}

FARPROC GetProcAddress(HMODULE module, LPCSTR name)
{
    return module == kRendererModule ? port_renderer_lookup(name) : 0;
}

HMODULE GetModuleHandleA(LPCSTR)
{
    return reinterpret_cast<HMODULE>(1);
}

DWORD GetModuleFileNameA(HMODULE, LPSTR name, DWORD size)
{
    ssize_t len = readlink("/proc/self/exe", name, size ? size - 1 : 0);
    if (len < 0)
    {
        len = 0;
    }
    if (size)
    {
        name[len] = 0;
    }
    return static_cast<DWORD>(len);
}

BOOL GetComputerNameA(LPSTR buffer, LPDWORD size)
{
    char host[256] = "linux";
    gethostname(host, sizeof(host) - 1);
    host[MAX_COMPUTERNAME_LENGTH] = 0;
    size_t len = strlen(host);
    if (len + 1 > *size)
    {
        *size = static_cast<DWORD>(len + 1);
        return FALSE;
    }
    strcpy(buffer, host);
    *size = static_cast<DWORD>(len);
    return TRUE;
}

UINT GetDriveTypeA(LPCSTR)
{
    return DRIVE_FIXED; // the game data is installed, never on a CD
}

DWORD GetFileAttributesA(LPCSTR name)
{
    struct stat st;
    if (stat(port_resolve_path(name).c_str(), &st) != 0)
    {
        return INVALID_FILE_ATTRIBUTES;
    }
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}

BOOL GetVersionExA(LPOSVERSIONINFOA info)
{
    info->dwMajorVersion = 5; // Windows XP: the newest the game's checks know about
    info->dwMinorVersion = 1;
    info->dwBuildNumber = 2600;
    info->dwPlatformId = VER_PLATFORM_WIN32_NT;
    info->szCSDVersion[0] = 0;
    return TRUE;
}

void OutputDebugStringA(LPCSTR s)
{
    fputs(s, stderr);
}

DWORD GetLastError(void)
{
    return 0;
}

int MultiByteToWideChar(UINT, DWORD, LPCSTR src, int srcLen, LPWSTR dst, int dstLen)
{
    int len = srcLen < 0 ? static_cast<int>(strlen(src)) + 1 : srcLen;
    if (!dstLen)
    {
        return len;
    }
    int n = len < dstLen ? len : dstLen;
    for (int i = 0; i < n; i++)
    {
        dst[i] = static_cast<unsigned char>(src[i]);
    }
    return n;
}

int WideCharToMultiByte(UINT, DWORD, LPCWSTR src, int srcLen, LPSTR dst, int dstLen, LPCSTR, LPBOOL usedDefault)
{
    int len = srcLen < 0 ? static_cast<int>(port_wcslen(src)) + 1 : srcLen;
    if (usedDefault)
    {
        *usedDefault = FALSE;
    }
    if (!dstLen)
    {
        return len;
    }
    int n = len < dstLen ? len : dstLen;
    for (int i = 0; i < n; i++)
    {
        unsigned short c = static_cast<unsigned short>(src[i]);
        dst[i] = c < 256 ? static_cast<char>(c) : '?';
        if (c >= 256 && usedDefault)
        {
            *usedDefault = TRUE;
        }
    }
    return n;
}

// FindFirstFileA & co. sit on the _findfirst family in crt.cpp
static void ToFindData(const _finddata_t& in, LPWIN32_FIND_DATAA out)
{
    memset(out, 0, sizeof(*out));
    out->dwFileAttributes = (in.attrib & _A_SUBDIR) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    out->nFileSizeLow = in.size;
    snprintf(out->cFileName, sizeof(out->cFileName), "%s", in.name);
}

HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA data)
{
    _finddata_t fd;
    long h = port_findfirst(pattern, &fd);
    if (h == -1)
    {
        return INVALID_HANDLE_VALUE;
    }
    ToFindData(fd, data);
    return reinterpret_cast<HANDLE>(h);
}

BOOL FindNextFileA(HANDLE find, LPWIN32_FIND_DATAA data)
{
    _finddata_t fd;
    if (port_findnext(reinterpret_cast<long>(find), &fd) != 0)
    {
        return FALSE;
    }
    ToFindData(fd, data);
    return TRUE;
}

BOOL FindClose(HANDLE find)
{
    return port_findclose(reinterpret_cast<long>(find)) == 0;
}

DWORD GetPrivateProfileStringA(LPCSTR app, LPCSTR key, LPCSTR def, LPSTR ret, DWORD size, LPCSTR file)
{
    // A minimal INI reader: [section] and key=value, both matched ignoring case
    std::string result = def ? def : "";
    if (FILE* f = fopen(port_resolve_path(file).c_str(), "r"))
    {
        char line[512];
        bool inSection = false;
        while (fgets(line, sizeof(line), f))
        {
            char* p = line;
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            p[strcspn(p, "\r\n")] = 0;
            if (*p == '[')
            {
                char* end = strchr(p, ']');
                if (end)
                {
                    *end = 0;
                }
                inSection = app && strcasecmp(p + 1, app) == 0;
            }
            else if (inSection && key)
            {
                char* eq = strchr(p, '=');
                if (eq)
                {
                    *eq = 0;
                    char* k = p;
                    char* kEnd = eq;
                    while (kEnd > k && (kEnd[-1] == ' ' || kEnd[-1] == '\t'))
                    {
                        *--kEnd = 0;
                    }
                    if (strcasecmp(k, key) == 0)
                    {
                        result = eq + 1;
                        break;
                    }
                }
            }
        }
        fclose(f);
    }
    if (!size)
    {
        return 0;
    }
    snprintf(ret, size, "%s", result.c_str());
    return static_cast<DWORD>(strlen(ret));
}

LPSTR lstrcatA(LPSTR a, LPCSTR b)
{
    return strcat(a, b);
}

LPSTR lstrcpyA(LPSTR a, LPCSTR b)
{
    return strcpy(a, b);
}

int lstrlenA(LPCSTR s)
{
    return s ? static_cast<int>(strlen(s)) : 0;
}

int lstrcmpiA(LPCSTR a, LPCSTR b)
{
    return strcasecmp(a, b);
}

int wvsprintfA(LPSTR buf, LPCSTR fmt, va_list args)
{
    return vsprintf(buf, fmt, args);
}

int wsprintfA(LPSTR buf, LPCSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsprintf(buf, fmt, args);
    va_end(args);
    return n;
}

// ---------------------------------------------------------------- user32: windows and messages
namespace
{
std::map<std::string, WNDPROC> gWindowClasses; // lower-case class name -> window procedure
std::map<HWND, WNDPROC> gWindowProcs;
std::map<HWND, LONG_PTR> gWindowLongs[2];
std::deque<MSG> gMessages;
HWND gMainWindow = 0; // the first top-level window: where platform events go
int gCursorCount = 0;
bool gPumping = false;
bool gFocused = false; // the window has focus, as last passed on to the game
std::vector<WNDPROC>* gWndProcHandles = 0; // 64-bit: see WndProcToLong

void Post(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MSG m;
    memset(&m, 0, sizeof(m));
    m.hwnd = hwnd;
    m.message = msg;
    m.wParam = wParam;
    m.lParam = lParam;
    m.time = MonotonicMs();
    gMessages.push_back(m);
}
} // namespace

} // extern "C"

void port_pump_events()
{
    if (gPumping) // DirectInput can be read from inside a window procedure
    {
        return;
    }
    gPumping = true;
    plat::Event e;
    while (plat::PollEvent(e))
    {
        switch (e.type)
        {
            case plat::EventType::Quit:
                Post(gMainWindow, WM_CLOSE, 0, 0);
                break;
            // Only on a change of focus, as on Windows (ShowWindow may have activated the window
            // already): WM_SETFOCUS makes the game set its video mode again
            case plat::EventType::FocusGained:
                if (!gFocused)
                {
                    gFocused = true;
                    Post(gMainWindow, WM_ACTIVATE, WA_ACTIVE, 0);
                    Post(gMainWindow, WM_SETFOCUS, 0, 0);
                }
                break;
            case plat::EventType::FocusLost:
                if (gFocused)
                {
                    gFocused = false;
                    Post(gMainWindow, WM_ACTIVATE, WA_INACTIVE, 0);
                    Post(gMainWindow, WM_KILLFOCUS, 0, 0);
                }
                break;
            case plat::EventType::Minimized:
                Post(gMainWindow, WM_SIZE, 1, 0); // SIZE_MINIMIZED
                break;
            case plat::EventType::Restored:
                Post(gMainWindow, WM_SIZE, 0, MAKELONG(640, 480)); // SIZE_RESTORED
                break;
            case plat::EventType::KeyDown:
            case plat::EventType::KeyUp:
            {
                const bool down = e.type == plat::EventType::KeyDown;
                // Alt+Enter switches fullscreen here rather than through the game's DirectDraw mode reset
                if (down && e.alt && e.vk == VK_RETURN)
                {
                    plat::ToggleFullscreen();
                    break;
                }
                if (e.dik)
                {
                    port_dinput_key_event(e.dik, down);
                }
                if (e.vk)
                {
                    const bool sys = e.alt || e.vk == VK_F10 || e.vk == VK_LMENU || e.vk == VK_RMENU;
                    const UINT msg = down ? (sys ? WM_SYSKEYDOWN : WM_KEYDOWN) : (sys ? WM_SYSKEYUP : WM_KEYUP);
                    Post(gMainWindow, msg, e.vk, e.alt ? (KF_ALTDOWN << 16) : 0);
                }
                break;
            }
            default:
                break;
        }
    }
    gPumping = false;
}

extern "C" {

int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT)
{
    plat::ShowMessage(caption ? caption : "GTA2", text ? text : "");
    return IDOK;
}

LRESULT DefWindowProcA(HWND hwnd, UINT msg, WPARAM, LPARAM)
{
    if (msg == WM_CLOSE)
    {
        DestroyWindow(hwnd);
    }
    return 0;
}

ATOM RegisterClassA(const WNDCLASSA* wc)
{
    std::string name = wc->lpszClassName ? wc->lpszClassName : "";
    for (char& c : name)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    gWindowClasses[name] = wc->lpfnWndProc;
    return static_cast<ATOM>(gWindowClasses.size());
}

HWND CreateWindowExA(DWORD, LPCSTR className, LPCSTR, DWORD, int, int, int, int, HWND parent, HMENU, HINSTANCE, LPVOID)
{
    HWND hwnd = reinterpret_cast<HWND>(MakeHandle());
    std::string name = className ? className : "";
    for (char& c : name)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    auto it = gWindowClasses.find(name);
    gWindowProcs[hwnd] = it != gWindowClasses.end() ? it->second : 0;
    if (!parent && !gMainWindow)
    {
        gMainWindow = hwnd;
    }
    // The real window appears when the game sets a video mode (renderer.cpp's Vid_SetMode)
    return hwnd;
}

BOOL DestroyWindow(HWND hwnd)
{
    auto it = gWindowProcs.find(hwnd);
    if (it == gWindowProcs.end())
    {
        return FALSE;
    }
    if (it->second)
    {
        it->second(hwnd, WM_DESTROY, 0, 0);
    }
    gWindowProcs.erase(hwnd);
    return TRUE;
}

BOOL ShowWindow(HWND hwnd, int cmd)
{
    // Windows activates a window it shows, unless it is active already or the command says not to.
    // The game shows its window whenever it sets the video mode, which it does on WM_SETFOCUS, so
    // activating it every time would loop forever.
    const bool activates = cmd != SW_HIDE && cmd != SW_MINIMIZE && cmd != SW_SHOWMINNOACTIVE &&
                           cmd != SW_SHOWNA && cmd != SW_SHOWNOACTIVATE;
    if (hwnd == gMainWindow && activates && !gFocused)
    {
        gFocused = true;
        Post(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
        Post(hwnd, WM_SETFOCUS, 0, 0);
    }
    return TRUE;
}

BOOL UpdateWindow(HWND)
{
    return TRUE;
}

BOOL SetForegroundWindow(HWND)
{
    return TRUE;
}

// GWL_WNDPROC passes a window procedure through a LONG. That fits in a 32-bit build; in a 64-bit one
// a LONG can't hold a pointer, so procedures travel as small numbered handles instead (64-bit
// Windows does the same), which CallWindowProcA and SetWindowLongA turn back into the procedure.
static LONG WndProcToLong(WNDPROC proc)
{
#if __SIZEOF_POINTER__ == 4
    return reinterpret_cast<LONG>(proc);
#else
    static std::vector<WNDPROC> handles;
    if (!proc)
    {
        return 0;
    }
    for (size_t i = 0; i < handles.size(); i++)
    {
        if (handles[i] == proc)
        {
            return static_cast<LONG>(0xFFFF0000u + i);
        }
    }
    handles.push_back(proc);
    gWndProcHandles = &handles;
    return static_cast<LONG>(0xFFFF0000u + handles.size() - 1);
#endif
}

static WNDPROC LongToWndProc(intptr_t value)
{
#if __SIZEOF_POINTER__ == 4
    return reinterpret_cast<WNDPROC>(value);
#else
    const uint32_t v = static_cast<uint32_t>(value);
    if (gWndProcHandles && value == static_cast<LONG>(v) && v >= 0xFFFF0000u && v - 0xFFFF0000u < gWndProcHandles->size())
    {
        return (*gWndProcHandles)[v - 0xFFFF0000u];
    }
    return reinterpret_cast<WNDPROC>(value);
#endif
}

LONG SetWindowLongA(HWND hwnd, int index, LONG value)
{
    if (index == GWL_WNDPROC)
    {
        LONG old = WndProcToLong(gWindowProcs[hwnd]);
        gWindowProcs[hwnd] = LongToWndProc(value);
        return old;
    }
    std::map<HWND, LONG_PTR>& longs = gWindowLongs[index == GWL_EXSTYLE ? 1 : 0];
    LONG old = static_cast<LONG>(longs[hwnd]);
    longs[hwnd] = value;
    return old;
}

LONG GetWindowLongA(HWND hwnd, int index)
{
    if (index == GWL_WNDPROC)
    {
        return WndProcToLong(gWindowProcs[hwnd]);
    }
    return static_cast<LONG>(gWindowLongs[index == GWL_EXSTYLE ? 1 : 0][hwnd]);
}

// The pointer-sized versions 64-bit Windows added: a procedure or a pointer travels whole.
LONG_PTR SetWindowLongPtrA(HWND hwnd, int index, LONG_PTR value)
{
    if (index == GWL_WNDPROC)
    {
        WNDPROC old = gWindowProcs[hwnd];
        gWindowProcs[hwnd] = reinterpret_cast<WNDPROC>(value);
        return reinterpret_cast<LONG_PTR>(old);
    }
    std::map<HWND, LONG_PTR>& longs = gWindowLongs[index == GWL_EXSTYLE ? 1 : 0];
    LONG_PTR old = longs[hwnd];
    longs[hwnd] = value;
    return old;
}

LONG_PTR GetWindowLongPtrA(HWND hwnd, int index)
{
    if (index == GWL_WNDPROC)
    {
        return reinterpret_cast<LONG_PTR>(gWindowProcs[hwnd]);
    }
    return gWindowLongs[index == GWL_EXSTYLE ? 1 : 0][hwnd];
}

BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT)
{
    return TRUE;
}

BOOL GetClientRect(HWND, LPRECT rect)
{
    // The game draws at 640x480; the renderer scales that to the real window
    rect->left = 0;
    rect->top = 0;
    rect->right = 640;
    rect->bottom = 480;
    return TRUE;
}

BOOL GetWindowRect(HWND hwnd, LPRECT rect)
{
    return GetClientRect(hwnd, rect);
}

BOOL AdjustWindowRect(LPRECT, DWORD, BOOL)
{
    return TRUE;
}

HWND FindWindowA(LPCSTR, LPCSTR)
{
    return 0;
}

HWND FindWindowExA(HWND, HWND, LPCSTR, LPCSTR)
{
    return 0;
}

BOOL PeekMessageA(LPMSG msg, HWND, UINT, UINT, UINT remove)
{
    port_pump_events();
    if (gMessages.empty())
    {
        return FALSE;
    }
    *msg = gMessages.front();
    if (remove & PM_REMOVE)
    {
        gMessages.pop_front();
    }
    return TRUE;
}

BOOL GetMessageA(LPMSG msg, HWND hwnd, UINT min, UINT max)
{
    while (!PeekMessageA(msg, hwnd, min, max, PM_REMOVE))
    {
        usleep(5000);
    }
    return msg->message != WM_QUIT;
}

BOOL TranslateMessage(const MSG*)
{
    return FALSE;
}

LRESULT DispatchMessageA(const MSG* msg)
{
    auto it = gWindowProcs.find(msg->hwnd);
    if (it == gWindowProcs.end() || !it->second)
    {
        return 0;
    }
    return it->second(msg->hwnd, msg->message, msg->wParam, msg->lParam);
}

BOOL PostMessageA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Post(hwnd, msg, wParam, lParam);
    return TRUE;
}

LRESULT SendMessageA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto it = gWindowProcs.find(hwnd);
    if (it == gWindowProcs.end() || !it->second)
    {
        return 0; // a lobby dialog control: none exist
    }
    return it->second(hwnd, msg, wParam, lParam);
}

void PostQuitMessage(int code)
{
    Post(0, WM_QUIT, static_cast<WPARAM>(code), 0);
}

int ShowCursor(BOOL show)
{
    gCursorCount += show ? 1 : -1;
    plat::SetCursorVisible(gCursorCount >= 0);
    return gCursorCount;
}

HICON LoadIconA(HINSTANCE, LPCSTR)
{
    return reinterpret_cast<HICON>(MakeHandle());
}

HCURSOR LoadCursorA(HINSTANCE, LPCSTR)
{
    return reinterpret_cast<HCURSOR>(MakeHandle());
}

HCURSOR SetCursor(HCURSOR cursor)
{
    return cursor;
}

int GetSystemMetrics(int index)
{
    switch (index)
    {
        case SM_CXSCREEN:
            return 640;
        case SM_CYSCREEN:
            return 480;
        default:
            return 0;
    }
}

HDC GetDC(HWND)
{
    return reinterpret_cast<HDC>(MakeHandle());
}

int ReleaseDC(HWND, HDC)
{
    return 1;
}

BOOL GetKeyboardLayoutNameA(LPSTR name)
{
    strcpy(name, "00000409"); // US English
    return TRUE;
}

short GetAsyncKeyState(int)
{
    return 0;
}

short GetKeyState(int)
{
    return 0;
}

BOOL SetWindowTextA(HWND hwnd, LPCSTR text)
{
    if (hwnd == gMainWindow && text)
    {
        plat::SetWindowTitle(text);
    }
    return TRUE;
}

HWND GetActiveWindow(void)
{
    return gMainWindow;
}

HWND GetForegroundWindow(void)
{
    return gMainWindow;
}

BOOL IsIconic(HWND)
{
    return FALSE;
}

BOOL InvalidateRect(HWND, const RECT*, BOOL)
{
    return TRUE;
}

BOOL ValidateRect(HWND, const RECT*)
{
    return TRUE;
}

// The one dialog is the network lobby (resource 103), which the port runs without a window
// (lobby.cpp). Its controls' text is kept, so the game's own code reading the player name back
// (Network_20324::cb_SavePlayerName_51BC00) gets what the lobby put there.
INT_PTR PortRunDialog(HINSTANCE instance, LPCSTR templateName, DLGPROC proc, LPARAM param);
static std::map<int, std::string> gDialogText;

INT_PTR DialogBoxParamA(HINSTANCE instance, LPCSTR templateName, HWND, DLGPROC proc, LPARAM param)
{
    gDialogText.clear();
    return PortRunDialog(instance, templateName, proc, param);
}

BOOL EndDialog(HWND, INT_PTR)
{
    return TRUE;
}

HWND GetDlgItem(HWND, int)
{
    return 0;
}

int GetDlgCtrlID(HWND)
{
    return 0;
}

UINT GetDlgItemTextA(HWND, int id, LPSTR text, int max)
{
    if (max <= 0)
    {
        return 0;
    }
    const std::string& s = gDialogText[id];
    const size_t n = std::min(s.size(), static_cast<size_t>(max - 1));
    memcpy(text, s.data(), n);
    text[n] = 0;
    return static_cast<UINT>(n);
}

BOOL SetDlgItemTextA(HWND, int id, LPCSTR text)
{
    gDialogText[id] = text ? text : "";
    return TRUE;
}

LRESULT SendDlgItemMessageA(HWND, int, UINT, WPARAM, LPARAM)
{
    return 0;
}

BOOL EnableWindow(HWND, BOOL)
{
    return FALSE;
}

HWND GetParent(HWND)
{
    return 0;
}

int GetWindowTextA(HWND, LPSTR text, int max)
{
    if (max > 0)
    {
        text[0] = 0;
    }
    return 0;
}

LRESULT CallWindowProcA(WNDPROC proc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    proc = LongToWndProc(reinterpret_cast<intptr_t>(proc)); // may be a handle from GetWindowLongA
    return proc ? proc(hwnd, msg, wParam, lParam) : 0;
}

UINT_PTR SetTimer(HWND, UINT_PTR id, UINT, TIMERPROC)
{
    return id ? id : 1;
}

BOOL KillTimer(HWND, UINT_PTR)
{
    return TRUE;
}

void InitCommonControls(void)
{
}

// ---------------------------------------------------------------- gdi32
int GetDeviceCaps(HDC, int index)
{
    switch (index)
    {
        case BITSPIXEL:
            return 32;
        case PLANES:
            return 1;
        case HORZRES:
            return 640;
        case VERTRES:
            return 480;
        default:
            return 0;
    }
}

HGDIOBJ GetStockObject(int)
{
    return MakeHandle();
}

// ---------------------------------------------------------------- advapi32: the registry
} // extern "C"

// The registry's own helpers are C++ (they return std::string), outside the C API's extern "C"
namespace
{
struct RegValue
{
    DWORD mType;
    std::vector<BYTE> mData;
};

// Key paths are stored lower-case ("hkey_local_machine\software\dma design ltd\gta2\screen")
std::map<std::string, std::map<std::string, RegValue>> gRegistry;
std::map<HKEY, std::string> gOpenKeys;
bool gRegistryLoaded = false;

std::string Lower(std::string s)
{
    for (char& c : s)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string RegistryFile()
{
    const char* base = getenv("XDG_CONFIG_HOME");
    std::string dir = base && *base ? base : std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.config";
    dir += "/gta2-native";
    mkdir(dir.c_str(), 0755);
    return dir + "/registry.txt";
}

// One value per line: key \t value-name \t type \t hex bytes
void LoadRegistry()
{
    if (gRegistryLoaded)
    {
        return;
    }
    gRegistryLoaded = true;
    // What an install of the game sets up. This build is the freeloader.com edition, which
    // downloaded the cities one at a time: before starting a level it checks that the city is
    // installed (FreeLoader::CheckCityInstalled_4AE1F0, cities 1 to 3), and when it isn't it
    // minimizes the game and runs WebLaunch.exe to fetch it. All three are here.
    RegValue cities;
    cities.mType = REG_DWORD;
    cities.mData = {3, 0, 0, 0};
    gRegistry["hkey_local_machine\\software\\freeloader.com\\gta2"]["cityinstalled"] = cities;

    FILE* f = fopen(RegistryFile().c_str(), "r");
    if (!f)
    {
        return;
    }
    char line[4096];
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;
        char* fields[4];
        char* p = line;
        int n = 0;
        for (; n < 4 && p; n++)
        {
            fields[n] = p;
            p = strchr(p, '\t');
            if (p)
            {
                *p++ = 0;
            }
        }
        if (n < 4)
        {
            continue;
        }
        RegValue v;
        v.mType = static_cast<DWORD>(strtoul(fields[2], 0, 10));
        for (const char* h = fields[3]; h[0] && h[1]; h += 2)
        {
            char byte[3] = {h[0], h[1], 0};
            v.mData.push_back(static_cast<BYTE>(strtoul(byte, 0, 16)));
        }
        gRegistry[fields[0]][fields[1]] = v;
    }
    fclose(f);
}

void SaveRegistry()
{
    FILE* f = fopen(RegistryFile().c_str(), "w");
    if (!f)
    {
        return;
    }
    for (const auto& key : gRegistry)
    {
        for (const auto& value : key.second)
        {
            fprintf(f, "%s\t%s\t%u\t", key.first.c_str(), value.first.c_str(), static_cast<unsigned>(value.second.mType));
            for (BYTE b : value.second.mData)
            {
                fprintf(f, "%02x", b);
            }
            fputc('\n', f);
        }
    }
    fclose(f);
}

bool KeyPath(HKEY key, LPCSTR subKey, std::string& out)
{
    LoadRegistry();
    std::string base;
    if (key == HKEY_LOCAL_MACHINE)
    {
        base = "hkey_local_machine";
    }
    else if (key == HKEY_CURRENT_USER)
    {
        base = "hkey_current_user";
    }
    else if (key == HKEY_CLASSES_ROOT)
    {
        base = "hkey_classes_root";
    }
    else if (key == HKEY_USERS)
    {
        base = "hkey_users";
    }
    else
    {
        auto it = gOpenKeys.find(key);
        if (it == gOpenKeys.end())
        {
            return false;
        }
        base = it->second;
    }
    out = base;
    if (subKey && *subKey)
    {
        out += '\\';
        out += Lower(subKey);
    }
    return true;
}

HKEY OpenHandle(const std::string& path)
{
    HKEY h = reinterpret_cast<HKEY>(MakeHandle());
    gOpenKeys[h] = path;
    return h;
}
} // namespace

extern "C" {

LSTATUS RegOpenKeyExA(HKEY key, LPCSTR subKey, DWORD, REGSAM, PHKEY result)
{
    std::string path;
    if (!KeyPath(key, subKey, path) || !gRegistry.count(path))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    *result = OpenHandle(path);
    return ERROR_SUCCESS;
}

LSTATUS RegOpenKeyA(HKEY key, LPCSTR subKey, PHKEY result)
{
    return RegOpenKeyExA(key, subKey, 0, KEY_ALL_ACCESS, result);
}

LSTATUS RegCreateKeyExA(HKEY key, LPCSTR subKey, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY result,
                        LPDWORD disposition)
{
    std::string path;
    if (!KeyPath(key, subKey, path))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    bool existed = gRegistry.count(path) != 0;
    gRegistry[path];
    if (disposition)
    {
        *disposition = existed ? REG_OPENED_EXISTING_KEY : REG_CREATED_NEW_KEY;
    }
    *result = OpenHandle(path);
    return ERROR_SUCCESS;
}

LSTATUS RegCloseKey(HKEY key)
{
    gOpenKeys.erase(key);
    return ERROR_SUCCESS;
}

LSTATUS RegQueryValueExA(HKEY key, LPCSTR name, LPDWORD, LPDWORD type, LPBYTE data, LPDWORD size)
{
    std::string path;
    if (!KeyPath(key, 0, path))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    auto keyIt = gRegistry.find(path);
    if (keyIt == gRegistry.end())
    {
        return ERROR_FILE_NOT_FOUND;
    }
    auto valueIt = keyIt->second.find(Lower(name ? name : ""));
    if (valueIt == keyIt->second.end())
    {
        return ERROR_FILE_NOT_FOUND;
    }
    const RegValue& v = valueIt->second;
    if (type)
    {
        *type = v.mType;
    }
    if (size)
    {
        DWORD available = *size;
        *size = static_cast<DWORD>(v.mData.size());
        if (data)
        {
            if (available < v.mData.size())
            {
                return ERROR_MORE_DATA;
            }
            memcpy(data, v.mData.data(), v.mData.size());
        }
    }
    return ERROR_SUCCESS;
}

LSTATUS RegSetValueExA(HKEY key, LPCSTR name, DWORD, DWORD type, const BYTE* data, DWORD size)
{
    std::string path;
    if (!KeyPath(key, 0, path))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    RegValue v;
    v.mType = type;
    v.mData.assign(data, data + size);
    gRegistry[path][Lower(name ? name : "")] = v;
    SaveRegistry();
    return ERROR_SUCCESS;
}

LSTATUS RegDeleteValueA(HKEY key, LPCSTR name)
{
    std::string path;
    if (!KeyPath(key, 0, path) || !gRegistry[path].erase(Lower(name ? name : "")))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    SaveRegistry();
    return ERROR_SUCCESS;
}

// ---------------------------------------------------------------- version, shell32, shlwapi
DWORD GetFileVersionInfoSizeA(LPCSTR, LPDWORD handle)
{
    if (handle)
    {
        *handle = 0;
    }
    return 0; // no version resources on Linux
}

BOOL GetFileVersionInfoA(LPCSTR, DWORD, DWORD, LPVOID)
{
    return FALSE;
}

BOOL VerQueryValueA(const void*, LPCSTR, LPVOID* buffer, PUINT len)
{
    *buffer = 0;
    *len = 0;
    return FALSE;
}

HINSTANCE ShellExecuteA(HWND, LPCSTR, LPCSTR file, LPCSTR, LPCSTR, int)
{
    fprintf(stderr, "ShellExecute(%s) is not supported\n", file ? file : "");
    return reinterpret_cast<HINSTANCE>(2); // <= 32 means failure
}

LPSTR StrStrIA(LPCSTR haystack, LPCSTR needle)
{
    return const_cast<LPSTR>(strcasestr(haystack, needle));
}

} // extern "C"

// gta2 --replay moves the registry to a scratch file (XDG_CONFIG_HOME) after the game's static
// initialisation may already have read the real one: forget it and read the scratch file
void port_registry_reload()
{
    gRegistry.clear();
    gOpenKeys.clear();
    gRegistryLoaded = false;
    LoadRegistry();
}
