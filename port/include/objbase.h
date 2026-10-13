// COM interface declaration macros (the C++ form), enough for ddraw/dinput/dplay headers.
#pragma once
#include <windows.h>

#define interface struct
#define STDMETHODCALLTYPE
#define STDMETHODVCALLTYPE
#define STDAPI extern "C" HRESULT
#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define PURE = 0
#define THIS_
#define THIS void
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, baseiface) struct iface : public baseiface
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern const GUID name
#define CLSCTX_INPROC_SERVER 0x1
#define CLSCTX_ALL 0x17
#define IsEqualGUID(a, b) (memcmp(&(a), &(b), sizeof(GUID)) == 0)
#define IsEqualIID(a, b) IsEqualGUID(a, b)

struct IUnknown
{
    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
};
typedef IUnknown* LPUNKNOWN;

extern "C" HRESULT CoInitialize(LPVOID reserved);
extern "C" void CoUninitialize();
extern "C" HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN outer, DWORD ctx, REFIID riid, LPVOID* ppv);
