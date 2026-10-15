// Win32 functions, structures and constants the game uses (kernel32, user32, gdi32, advapi32,
// version, shell32, shlwapi). Declared here, implemented in port/platform/win32_stubs.cpp and friends.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ---- kernel32
#define INFINITE 0xFFFFFFFF
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define MUTEX_ALL_ACCESS 0x1F0001
#define DRIVE_UNKNOWN 0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED 3
#define DRIVE_REMOTE 4
#define DRIVE_CDROM 5
#define DRIVE_RAMDISK 6
#define CP_ACP 0
#define FILE_ATTRIBUTE_READONLY 0x01
#define FILE_ATTRIBUTE_HIDDEN 0x02
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL 0x80
#define VER_PLATFORM_WIN32s 0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2
#define MAX_COMPUTERNAME_LENGTH 15

typedef struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;

typedef struct _WIN32_FIND_DATAA
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, *LPWIN32_FIND_DATAA, WIN32_FIND_DATA, *LPWIN32_FIND_DATA;

typedef struct _OSVERSIONINFOA
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[128];
} OSVERSIONINFOA, *LPOSVERSIONINFOA, OSVERSIONINFO, *LPOSVERSIONINFO;

typedef struct _SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

BOOL CloseHandle(HANDLE h);
HANDLE CreateEventA(LPSECURITY_ATTRIBUTES sa, BOOL manualReset, BOOL initialState, LPCSTR name);
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL initialOwner, LPCSTR name);
HANDLE OpenMutexA(DWORD access, BOOL inherit, LPCSTR name);
BOOL ReleaseMutex(HANDLE h);
BOOL SetEvent(HANDLE h);
BOOL ResetEvent(HANDLE h);
DWORD WaitForSingleObject(HANDLE h, DWORD ms);
void Sleep(DWORD ms);
DWORD GetTickCount(void);
BOOL QueryPerformanceCounter(LARGE_INTEGER* count);
BOOL QueryPerformanceFrequency(LARGE_INTEGER* freq);
HMODULE LoadLibraryA(LPCSTR name);
BOOL FreeLibrary(HMODULE module);
FARPROC GetProcAddress(HMODULE module, LPCSTR name);
HMODULE GetModuleHandleA(LPCSTR name);
DWORD GetModuleFileNameA(HMODULE module, LPSTR name, DWORD size);
BOOL GetComputerNameA(LPSTR buffer, LPDWORD size);
UINT GetDriveTypeA(LPCSTR root);
DWORD GetFileAttributesA(LPCSTR name);
BOOL GetVersionExA(LPOSVERSIONINFOA info);
void OutputDebugStringA(LPCSTR s);
DWORD GetLastError(void);
int MultiByteToWideChar(UINT codePage, DWORD flags, LPCSTR src, int srcLen, LPWSTR dst, int dstLen);
int WideCharToMultiByte(UINT codePage, DWORD flags, LPCWSTR src, int srcLen, LPSTR dst, int dstLen, LPCSTR defChar, LPBOOL usedDef);
HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA data);
BOOL FindNextFileA(HANDLE find, LPWIN32_FIND_DATAA data);
BOOL FindClose(HANDLE find);
DWORD GetPrivateProfileStringA(LPCSTR app, LPCSTR key, LPCSTR def, LPSTR ret, DWORD size, LPCSTR file);
LPSTR lstrcatA(LPSTR a, LPCSTR b);
LPSTR lstrcpyA(LPSTR a, LPCSTR b);
int lstrlenA(LPCSTR s);
int lstrcmpiA(LPCSTR a, LPCSTR b);
int wsprintfA(LPSTR buf, LPCSTR fmt, ...);
int wvsprintfA(LPSTR buf, LPCSTR fmt, va_list args);
#define wsprintf wsprintfA
#define wvsprintf wvsprintfA
#define lstrcat lstrcatA
#define lstrcpy lstrcpyA
#define lstrlen lstrlenA
#define LoadLibrary LoadLibraryA
#define OutputDebugString OutputDebugStringA
#define GetModuleHandle GetModuleHandleA
#define GetModuleFileName GetModuleFileNameA
#define GetVersionEx GetVersionExA
#define FindFirstFile FindFirstFileA
#define FindNextFile FindNextFileA

// ---- user32
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagMSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG, *PMSG, *LPMSG;

typedef struct tagWNDCLASSA
{
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASSA, *LPWNDCLASSA, WNDCLASS;

typedef struct tagWINDOWPOS
{
    HWND hwnd;
    HWND hwndInsertAfter;
    int x;
    int y;
    int cx;
    int cy;
    UINT flags;
} WINDOWPOS, *LPWINDOWPOS, *PWINDOWPOS;

#define WM_NULL 0x0000
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_ERASEBKGND 0x0014
#define WM_ACTIVATEAPP 0x001C
#define WM_SETCURSOR 0x0020
#define WM_WINDOWPOSCHANGING 0x0046
#define WM_WINDOWPOSCHANGED 0x0047
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_SYSCOMMAND 0x0112
#define WM_MOUSEMOVE 0x0200
#define WM_SIZING 0x0214
#define WM_USER 0x0400
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define SC_SIZE 0xF000
#define SC_MOVE 0xF010
#define SC_MINIMIZE 0xF020
#define SC_MAXIMIZE 0xF030
#define SC_CLOSE 0xF060
#define SC_KEYMENU 0xF100
#define SC_SCREENSAVE 0xF140
#define SC_MONITORPOWER 0xF170
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_F1 0x70
#define VK_F10 0x79
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define MB_OK 0x00
#define MB_OKCANCEL 0x01
#define MB_YESNO 0x04
#define MB_ICONERROR 0x10
#define MB_ICONSTOP 0x10
#define MB_ICONEXCLAMATION 0x30
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_SYSTEMMODAL 0x1000
#define MB_TOPMOST 0x40000
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_OWNDC 0x0020
#define WS_OVERLAPPED 0x00000000L
#define WS_POPUP 0x80000000L
#define WS_VISIBLE 0x10000000L
#define WS_CAPTION 0x00C00000L
#define WS_BORDER 0x00800000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_EX_TOPMOST 0x00000008L
#define WS_EX_APPWINDOW 0x00040000L
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOACTIVATE 0x0010
#define SWP_SHOWWINDOW 0x0040
#define HWND_TOP ((HWND)0)
#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define IDC_ARROW MAKEINTRESOURCE(32512)
#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define COLOR_WINDOW 5
#define BLACK_BRUSH 4
#define KL_NAMELENGTH 9
#define CW_USEDEFAULT ((int)0x80000000)

int MessageBoxA(HWND hwnd, LPCSTR text, LPCSTR caption, UINT type);
LRESULT DefWindowProcA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
ATOM RegisterClassA(const WNDCLASSA* wc);
HWND CreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style, int x, int y, int w, int h,
                     HWND parent, HMENU menu, HINSTANCE instance, LPVOID param);
BOOL DestroyWindow(HWND hwnd);
BOOL ShowWindow(HWND hwnd, int cmd);
BOOL UpdateWindow(HWND hwnd);
BOOL SetForegroundWindow(HWND hwnd);
LONG SetWindowLongA(HWND hwnd, int index, LONG value);
LONG GetWindowLongA(HWND hwnd, int index);
BOOL SetWindowPos(HWND hwnd, HWND after, int x, int y, int cx, int cy, UINT flags);
BOOL GetClientRect(HWND hwnd, LPRECT rect);
BOOL GetWindowRect(HWND hwnd, LPRECT rect);
BOOL AdjustWindowRect(LPRECT rect, DWORD style, BOOL menu);
HWND FindWindowA(LPCSTR className, LPCSTR windowName);
HWND FindWindowExA(HWND parent, HWND after, LPCSTR className, LPCSTR windowName);
BOOL PeekMessageA(LPMSG msg, HWND hwnd, UINT min, UINT max, UINT remove);
BOOL GetMessageA(LPMSG msg, HWND hwnd, UINT min, UINT max);
BOOL TranslateMessage(const MSG* msg);
LRESULT DispatchMessageA(const MSG* msg);
BOOL PostMessageA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT SendMessageA(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PostQuitMessage(int code);
int ShowCursor(BOOL show);
HICON LoadIconA(HINSTANCE instance, LPCSTR name);
HCURSOR LoadCursorA(HINSTANCE instance, LPCSTR name);
HCURSOR SetCursor(HCURSOR cursor);
int GetSystemMetrics(int index);
HDC GetDC(HWND hwnd);
int ReleaseDC(HWND hwnd, HDC dc);
BOOL GetKeyboardLayoutNameA(LPSTR name);
short GetAsyncKeyState(int key);
short GetKeyState(int key);
BOOL SetWindowTextA(HWND hwnd, LPCSTR text);
HWND GetActiveWindow(void);
HWND GetForegroundWindow(void);
BOOL IsIconic(HWND hwnd);
BOOL InvalidateRect(HWND hwnd, const RECT* rect, BOOL erase);
BOOL ValidateRect(HWND hwnd, const RECT* rect);
#define MessageBox MessageBoxA
#define DefWindowProc DefWindowProcA
#define RegisterClass RegisterClassA
#define CreateWindowEx CreateWindowExA
#define CreateWindowA(cls, name, style, x, y, w, h, parent, menu, inst, param) \
    CreateWindowExA(0, cls, name, style, x, y, w, h, parent, menu, inst, param)
#define CreateWindow CreateWindowA
#define SetWindowLong SetWindowLongA
#define GetWindowLong GetWindowLongA
#define FindWindow FindWindowA
#define FindWindowEx FindWindowExA
#define PeekMessage PeekMessageA
#define GetMessage GetMessageA
#define DispatchMessage DispatchMessageA
#define PostMessage PostMessageA
#define SendMessage SendMessageA
#define LoadIcon LoadIconA
#define LoadCursor LoadCursorA
#define WNDCLASS WNDCLASSA

// ---- dialogs and standard controls (the network lobby)
typedef INT_PTR (*DLGPROC)(HWND, UINT, WPARAM, LPARAM);
typedef void (*TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
#define WM_INITDIALOG 0x0110
#define WM_COMMAND 0x0111
#define WM_TIMER 0x0113
#define WM_HSCROLL 0x0114
#define WM_VSCROLL 0x0115
#define GWL_WNDPROC (-4)
#define KF_ALTDOWN 0x2000
#define BM_GETCHECK 0x00F0
#define BM_SETCHECK 0x00F1
#define BST_UNCHECKED 0x0000
#define BST_CHECKED 0x0001
#define EN_CHANGE 0x0300
#define EM_LINESCROLL 0x00B6
#define EM_GETLINECOUNT 0x00BA
#define CB_ADDSTRING 0x0143
#define CB_GETCURSEL 0x0147
#define CB_SETCURSEL 0x014E
#define CB_GETITEMDATA 0x0150
#define CB_SETITEMDATA 0x0151
#define CB_RESETCONTENT 0x014B
#define CBN_SELCHANGE 1
#define LB_ADDSTRING 0x0180
#define LB_RESETCONTENT 0x0184
#define LB_SETCURSEL 0x0186
#define LB_GETCURSEL 0x0188
#define LB_GETTEXT 0x0189
#define LB_SELECTSTRING 0x018C
#define LB_GETITEMDATA 0x0199
#define LB_SETITEMDATA 0x019A
#define LB_ERR (-1)
#define LB_ERRSPACE (-2)
#define LBN_DBLCLK 2
#define TBM_GETPOS (WM_USER)
#define TBM_SETPOS (WM_USER + 5)
#define TBM_SETRANGE (WM_USER + 6)

INT_PTR DialogBoxParamA(HINSTANCE instance, LPCSTR templateName, HWND parent, DLGPROC proc, LPARAM param);
BOOL EndDialog(HWND dlg, INT_PTR result);
HWND GetDlgItem(HWND dlg, int id);
int GetDlgCtrlID(HWND hwnd);
UINT GetDlgItemTextA(HWND dlg, int id, LPSTR text, int max);
BOOL SetDlgItemTextA(HWND dlg, int id, LPCSTR text);
LRESULT SendDlgItemMessageA(HWND dlg, int id, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL EnableWindow(HWND hwnd, BOOL enable);
HWND GetParent(HWND hwnd);
int GetWindowTextA(HWND hwnd, LPSTR text, int max);
LRESULT CallWindowProcA(WNDPROC proc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
UINT_PTR SetTimer(HWND hwnd, UINT_PTR id, UINT ms, TIMERPROC proc);
BOOL KillTimer(HWND hwnd, UINT_PTR id);
#define DialogBoxParam DialogBoxParamA
#define DialogBoxA(inst, tmpl, parent, proc) DialogBoxParamA(inst, tmpl, parent, proc, 0)
#define DialogBox DialogBoxA
#define GetDlgItemText GetDlgItemTextA
#define SetDlgItemText SetDlgItemTextA
#define SendDlgItemMessage SendDlgItemMessageA
#define GetWindowText GetWindowTextA
#define CallWindowProc CallWindowProcA

// ---- gdi32
#define BITSPIXEL 12
#define HORZRES 8
#define VERTRES 10
#define PLANES 14

typedef struct tagPALETTEENTRY
{
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY, *PPALETTEENTRY, *LPPALETTEENTRY;

typedef struct _RGNDATAHEADER
{
    DWORD dwSize;
    DWORD iType;
    DWORD nCount;
    DWORD nRgnSize;
    RECT rcBound;
} RGNDATAHEADER;

typedef struct _RGNDATA
{
    RGNDATAHEADER rdh;
    char Buffer[1];
} RGNDATA, *PRGNDATA, *LPRGNDATA;

int GetDeviceCaps(HDC dc, int index);
HGDIOBJ GetStockObject(int index);

// ---- advapi32
typedef LONG LSTATUS;
typedef DWORD REGSAM;
#define HKEY_CLASSES_ROOT ((HKEY)(ULONG_PTR)0x80000000)
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)0x80000002)
#define HKEY_USERS ((HKEY)(ULONG_PTR)0x80000003)
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_MORE_DATA 234L
#define REG_NONE 0
#define REG_SZ 1
#define REG_BINARY 3
#define REG_DWORD 4
#define REG_OPTION_NON_VOLATILE 0
#define KEY_QUERY_VALUE 0x0001
#define KEY_SET_VALUE 0x0002
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
#define REG_CREATED_NEW_KEY 1
#define REG_OPENED_EXISTING_KEY 2

LSTATUS RegOpenKeyA(HKEY key, LPCSTR subKey, PHKEY result);
LSTATUS RegOpenKeyExA(HKEY key, LPCSTR subKey, DWORD options, REGSAM sam, PHKEY result);
LSTATUS RegCreateKeyExA(HKEY key, LPCSTR subKey, DWORD reserved, LPSTR cls, DWORD options, REGSAM sam,
                        LPSECURITY_ATTRIBUTES sa, PHKEY result, LPDWORD disposition);
LSTATUS RegCloseKey(HKEY key);
LSTATUS RegQueryValueExA(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size);
LSTATUS RegSetValueExA(HKEY key, LPCSTR name, DWORD reserved, DWORD type, const BYTE* data, DWORD size);
LSTATUS RegDeleteValueA(HKEY key, LPCSTR name);
#define RegOpenKey RegOpenKeyA
#define RegOpenKeyEx RegOpenKeyExA
#define RegCreateKeyEx RegCreateKeyExA
#define RegQueryValueEx RegQueryValueExA
#define RegSetValueEx RegSetValueExA

// ---- version
typedef struct tagVS_FIXEDFILEINFO
{
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;

DWORD GetFileVersionInfoSizeA(LPCSTR file, LPDWORD handle);
BOOL GetFileVersionInfoA(LPCSTR file, DWORD handle, DWORD len, LPVOID data);
BOOL VerQueryValueA(const void* block, LPCSTR subBlock, LPVOID* buffer, PUINT len);

// ---- shell32 / shlwapi
HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR op, LPCSTR file, LPCSTR params, LPCSTR dir, int show);
LPSTR StrStrIA(LPCSTR haystack, LPCSTR needle);

#ifdef __cplusplus
}
#endif
