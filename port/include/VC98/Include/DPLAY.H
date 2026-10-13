// DirectPlay 6 subset for the native build, at the path NetPlay.hpp includes it from
// (<VC98/Include/DPLAY.H>). Structure layouts follow the SDK. Multiplayer is stubbed for now:
// DirectPlayCreate fails, so the game reports no network connections.
#pragma once
#include <windows.h>
#include <objbase.h>

typedef DWORD DPID;
typedef DPID* LPDPID;

#define _FACDP 0x877
#define MAKE_DPHRESULT(code) MAKE_HRESULT(1, _FACDP, code)
#define DP_OK S_OK
#define DPERR_GENERIC E_FAIL
#define DPERR_BUFFERTOOSMALL MAKE_DPHRESULT(30)
#define DPERR_NOMESSAGES MAKE_DPHRESULT(190)
#define DPERR_UNAVAILABLE MAKE_DPHRESULT(250)
#define DPERR_USERCANCEL MAKE_DPHRESULT(280)
#define DPERR_CONNECTING MAKE_DPHRESULT(350)

#define DPID_SYSMSG 0
#define DPID_ALLPLAYERS 0
#define DPENUMSESSIONS_AVAILABLE 0x00000001
#define DPENUMSESSIONS_ALL 0x00000002
#define DPENUMSESSIONS_ASYNC 0x00000010
#define DPENUMSESSIONS_RETURNSTATUS 0x00000080
#define DPENUMPLAYERS_ALL 0x00000000
#define DPENUMGROUPS_ALL DPENUMPLAYERS_ALL
#define DPESC_TIMEDOUT 0x00000001
#define DPOPEN_JOIN 0x00000001
#define DPOPEN_CREATE 0x00000002
#define DPOPEN_RETURNSTATUS DPENUMSESSIONS_RETURNSTATUS
#define DPPLAYERTYPE_GROUP 0x00000000
#define DPPLAYERTYPE_PLAYER 0x00000001
#define DPRECEIVE_ALL 0x00000001
#define DPSEND_GUARANTEED 0x00000001
#define DPSESSION_JOINDISABLED 0x00000020
#define DPCONNECTION_DIRECTPLAY 0x00000001
#define DPSYS_CREATEPLAYERORGROUP 0x0003
#define DPSYS_DESTROYPLAYERORGROUP 0x0005
#define DPSYS_ADDPLAYERTOGROUP 0x0007
#define DPSYS_DELETEPLAYERFROMGROUP 0x0021
#define DPSYS_SESSIONLOST 0x0031
#define DPSYS_SETPLAYERORGROUPDATA 0x0102
#define DPSYS_CHAT 0x0109

typedef struct
{
    DWORD dwSize;
    DWORD dwFlags;
    GUID guidInstance;
    GUID guidApplication;
    DWORD dwMaxPlayers;
    DWORD dwCurrentPlayers;
    union
    {
        LPWSTR lpszSessionName;
        LPSTR lpszSessionNameA;
    };
    union
    {
        LPWSTR lpszPassword;
        LPSTR lpszPasswordA;
    };
    DWORD dwReserved1;
    DWORD dwReserved2;
    DWORD dwUser1;
    DWORD dwUser2;
    DWORD dwUser3;
    DWORD dwUser4;
} DPSESSIONDESC2, *LPDPSESSIONDESC2;
typedef const DPSESSIONDESC2* LPCDPSESSIONDESC2;

typedef struct
{
    DWORD dwSize;
    DWORD dwFlags;
    union
    {
        LPWSTR lpszShortName;
        LPSTR lpszShortNameA;
    };
    union
    {
        LPWSTR lpszLongName;
        LPSTR lpszLongNameA;
    };
} DPNAME, *LPDPNAME;
typedef const DPNAME* LPCDPNAME;

typedef struct
{
    DWORD dwSize;
    DWORD dwFlags;
    union
    {
        LPWSTR lpszMessage;
        LPSTR lpszMessageA;
    };
} DPCHAT, *LPDPCHAT;

typedef struct
{
    DWORD dwType;
} DPMSG_GENERIC, *LPDPMSG_GENERIC;

typedef struct
{
    DWORD dwType;
    DWORD dwPlayerType;
    DPID dpId;
    DWORD dwCurrentPlayers;
    LPVOID lpData;
    DWORD dwDataSize;
    DPNAME dpnName;
    DPID dpIdParent;
    DWORD dwFlags;
} DPMSG_CREATEPLAYERORGROUP, *LPDPMSG_CREATEPLAYERORGROUP;

typedef struct
{
    DWORD dwType;
    DWORD dwPlayerType;
    DPID dpId;
    LPVOID lpLocalData;
    DWORD dwLocalDataSize;
    LPVOID lpRemoteData;
    DWORD dwRemoteDataSize;
    DPNAME dpnName;
    DPID dpIdParent;
    DWORD dwFlags;
} DPMSG_DESTROYPLAYERORGROUP, *LPDPMSG_DESTROYPLAYERORGROUP;

typedef struct
{
    DWORD dwType;
    DPID dpIdGroup;
    DPID dpIdPlayer;
} DPMSG_ADDPLAYERTOGROUP, *LPDPMSG_ADDPLAYERTOGROUP;
typedef DPMSG_ADDPLAYERTOGROUP DPMSG_DELETEPLAYERFROMGROUP;
typedef DPMSG_DELETEPLAYERFROMGROUP* LPDPMSG_DELETEPLAYERFROMGROUP;

typedef struct
{
    DWORD dwType;
    DWORD dwPlayerType;
    DPID dpId;
    LPVOID lpData;
    DWORD dwDataSize;
} DPMSG_SETPLAYERORGROUPDATA, *LPDPMSG_SETPLAYERORGROUPDATA;

typedef struct
{
    DWORD dwType;
    DWORD dwFlags;
    DPID idFromPlayer;
    DPID idToPlayer;
    DPID idToGroup;
    LPDPCHAT lpChat;
} DPMSG_CHAT, *LPDPMSG_CHAT;

typedef BOOL (*LPDPENUMSESSIONSCALLBACK2)(LPCDPSESSIONDESC2 desc, LPDWORD timeOut, DWORD flags, LPVOID context);
typedef BOOL (*LPDPENUMPLAYERSCALLBACK2)(DPID id, DWORD playerType, LPCDPNAME name, DWORD flags, LPVOID context);
typedef BOOL (*LPDPENUMCONNECTIONSCALLBACK)(LPCGUID sp, LPVOID connection, DWORD connectionSize, LPCDPNAME name,
                                            DWORD flags, LPVOID context);

struct IDirectPlay : public IUnknown
{
};

struct IDirectPlay2 : public IUnknown
{
    virtual HRESULT AddPlayerToGroup(DPID group, DPID player) = 0;
    virtual HRESULT Close() = 0;
    virtual HRESULT CreateGroup(LPDPID group, LPDPNAME name, LPVOID data, DWORD size, DWORD flags) = 0;
    virtual HRESULT CreatePlayer(LPDPID player, LPDPNAME name, HANDLE event, LPVOID data, DWORD size, DWORD flags) = 0;
    virtual HRESULT DeletePlayerFromGroup(DPID group, DPID player) = 0;
    virtual HRESULT DestroyGroup(DPID group) = 0;
    virtual HRESULT DestroyPlayer(DPID player) = 0;
    virtual HRESULT EnumGroupPlayers(DPID group, LPGUID instance, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT EnumGroups(LPGUID instance, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT EnumPlayers(LPGUID instance, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT EnumSessions(LPDPSESSIONDESC2 desc, DWORD timeout, LPDPENUMSESSIONSCALLBACK2 cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT GetCaps(LPVOID caps, DWORD flags) = 0;
    virtual HRESULT GetGroupData(DPID group, LPVOID data, LPDWORD size, DWORD flags) = 0;
    virtual HRESULT GetGroupName(DPID group, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT GetMessageCount(DPID player, LPDWORD count) = 0;
    virtual HRESULT GetPlayerAddress(DPID player, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT GetPlayerCaps(DPID player, LPVOID caps, DWORD flags) = 0;
    virtual HRESULT GetPlayerData(DPID player, LPVOID data, LPDWORD size, DWORD flags) = 0;
    virtual HRESULT GetPlayerName(DPID player, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT GetSessionDesc(LPVOID data, LPDWORD size) = 0;
    virtual HRESULT Initialize(LPGUID guid) = 0;
    virtual HRESULT Open(LPDPSESSIONDESC2 desc, DWORD flags) = 0;
    virtual HRESULT Receive(LPDPID from, LPDPID to, DWORD flags, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT Send(DPID from, DPID to, DWORD flags, LPVOID data, DWORD size) = 0;
    virtual HRESULT SetGroupData(DPID group, LPVOID data, DWORD size, DWORD flags) = 0;
    virtual HRESULT SetGroupName(DPID group, LPDPNAME name, DWORD flags) = 0;
    virtual HRESULT SetPlayerData(DPID player, LPVOID data, DWORD size, DWORD flags) = 0;
    virtual HRESULT SetPlayerName(DPID player, LPDPNAME name, DWORD flags) = 0;
    virtual HRESULT SetSessionDesc(LPDPSESSIONDESC2 desc, DWORD flags) = 0;
};

struct IDirectPlay3 : public IDirectPlay2
{
    virtual HRESULT AddGroupToGroup(DPID parent, DPID group) = 0;
    virtual HRESULT CreateGroupInGroup(DPID parent, LPDPID group, LPDPNAME name, LPVOID data, DWORD size, DWORD flags) = 0;
    virtual HRESULT DeleteGroupFromGroup(DPID parent, DPID group) = 0;
    virtual HRESULT EnumConnections(LPCGUID app, LPDPENUMCONNECTIONSCALLBACK cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT EnumGroupsInGroup(DPID group, LPGUID instance, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD flags) = 0;
    virtual HRESULT GetGroupConnectionSettings(DWORD flags, DPID group, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT InitializeConnection(LPVOID connection, DWORD flags) = 0;
    virtual HRESULT SecureOpen(LPCDPSESSIONDESC2 desc, DWORD flags, LPVOID security, LPVOID credentials) = 0;
    virtual HRESULT SendChatMessage(DPID from, DPID to, DWORD flags, LPDPCHAT chat) = 0;
    virtual HRESULT SetGroupConnectionSettings(DWORD flags, DPID group, LPVOID connection) = 0;
    virtual HRESULT StartSession(DWORD flags, DPID group) = 0;
    virtual HRESULT GetGroupFlags(DPID group, LPDWORD flags) = 0;
    virtual HRESULT GetGroupParent(DPID group, LPDPID parent) = 0;
    virtual HRESULT GetPlayerAccount(DPID player, DWORD flags, LPVOID data, LPDWORD size) = 0;
    virtual HRESULT GetPlayerFlags(DPID player, LPDWORD flags) = 0;
};

typedef IDirectPlay* LPDIRECTPLAY;
typedef IDirectPlay2* LPDIRECTPLAY2;
typedef IDirectPlay3* LPDIRECTPLAY3;

extern "C" {
HRESULT DirectPlayCreate(LPGUID sp, LPDIRECTPLAY* out, IUnknown* outer);
extern const GUID CLSID_DirectPlay;
extern const GUID IID_IDirectPlay2;
extern const GUID IID_IDirectPlay3;
extern const GUID DPSPGUID_IPX;
extern const GUID DPSPGUID_TCPIP;
extern const GUID DPSPGUID_SERIAL;
extern const GUID DPSPGUID_MODEM;
}
