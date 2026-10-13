// DirectInput 5 subset for the native build. The interfaces are implemented over SDL in
// port/platform/; layouts of the structures follow the SDK's.
#pragma once
#include <windows.h>
#include <objbase.h>

#ifndef DIRECTINPUT_VERSION
    #define DIRECTINPUT_VERSION 0x0500
#endif

#define SEVERITY_ERROR 1
#define FACILITY_WIN32 7
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_ACCESSDENIED ((HRESULT)0x80070005L)
#define CLASS_E_NOAGGREGATION ((HRESULT)0x80040110L)
#define REGDB_E_CLASSNOTREG ((HRESULT)0x80040154L)

// ---- return codes
#define DI_OK S_OK
#define DI_NOTATTACHED S_FALSE
#define DI_BUFFEROVERFLOW S_FALSE
#define DI_PROPNOEFFECT S_FALSE
#define DI_NOEFFECT S_FALSE
#define DI_POLLEDDEVICE ((HRESULT)0x00000002L)
#define DI_DOWNLOADSKIPPED ((HRESULT)0x00000003L)
#define DI_EFFECTRESTARTED ((HRESULT)0x00000004L)
#define DI_TRUNCATED ((HRESULT)0x00000008L)
#define DI_TRUNCATEDANDRESTARTED ((HRESULT)0x0000000CL)
#define DIERR_OLDDIRECTINPUTVERSION MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 1150)
#define DIERR_BETADIRECTINPUTVERSION MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 1153)
#define DIERR_BADDRIVERVER MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 119)
#define DIERR_DEVICENOTREG REGDB_E_CLASSNOTREG
#define DIERR_NOTFOUND MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 2)
#define DIERR_OBJECTNOTFOUND MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 2)
#define DIERR_INVALIDPARAM E_INVALIDARG
#define DIERR_NOINTERFACE E_NOINTERFACE
#define DIERR_GENERIC E_FAIL
#define DIERR_OUTOFMEMORY E_OUTOFMEMORY
#define DIERR_UNSUPPORTED E_NOTIMPL
#define DIERR_NOTINITIALIZED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 21)
#define DIERR_ALREADYINITIALIZED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 1247)
#define DIERR_NOAGGREGATION CLASS_E_NOAGGREGATION
#define DIERR_OTHERAPPHASPRIO E_ACCESSDENIED
#define DIERR_INPUTLOST MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 30)
#define DIERR_ACQUIRED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 170)
#define DIERR_NOTACQUIRED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 12)
#define DIERR_READONLY E_ACCESSDENIED
#define DIERR_HANDLEEXISTS E_ACCESSDENIED
#define DIERR_INSUFFICIENTPRIVS ((HRESULT)0x80040200L)
#define DIERR_DEVICEFULL ((HRESULT)0x80040201L)
#define DIERR_MOREDATA ((HRESULT)0x80040202L)
#define DIERR_NOTDOWNLOADED ((HRESULT)0x80040203L)
#define DIERR_HASEFFECTS ((HRESULT)0x80040204L)
#define DIERR_NOTEXCLUSIVEACQUIRED ((HRESULT)0x80040205L)
#define DIERR_INCOMPLETEEFFECT ((HRESULT)0x80040206L)
#define DIERR_NOTBUFFERED ((HRESULT)0x80040207L)
#define DIERR_EFFECTPLAYING ((HRESULT)0x80040208L)
#define DIERR_UNPLUGGED ((HRESULT)0x80040209L)

// ---- flags
#define DIDF_ABSAXIS 0x00000001
#define DIDF_RELAXIS 0x00000002
#define DIEDFL_ALLDEVICES 0x00000000
#define DIEDFL_ATTACHEDONLY 0x00000001
#define DIENUM_STOP 0
#define DIENUM_CONTINUE 1
#define DIGDD_PEEK 0x00000001
#define DIPH_DEVICE 0
#define DIPH_BYOFFSET 1
#define DIPH_BYID 2
#define DISCL_EXCLUSIVE 0x00000001
#define DISCL_NONEXCLUSIVE 0x00000002
#define DISCL_FOREGROUND 0x00000004
#define DISCL_BACKGROUND 0x00000008
#define DIDEVTYPE_DEVICE 1
#define DIDEVTYPE_MOUSE 2
#define DIDEVTYPE_KEYBOARD 3
#define DIDEVTYPE_JOYSTICK 4
#define GET_DIDEVICE_TYPE(dwDevType) LOBYTE(dwDevType)
#define DIDFT_ALL 0x00000000
#define DIDFT_RELAXIS 0x00000001
#define DIDFT_ABSAXIS 0x00000002
#define DIDFT_AXIS 0x00000003
#define DIDFT_PSHBUTTON 0x00000004
#define DIDFT_TGLBUTTON 0x00000008
#define DIDFT_BUTTON 0x0000000C
#define DIDFT_POV 0x00000010
#define DIDFT_ANYINSTANCE 0x00FFFF00
#define DIDFT_OPTIONAL 0x80000000
#define DIDFT_MAKEINSTANCE(n) ((WORD)(n) << 8)

// ---- structures
typedef struct _DIOBJECTDATAFORMAT
{
    const GUID* pguid;
    DWORD dwOfs;
    DWORD dwType;
    DWORD dwFlags;
} DIOBJECTDATAFORMAT, *LPDIOBJECTDATAFORMAT;
typedef const DIOBJECTDATAFORMAT* LPCDIOBJECTDATAFORMAT;

typedef struct _DIDATAFORMAT
{
    DWORD dwSize;
    DWORD dwObjSize;
    DWORD dwFlags;
    DWORD dwDataSize;
    DWORD dwNumObjs;
    LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, *LPDIDATAFORMAT;
typedef const DIDATAFORMAT* LPCDIDATAFORMAT;

typedef struct DIDEVICEINSTANCEA
{
    DWORD dwSize;
    GUID guidInstance;
    GUID guidProduct;
    DWORD dwDevType;
    CHAR tszInstanceName[MAX_PATH];
    CHAR tszProductName[MAX_PATH];
    GUID guidFFDriver;
    WORD wUsagePage;
    WORD wUsage;
} DIDEVICEINSTANCEA, *LPDIDEVICEINSTANCEA;
typedef const DIDEVICEINSTANCEA* LPCDIDEVICEINSTANCEA;

typedef struct DIPROPHEADER
{
    DWORD dwSize;
    DWORD dwHeaderSize;
    DWORD dwObj;
    DWORD dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;
typedef const DIPROPHEADER* LPCDIPROPHEADER;

typedef struct DIPROPDWORD
{
    DIPROPHEADER diph;
    DWORD dwData;
} DIPROPDWORD, *LPDIPROPDWORD;

typedef struct DIPROPRANGE
{
    DIPROPHEADER diph;
    LONG lMin;
    LONG lMax;
} DIPROPRANGE, *LPDIPROPRANGE;

typedef struct DIDEVICEOBJECTDATA
{
    DWORD dwOfs;
    DWORD dwData;
    DWORD dwTimeStamp;
    DWORD dwSequence;
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

typedef struct DIJOYSTATE
{
    LONG lX;
    LONG lY;
    LONG lZ;
    LONG lRx;
    LONG lRy;
    LONG lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE rgbButtons[32];
} DIJOYSTATE, *LPDIJOYSTATE;

typedef struct _DIMOUSESTATE
{
    LONG lX;
    LONG lY;
    LONG lZ;
    BYTE rgbButtons[4];
} DIMOUSESTATE, *LPDIMOUSESTATE;

typedef struct DIDEVCAPS
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDevType;
    DWORD dwAxes;
    DWORD dwButtons;
    DWORD dwPOVs;
    DWORD dwFFSamplePeriod;
    DWORD dwFFMinTimeResolution;
    DWORD dwFirmwareRevision;
    DWORD dwHardwareRevision;
    DWORD dwFFDriverVersion;
} DIDEVCAPS, *LPDIDEVCAPS;

#define DIJOFS_X offsetof(DIJOYSTATE, lX)
#define DIJOFS_Y offsetof(DIJOYSTATE, lY)
#define DIJOFS_Z offsetof(DIJOYSTATE, lZ)
#define DIJOFS_RX offsetof(DIJOYSTATE, lRx)
#define DIJOFS_RY offsetof(DIJOYSTATE, lRy)
#define DIJOFS_RZ offsetof(DIJOYSTATE, lRz)
#define DIJOFS_SLIDER(n) (offsetof(DIJOYSTATE, rglSlider) + (n) * sizeof(LONG))
#define DIJOFS_POV(n) (offsetof(DIJOYSTATE, rgdwPOV) + (n) * sizeof(DWORD))
#define DIJOFS_BUTTON(n) (offsetof(DIJOYSTATE, rgbButtons) + (n))

// Properties are identified by small integers cast to a GUID reference, as in the SDK
#define MAKEDIPROP(prop) (*(const GUID*)(prop))
#define DIPROP_BUFFERSIZE MAKEDIPROP(1)
#define DIPROP_AXISMODE MAKEDIPROP(2)
#define DIPROP_GRANULARITY MAKEDIPROP(3)
#define DIPROP_RANGE MAKEDIPROP(4)
#define DIPROP_DEADZONE MAKEDIPROP(5)
#define DIPROP_SATURATION MAKEDIPROP(6)
#define DIPROP_FFGAIN MAKEDIPROP(7)
#define DIPROP_AUTOCENTER MAKEDIPROP(9)

typedef BOOL (*LPDIENUMDEVICESCALLBACKA)(LPCDIDEVICEINSTANCEA, LPVOID);

// ---- interfaces (methods in the SDK's vtable order)
struct IDirectInputDeviceA : public IUnknown
{
    virtual HRESULT GetCapabilities(LPDIDEVCAPS caps) = 0;
    virtual HRESULT EnumObjects(LPVOID callback, LPVOID ref, DWORD flags) = 0;
    virtual HRESULT GetProperty(REFGUID prop, LPDIPROPHEADER header) = 0;
    virtual HRESULT SetProperty(REFGUID prop, LPCDIPROPHEADER header) = 0;
    virtual HRESULT Acquire() = 0;
    virtual HRESULT Unacquire() = 0;
    virtual HRESULT GetDeviceState(DWORD size, LPVOID data) = 0;
    virtual HRESULT GetDeviceData(DWORD objSize, LPDIDEVICEOBJECTDATA data, LPDWORD inOut, DWORD flags) = 0;
    virtual HRESULT SetDataFormat(LPCDIDATAFORMAT format) = 0;
    virtual HRESULT SetEventNotification(HANDLE event) = 0;
    virtual HRESULT SetCooperativeLevel(HWND hwnd, DWORD flags) = 0;
    virtual HRESULT GetObjectInfo(LPVOID info, DWORD obj, DWORD how) = 0;
    virtual HRESULT GetDeviceInfo(LPDIDEVICEINSTANCEA info) = 0;
    virtual HRESULT RunControlPanel(HWND owner, DWORD flags) = 0;
    virtual HRESULT Initialize(HINSTANCE instance, DWORD version, REFGUID guid) = 0;
};

struct IDirectInputDevice2A : public IDirectInputDeviceA
{
    virtual HRESULT CreateEffect(REFGUID guid, LPVOID effect, LPVOID* out, LPUNKNOWN outer) = 0;
    virtual HRESULT EnumEffects(LPVOID callback, LPVOID ref, DWORD type) = 0;
    virtual HRESULT GetEffectInfo(LPVOID info, REFGUID guid) = 0;
    virtual HRESULT GetForceFeedbackState(LPDWORD out) = 0;
    virtual HRESULT SendForceFeedbackCommand(DWORD flags) = 0;
    virtual HRESULT EnumCreatedEffectObjects(LPVOID callback, LPVOID ref, DWORD flags) = 0;
    virtual HRESULT Escape(LPVOID escape) = 0;
    virtual HRESULT Poll() = 0;
    virtual HRESULT SendDeviceData(DWORD objSize, LPDIDEVICEOBJECTDATA data, LPDWORD inOut, DWORD flags) = 0;
};

struct IDirectInputA : public IUnknown
{
    virtual HRESULT CreateDevice(REFGUID guid, IDirectInputDeviceA** device, LPUNKNOWN outer) = 0;
    virtual HRESULT EnumDevices(DWORD devType, LPDIENUMDEVICESCALLBACKA callback, LPVOID ref, DWORD flags) = 0;
    virtual HRESULT GetDeviceStatus(REFGUID guid) = 0;
    virtual HRESULT RunControlPanel(HWND owner, DWORD flags) = 0;
    virtual HRESULT Initialize(HINSTANCE instance, DWORD version) = 0;
};

typedef IDirectInputA* LPDIRECTINPUTA;
typedef IDirectInputA* LPDIRECTINPUT;
typedef IDirectInputDeviceA* LPDIRECTINPUTDEVICEA;
typedef IDirectInputDeviceA* LPDIRECTINPUTDEVICE;
typedef IDirectInputDevice2A* LPDIRECTINPUTDEVICE2A;
typedef IDirectInputDevice2A* LPDIRECTINPUTDEVICE2;
typedef IDirectInputA IDirectInput;
typedef IDirectInputDeviceA IDirectInputDevice;
typedef IDirectInputDevice2A IDirectInputDevice2;

extern "C" {
HRESULT DirectInputCreateA(HINSTANCE instance, DWORD version, LPDIRECTINPUTA* out, LPUNKNOWN outer);
extern const GUID GUID_SysKeyboard;
extern const GUID GUID_SysMouse;
extern const GUID GUID_Joystick;
extern const GUID GUID_XAxis;
extern const GUID GUID_YAxis;
extern const GUID GUID_ZAxis;
extern const GUID GUID_RxAxis;
extern const GUID GUID_RyAxis;
extern const GUID GUID_RzAxis;
extern const GUID GUID_Slider;
extern const GUID GUID_Button;
extern const GUID GUID_Key;
extern const GUID GUID_POV;
extern const GUID IID_IDirectInputA;
extern const GUID IID_IDirectInputDeviceA;
extern const GUID IID_IDirectInputDevice2A;
extern const DIDATAFORMAT c_dfDIKeyboard;
extern const DIDATAFORMAT c_dfDIMouse;
extern const DIDATAFORMAT c_dfDIJoystick;
}
#define DirectInputCreate DirectInputCreateA

// ---- keyboard scan codes (values from the SDK's DINPUT.H)
#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0A
#define DIK_0 0x0B
#define DIK_MINUS 0x0C
#define DIK_EQUALS 0x0D
#define DIK_BACK 0x0E
#define DIK_TAB 0x0F
#define DIK_Q 0x10
#define DIK_W 0x11
#define DIK_E 0x12
#define DIK_R 0x13
#define DIK_T 0x14
#define DIK_Y 0x15
#define DIK_U 0x16
#define DIK_I 0x17
#define DIK_O 0x18
#define DIK_P 0x19
#define DIK_LBRACKET 0x1A
#define DIK_RBRACKET 0x1B
#define DIK_RETURN 0x1C
#define DIK_LCONTROL 0x1D
#define DIK_A 0x1E
#define DIK_S 0x1F
#define DIK_D 0x20
#define DIK_F 0x21
#define DIK_G 0x22
#define DIK_H 0x23
#define DIK_J 0x24
#define DIK_K 0x25
#define DIK_L 0x26
#define DIK_SEMICOLON 0x27
#define DIK_APOSTROPHE 0x28
#define DIK_GRAVE 0x29
#define DIK_LSHIFT 0x2A
#define DIK_BACKSLASH 0x2B
#define DIK_Z 0x2C
#define DIK_X 0x2D
#define DIK_C 0x2E
#define DIK_V 0x2F
#define DIK_B 0x30
#define DIK_N 0x31
#define DIK_M 0x32
#define DIK_COMMA 0x33
#define DIK_PERIOD 0x34
#define DIK_SLASH 0x35
#define DIK_RSHIFT 0x36
#define DIK_MULTIPLY 0x37
#define DIK_LMENU 0x38
#define DIK_SPACE 0x39
#define DIK_CAPITAL 0x3A
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F3 0x3D
#define DIK_F4 0x3E
#define DIK_F5 0x3F
#define DIK_F6 0x40
#define DIK_F7 0x41
#define DIK_F8 0x42
#define DIK_F9 0x43
#define DIK_F10 0x44
#define DIK_NUMLOCK 0x45
#define DIK_SCROLL 0x46
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_SUBTRACT 0x4A
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPAD5 0x4C
#define DIK_NUMPAD6 0x4D
#define DIK_ADD 0x4E
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD0 0x52
#define DIK_DECIMAL 0x53
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_F13 0x64
#define DIK_F14 0x65
#define DIK_F15 0x66
#define DIK_KANA 0x70
#define DIK_CONVERT 0x79
#define DIK_NOCONVERT 0x7B
#define DIK_YEN 0x7D
#define DIK_NUMPADEQUALS 0x8D
#define DIK_CIRCUMFLEX 0x90
#define DIK_AT 0x91
#define DIK_COLON 0x92
#define DIK_UNDERLINE 0x93
#define DIK_KANJI 0x94
#define DIK_STOP 0x95
#define DIK_AX 0x96
#define DIK_UNLABELED 0x97
#define DIK_NUMPADENTER 0x9C
#define DIK_RCONTROL 0x9D
#define DIK_NUMPADCOMMA 0xB3
#define DIK_DIVIDE 0xB5
#define DIK_SYSRQ 0xB7
#define DIK_RMENU 0xB8
#define DIK_PAUSE 0xC5
#define DIK_HOME 0xC7
#define DIK_UP 0xC8
#define DIK_PRIOR 0xC9
#define DIK_LEFT 0xCB
#define DIK_RIGHT 0xCD
#define DIK_END 0xCF
#define DIK_DOWN 0xD0
#define DIK_NEXT 0xD1
#define DIK_INSERT 0xD2
#define DIK_DELETE 0xD3
#define DIK_LWIN 0xDB
#define DIK_RWIN 0xDC
#define DIK_APPS 0xDD
#define DIK_POWER 0xDE
#define DIK_SLEEP 0xDF
#define DIK_BACKSPACE DIK_BACK
#define DIK_NUMPADSTAR DIK_MULTIPLY
#define DIK_LALT DIK_LMENU
#define DIK_CAPSLOCK DIK_CAPITAL
#define DIK_NUMPADMINUS DIK_SUBTRACT
#define DIK_NUMPADPLUS DIK_ADD
#define DIK_NUMPADPERIOD DIK_DECIMAL
#define DIK_NUMPADSLASH DIK_DIVIDE
#define DIK_RALT DIK_RMENU
#define DIK_UPARROW DIK_UP
#define DIK_PGUP DIK_PRIOR
#define DIK_LEFTARROW DIK_LEFT
#define DIK_RIGHTARROW DIK_RIGHT
#define DIK_DOWNARROW DIK_DOWN
#define DIK_PGDN DIK_NEXT
