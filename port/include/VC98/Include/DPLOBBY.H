// DirectPlayLobby subset for the native build (see DPLAY.H).
#pragma once
#include "DPLAY.H"

typedef struct _DPCOMPORTADDRESS
{
    DWORD dwComPort;
    DWORD dwBaudRate;
    DWORD dwStopBits;
    DWORD dwParity;
    DWORD dwFlowControl;
} DPCOMPORTADDRESS, *LPDPCOMPORTADDRESS;

typedef struct DPCOMPOUNDADDRESSELEMENT
{
    GUID guidDataType;
    DWORD dwDataSize;
    LPVOID lpData;
} DPCOMPOUNDADDRESSELEMENT, *LPDPCOMPOUNDADDRESSELEMENT;
typedef const DPCOMPOUNDADDRESSELEMENT* LPCDPCOMPOUNDADDRESSELEMENT;

typedef BOOL (*LPDPENUMADDRESSCALLBACK)(REFGUID dataType, DWORD size, LPCVOID data, LPVOID context);

struct IDirectPlayLobby : public IUnknown
{
    virtual HRESULT Connect(DWORD flags, LPDIRECTPLAY2* out, IUnknown* outer) = 0;
    virtual HRESULT CreateAddress(REFGUID sp, REFGUID dataType, LPCVOID data, DWORD size, LPVOID address, LPDWORD addressSize) = 0;
    virtual HRESULT EnumAddress(LPDPENUMADDRESSCALLBACK cb, LPCVOID address, DWORD size, LPVOID ctx) = 0;
    virtual HRESULT EnumAddressTypes(LPVOID cb, REFGUID sp, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT EnumLocalApplications(LPVOID cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT GetConnectionSettings(DWORD appId, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT ReceiveLobbyMessage(DWORD flags, DWORD appId, LPDWORD msgFlags, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT RunApplication(DWORD flags, LPDWORD appId, LPVOID connection, HANDLE event) = 0;
    virtual HRESULT SendLobbyMessage(DWORD flags, DWORD appId, LPVOID data, DWORD size) = 0;
    virtual HRESULT SetConnectionSettings(DWORD flags, DWORD appId, LPVOID connection) = 0;
    virtual HRESULT SetLobbyMessageEvent(DWORD flags, DWORD appId, HANDLE event) = 0;
};

struct IDirectPlayLobby2 : public IDirectPlayLobby
{
    virtual HRESULT CreateCompoundAddress(LPCDPCOMPOUNDADDRESSELEMENT elements, DWORD count, LPVOID address, LPDWORD size) = 0;
};

typedef IDirectPlayLobby* LPDIRECTPLAYLOBBY;
typedef IDirectPlayLobby2* LPDIRECTPLAYLOBBY2;

extern "C" {
HRESULT DirectPlayLobbyCreateW(LPGUID sp, LPDIRECTPLAYLOBBY* out, IUnknown* outer, LPVOID data, DWORD size);
HRESULT DirectPlayLobbyCreateA(LPGUID sp, LPDIRECTPLAYLOBBY* out, IUnknown* outer, LPVOID data, DWORD size);
extern const GUID IID_IDirectPlayLobby;
extern const GUID IID_IDirectPlayLobby2;
extern const GUID DPAID_TotalSize;
extern const GUID DPAID_ServiceProvider;
extern const GUID DPAID_LobbyProvider;
extern const GUID DPAID_Phone;
extern const GUID DPAID_PhoneW;
extern const GUID DPAID_Modem;
extern const GUID DPAID_ModemW;
extern const GUID DPAID_INet;
extern const GUID DPAID_INetW;
extern const GUID DPAID_ComPort;
}
