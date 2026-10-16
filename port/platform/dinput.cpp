// DirectInput 5 over the SDL platform layer: the system keyboard, the system mouse and SDL game
// controllers as joysticks. As real DirectInput does, a device lays out its state and its buffered
// events by the data format the game sets (the game defines its own: gKeyboardDataFormat_601A54,
// gInputDeviceFormat_601A6C, gMouseDataFormat_601A84).
#include "controls.h"
#include "events.h"
#include "overlay.h"
#include "platform.h"

#include <dinput.h>

#include <deque>
#include <vector>

namespace
{
#define DIDFT_GETTYPE(n) LOBYTE(n)
#define DIDFT_GETINSTANCE(n) LOWORD((n) >> 8)
const DWORD kAnyInstance = 0xFFFF;

enum class Kind
{
    Keyboard,
    Mouse,
    Pad
};

// A pad's instance GUID: GUID_Joystick with the pad index in the last byte
int PadIndexFromGuid(const GUID& g)
{
    GUID base = g;
    base.Data4[7] = GUID_Joystick.Data4[7];
    if (base != GUID_Joystick || !g.Data4[7])
    {
        return -1;
    }
    return g.Data4[7] - 1;
}

class DirectInputDevice;
std::vector<DirectInputDevice*> gKeyboards; // receive key events from the pump
DWORD gSequence = 0;

// What one object in the game's data format reads from
enum class Source
{
    None,
    Key, // mKey = DIK code
    AxisX,
    AxisY,
    AxisZ,
    Pov,
    Button, // mIndex = button number
};

struct Object
{
    DIOBJECTDATAFORMAT mFormat;
    Source mSource;
    int mIndex;
    LONG mMin;
    LONG mMax;
    DWORD mDeadZone; // 0..10000
    DWORD mLast; // last value reported, for buffered events
    mutable int mDirection = 0; // an axis held past its trigger point: -1, 0 or 1
};

class DirectInputDevice final : public IDirectInputDevice2A
{
  public:
    DirectInputDevice(Kind kind, const GUID& guid, int padIndex) :
        mKind(kind), mGuid(guid), mPadIndex(padIndex), mRefs(1), mStateSize(0), mBufferSize(0), mAcquired(false)
    {
        if (mKind == Kind::Keyboard)
        {
            gKeyboards.push_back(this);
        }
    }

    ~DirectInputDevice()
    {
        for (size_t i = 0; i < gKeyboards.size(); i++)
        {
            if (gKeyboards[i] == this)
            {
                gKeyboards.erase(gKeyboards.begin() + i);
                break;
            }
        }
    }

    void OnKey(uint8_t dik, bool down)
    {
        if (!mAcquired || !mBufferSize)
        {
            return;
        }
        for (Object& o : mObjects)
        {
            if (o.mSource == Source::Key && o.mIndex == dik)
            {
                Push(o.mFormat.dwOfs, down ? 0x80 : 0);
                return;
            }
        }
        if (mObjects.empty())
        {
            Push(dik, down ? 0x80 : 0); // no format set: offsets are the scan codes
        }
    }

    // ---- IUnknown
    HRESULT QueryInterface(REFIID riid, void** out) override
    {
        if (riid == IID_IDirectInputDeviceA || riid == IID_IDirectInputDevice2A)
        {
            AddRef();
            *out = this;
            return S_OK;
        }
        *out = 0;
        return E_NOINTERFACE;
    }
    ULONG AddRef() override
    {
        return ++mRefs;
    }
    ULONG Release() override
    {
        ULONG refs = --mRefs;
        if (!refs)
        {
            delete this;
        }
        return refs;
    }

    // ---- IDirectInputDeviceA
    HRESULT GetCapabilities(LPDIDEVCAPS caps) override
    {
        DWORD size = caps->dwSize;
        memset(caps, 0, size);
        caps->dwSize = size;
        caps->dwFlags = 1; // DIDC_ATTACHED
        caps->dwDevType = DevType();
        caps->dwAxes = mKind == Kind::Keyboard ? 0 : 2;
        caps->dwButtons = mKind == Kind::Keyboard ? 128 : (mKind == Kind::Mouse ? 3 : 12);
        caps->dwPOVs = mKind == Kind::Pad ? 1 : 0;
        return DI_OK;
    }
    HRESULT EnumObjects(LPVOID, LPVOID, DWORD) override
    {
        return DI_OK;
    }
    HRESULT GetProperty(REFGUID prop, LPDIPROPHEADER header) override
    {
        if (&prop == &DIPROP_BUFFERSIZE)
        {
            reinterpret_cast<LPDIPROPDWORD>(header)->dwData = mBufferSize;
            return DI_OK;
        }
        if (&prop == &DIPROP_RANGE)
        {
            Object* o = Find(header);
            LPDIPROPRANGE range = reinterpret_cast<LPDIPROPRANGE>(header);
            range->lMin = o ? o->mMin : 0;
            range->lMax = o ? o->mMax : 65535;
            return DI_OK;
        }
        return DIERR_UNSUPPORTED;
    }
    HRESULT SetProperty(REFGUID prop, LPCDIPROPHEADER header) override
    {
        if (&prop == &DIPROP_BUFFERSIZE)
        {
            mBufferSize = reinterpret_cast<const DIPROPDWORD*>(header)->dwData;
            return DI_OK;
        }
        if (&prop == &DIPROP_RANGE || &prop == &DIPROP_DEADZONE)
        {
            for (Object& o : mObjects)
            {
                if (header->dwHow == DIPH_DEVICE || (header->dwHow == DIPH_BYOFFSET && o.mFormat.dwOfs == header->dwObj))
                {
                    if (&prop == &DIPROP_RANGE)
                    {
                        o.mMin = reinterpret_cast<const DIPROPRANGE*>(header)->lMin;
                        o.mMax = reinterpret_cast<const DIPROPRANGE*>(header)->lMax;
                    }
                    else
                    {
                        o.mDeadZone = reinterpret_cast<const DIPROPDWORD*>(header)->dwData;
                    }
                }
            }
            return DI_OK;
        }
        return DI_PROPNOEFFECT;
    }
    HRESULT Acquire() override
    {
        if (mAcquired)
        {
            return S_FALSE;
        }
        mAcquired = true;
        mQueue.clear();
        Sample(false); // the state at acquisition is the baseline, not a burst of events
        return DI_OK;
    }
    HRESULT Unacquire() override
    {
        if (!mAcquired)
        {
            return DI_NOEFFECT;
        }
        mAcquired = false;
        return DI_OK;
    }
    HRESULT GetDeviceState(DWORD size, LPVOID data) override
    {
        if (!mAcquired)
        {
            return DIERR_NOTACQUIRED;
        }
        port_pump_events();
        memset(data, 0, size);
        BYTE* out = static_cast<BYTE*>(data);
        if (mObjects.empty())
        {
            if (mKind == Kind::Keyboard && size >= 256 && !overlay::Captured())
            {
                plat::GetKeyboardState(out);
                controls::MenuKeys(out); // the pad in the front end's menus
            }
            return DI_OK;
        }
        uint8_t keys[256];
        if (mKind == Kind::Keyboard)
        {
            plat::GetKeyboardState(keys);
            if (overlay::Captured())
            {
                memset(keys, 0, sizeof(keys));
            }
        }
        for (Object& o : mObjects)
        {
            DWORD value = Value(o, keys);
            if (DIDFT_GETTYPE(o.mFormat.dwType) & DIDFT_AXIS || o.mSource == Source::Pov)
            {
                if (o.mFormat.dwOfs + 4 <= size)
                {
                    memcpy(out + o.mFormat.dwOfs, &value, 4);
                }
            }
            else if (o.mFormat.dwOfs < size)
            {
                out[o.mFormat.dwOfs] = static_cast<BYTE>(value);
            }
        }
        return DI_OK;
    }
    HRESULT GetDeviceData(DWORD objSize, LPDIDEVICEOBJECTDATA data, LPDWORD inOut, DWORD flags) override
    {
        if (!mAcquired)
        {
            return DIERR_NOTACQUIRED;
        }
        if (!mBufferSize)
        {
            return DIERR_NOTBUFFERED;
        }
        port_pump_events();
        if (mKind != Kind::Keyboard)
        {
            Sample(true);
        }
        if (!data)
        {
            // Without a buffer: how many there are (DIGDD_PEEK), or discard that many
            DWORD n = static_cast<DWORD>(mQueue.size());
            if (*inOut != INFINITE && *inOut < n && !(flags & DIGDD_PEEK))
            {
                n = *inOut;
            }
            if (!(flags & DIGDD_PEEK))
            {
                mQueue.erase(mQueue.begin(), mQueue.begin() + n);
            }
            *inOut = n;
            return DI_OK;
        }
        DWORD n = 0;
        BYTE* out = reinterpret_cast<BYTE*>(data);
        while (n < *inOut && n < mQueue.size())
        {
            memset(out + n * objSize, 0, objSize);
            memcpy(out + n * objSize, &mQueue[n], objSize < sizeof(DIDEVICEOBJECTDATA) ? objSize : sizeof(DIDEVICEOBJECTDATA));
            n++;
        }
        if (!(flags & DIGDD_PEEK))
        {
            mQueue.erase(mQueue.begin(), mQueue.begin() + n);
        }
        *inOut = n;
        return DI_OK;
    }
    HRESULT SetDataFormat(LPCDIDATAFORMAT format) override
    {
        mObjects.clear();
        mStateSize = format ? format->dwDataSize : 0;
        if (!format)
        {
            return DI_OK;
        }
        int nextButton = 0;
        int nextPov = 0;
        for (DWORD i = 0; i < format->dwNumObjs; i++)
        {
            Object o;
            memset(&o, 0, sizeof(o));
            o.mFormat = format->rgodf[i];
            o.mMin = 0;
            o.mMax = 65535; // DirectInput's default axis range
            const DWORD type = DIDFT_GETTYPE(o.mFormat.dwType);
            const DWORD instance = DIDFT_GETINSTANCE(o.mFormat.dwType);
            const GUID* g = o.mFormat.pguid;
            if (mKind == Kind::Keyboard)
            {
                o.mSource = Source::Key;
                o.mIndex = instance == kAnyInstance ? static_cast<int>(o.mFormat.dwOfs) : static_cast<int>(instance);
            }
            else if (g && *g == GUID_XAxis)
            {
                o.mSource = Source::AxisX;
            }
            else if (g && *g == GUID_YAxis)
            {
                o.mSource = Source::AxisY;
            }
            else if (g && *g == GUID_ZAxis)
            {
                o.mSource = Source::AxisZ;
            }
            else if (g && *g == GUID_POV)
            {
                o.mSource = Source::Pov;
                o.mIndex = nextPov++; // only the first reads the d-pad; the rest report centred
            }
            else if ((!g || *g == GUID_Button) && (type & DIDFT_BUTTON))
            {
                o.mSource = Source::Button;
                o.mIndex = instance == kAnyInstance ? nextButton : static_cast<int>(instance);
                nextButton = o.mIndex + 1;
            }
            else
            {
                o.mSource = Source::None;
            }
            mObjects.push_back(o);
        }
        return DI_OK;
    }
    HRESULT SetEventNotification(HANDLE) override
    {
        return DI_OK;
    }
    HRESULT SetCooperativeLevel(HWND, DWORD) override
    {
        return DI_OK;
    }
    HRESULT GetObjectInfo(LPVOID, DWORD, DWORD) override
    {
        return DIERR_OBJECTNOTFOUND;
    }
    HRESULT GetDeviceInfo(LPDIDEVICEINSTANCEA info) override
    {
        DWORD size = info->dwSize;
        memset(info, 0, size);
        info->dwSize = size;
        info->guidInstance = mGuid;
        info->guidProduct = mGuid;
        info->dwDevType = DevType();
        const char* name = mKind == Kind::Pad ? plat::PadName(mPadIndex) : (mKind == Kind::Keyboard ? "Keyboard" : "Mouse");
        snprintf(info->tszInstanceName, sizeof(info->tszInstanceName), "%s", name);
        snprintf(info->tszProductName, sizeof(info->tszProductName), "%s", name);
        return DI_OK;
    }
    HRESULT RunControlPanel(HWND, DWORD) override
    {
        return DI_OK;
    }
    HRESULT Initialize(HINSTANCE, DWORD, REFGUID) override
    {
        return DI_OK;
    }

    // ---- IDirectInputDevice2A
    HRESULT CreateEffect(REFGUID, LPVOID, LPVOID*, LPUNKNOWN) override
    {
        return DIERR_UNSUPPORTED;
    }
    HRESULT EnumEffects(LPVOID, LPVOID, DWORD) override
    {
        return DI_OK;
    }
    HRESULT GetEffectInfo(LPVOID, REFGUID) override
    {
        return DIERR_UNSUPPORTED;
    }
    HRESULT GetForceFeedbackState(LPDWORD) override
    {
        return DIERR_UNSUPPORTED;
    }
    HRESULT SendForceFeedbackCommand(DWORD) override
    {
        return DIERR_UNSUPPORTED;
    }
    HRESULT EnumCreatedEffectObjects(LPVOID, LPVOID, DWORD) override
    {
        return DI_OK;
    }
    HRESULT Escape(LPVOID) override
    {
        return DIERR_UNSUPPORTED;
    }
    HRESULT Poll() override
    {
        port_pump_events();
        return DI_OK;
    }
    HRESULT SendDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) override
    {
        return DIERR_UNSUPPORTED;
    }

  private:
    DWORD DevType() const
    {
        return mKind == Kind::Keyboard ? DIDEVTYPE_KEYBOARD : (mKind == Kind::Mouse ? DIDEVTYPE_MOUSE : DIDEVTYPE_JOYSTICK);
    }

    Object* Find(LPCDIPROPHEADER header)
    {
        for (Object& o : mObjects)
        {
            if (header->dwHow == DIPH_BYOFFSET && o.mFormat.dwOfs == header->dwObj)
            {
                return &o;
            }
        }
        return mObjects.empty() ? 0 : &mObjects[0];
    }

    // The game reads a stick as four buttons: it acts when the axis passes 75% of its range, which
    // takes a stick pushed nearly to the rim (and its dead zone of 25% comes on top). So the stick
    // is turned into those buttons here: on at 40% of its travel, off again at 25%, and then fully
    // deflected. The d-pad already reports full deflection.
    static int32_t Trigger(const Object& o, int32_t raw)
    {
        const int32_t on = 32767 * 40 / 100;
        const int32_t off = 32767 * 25 / 100;
        const int sign = raw < 0 ? -1 : 1;
        const int32_t size = raw < 0 ? -raw : raw;
        if (o.mDirection != 0 && (o.mDirection != sign || size < off))
        {
            o.mDirection = 0;
        }
        if (o.mDirection == 0 && size > on)
        {
            o.mDirection = sign;
        }
        return o.mDirection * 32767;
    }

    static DWORD ScaleAxis(const Object& o, int32_t raw)
    {
        // Dead zone in ten-thousandths of the full deflection, then into the object's range
        const int32_t dead = static_cast<int32_t>(32767LL * o.mDeadZone / 10000);
        if (raw > -dead && raw < dead)
        {
            raw = 0;
        }
        const long long span = static_cast<long long>(o.mMax) - o.mMin;
        const long long v = o.mMin + (static_cast<long long>(raw) + 32768) * span / 65535;
        return static_cast<DWORD>(static_cast<LONG>(v));
    }

    DWORD Value(const Object& o, const uint8_t* keys)
    {
        switch (o.mSource)
        {
            case Source::Key:
                return (o.mIndex >= 0 && o.mIndex < 256) ? keys[o.mIndex] : 0;
            case Source::AxisX:
            case Source::AxisY:
            case Source::AxisZ:
            case Source::Pov:
            case Source::Button:
            {
                if (mKind == Kind::Mouse)
                {
                    return 0; // the game doesn't steer with the mouse; no motion is reported
                }
                plat::PadState pad;
                plat::GetPadState(mPadIndex, pad);
                if (overlay::Captured()) // the settings screen has the pad: the game sees it at rest
                {
                    memset(&pad, 0, sizeof(pad));
                    pad.pov = -1;
                }
                switch (o.mSource)
                {
                    case Source::AxisX:
                        return ScaleAxis(o, Trigger(o, pad.axisX));
                    case Source::AxisY:
                        return ScaleAxis(o, Trigger(o, pad.axisY));
                    case Source::AxisZ:
                        return ScaleAxis(o, 0);
                    case Source::Pov:
                        return o.mIndex == 0 ? static_cast<DWORD>(pad.pov) : 0xFFFFFFFF;
                    default:
                        return (o.mIndex >= 0 && o.mIndex < 32) ? pad.buttons[o.mIndex] : 0;
                }
            }
            default:
                return 0;
        }
    }

    // Polls the pad and turns changes into buffered events
    void Sample(bool emit)
    {
        if (mKind != Kind::Pad)
        {
            return;
        }
        for (Object& o : mObjects)
        {
            if (o.mSource == Source::None)
            {
                continue;
            }
            DWORD v = Value(o, 0);
            if (emit && v != o.mLast)
            {
                Push(o.mFormat.dwOfs, v);
            }
            o.mLast = v;
        }
    }

    void Push(DWORD ofs, DWORD value)
    {
        DIDEVICEOBJECTDATA d;
        d.dwOfs = ofs;
        d.dwData = value;
        d.dwTimeStamp = GetTickCount();
        d.dwSequence = ++gSequence;
        if (mQueue.size() >= mBufferSize)
        {
            mQueue.pop_front(); // overflow drops the oldest, as DirectInput does
        }
        mQueue.push_back(d);
    }

    Kind mKind;
    GUID mGuid;
    int mPadIndex;
    ULONG mRefs;
    DWORD mStateSize;
    DWORD mBufferSize;
    bool mAcquired;
    std::vector<Object> mObjects;
    std::deque<DIDEVICEOBJECTDATA> mQueue;
};

class DirectInput final : public IDirectInputA
{
  public:
    DirectInput() : mRefs(1)
    {
    }

    HRESULT QueryInterface(REFIID riid, void** out) override
    {
        if (riid == IID_IDirectInputA)
        {
            AddRef();
            *out = this;
            return S_OK;
        }
        *out = 0;
        return E_NOINTERFACE;
    }
    ULONG AddRef() override
    {
        return ++mRefs;
    }
    ULONG Release() override
    {
        ULONG refs = --mRefs;
        if (!refs)
        {
            delete this;
        }
        return refs;
    }
    HRESULT CreateDevice(REFGUID guid, IDirectInputDeviceA** device, LPUNKNOWN) override
    {
        *device = 0;
        if (guid == GUID_SysKeyboard)
        {
            *device = new DirectInputDevice(Kind::Keyboard, guid, -1);
        }
        else if (guid == GUID_SysMouse)
        {
            *device = new DirectInputDevice(Kind::Mouse, guid, -1);
        }
        else
        {
            int pad = PadIndexFromGuid(guid);
            if (pad < 0 || pad >= plat::PadCount())
            {
                return DIERR_DEVICENOTREG;
            }
            *device = new DirectInputDevice(Kind::Pad, guid, pad);
        }
        return DI_OK;
    }
    HRESULT EnumDevices(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD) override
    {
        // No joysticks: the game's own pad support (one device per control, the stick as four
        // buttons) is not offered. The pad is read by platform/controls.cpp, next to the keyboard.
        return DI_OK;
    }
    HRESULT GetDeviceStatus(REFGUID guid) override
    {
        return (guid == GUID_SysKeyboard || guid == GUID_SysMouse || PadIndexFromGuid(guid) >= 0) ? DI_OK : DI_NOTATTACHED;
    }
    HRESULT RunControlPanel(HWND, DWORD) override
    {
        return DI_OK;
    }
    HRESULT Initialize(HINSTANCE, DWORD) override
    {
        return DI_OK;
    }

  private:
    ULONG mRefs;
};
} // namespace

void port_dinput_key_event(uint8_t dik, bool down)
{
    for (DirectInputDevice* keyboard : gKeyboards)
    {
        keyboard->OnKey(dik, down);
    }
}

extern "C" HRESULT DirectInputCreateA(HINSTANCE, DWORD, LPDIRECTINPUTA* out, LPUNKNOWN)
{
    *out = new DirectInput();
    return DI_OK;
}
