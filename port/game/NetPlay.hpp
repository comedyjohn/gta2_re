#pragma once

#include "core/function.hpp"
// Included this way as a hack so > msvc6 can use these headers too as they've been removed in later versions.
#include <VC98/Include/DPLAY.H>
#include <VC98/Include/DPLOBBY.H>
#include <windows.h>

struct naughty_sinoussi_0x800;
class Game_0x40;
class Network_20324;

#pragma pack(push, 1)
class PacketHeader
{
  public:
    s32 field_0_type;
    s32 field_4_sub_type;
};

class Packet_Ping_C
{
  public:
    DPID field_0_sender_player_id;
    DPID field_4_receiver_player_id;
    s32 field_8_time;
};

class Standard_Packet
{
  public:
    char_type field_0_std_packet_subtype;
    s32 field_1_unused;
};

class Packet_SubType_3
{
  public:
    PacketHeader header;
    u8 field_8_seq; // mod 256
    s32 field_9_bLoadData;
    void* field_D_data;
    s32 field_11_len;
};
#pragma pack(pop)

class Network_Player
{
  public:
    s32 field_0_in_use;
    s32 field_4_num_ping_samples;
    s32 field_8_average_ping;
    s32 field_C_ping_sum;
    DPID field_10_player_id;
    u32 field_14_dpname;
    s32 field_18_not_used;
    wchar_t* field_1C_player_name;
    s32 field_20_not_used; // not used
    void* field_24_pData;
    s32 field_28_data_len;
};

struct Network_Data
{
    u8 field_0;
    u8 field_1_tick;
    u8 field_2_unused;
    u8 field_3_type;
    u8 field_4_length;
    void* field_5_data;
};

class Network_Game_Session
{
  public:
    DPID field_0_group_id;
    u32 field_4_players_count;
    u8 field_8_packet_game_tick[MAX_PLAYERS];
    u16 field_E_not_used;
    Network_Player field_10_players[MAX_PLAYERS];
    u8* field_118_game_settings;
    s32 field_11C_group_data_len;
    DPSESSIONDESC2 field_120_session_desc;
};

class EnumeratedConnection
{
  public:
    GUID field_0_sp_guid;
    wchar_t* field_10_pConnectionName;
    u8* field_14_pConnection;
    s32 field_18_connection_len;
};

struct Network_NameList
{
    wchar_t* field_0_names[16];
    u32 field_40_count;
};

struct Network_ChatMessage
{
    wchar_t field_0_message[128];
    wchar_t field_100_name[16];
};

struct Network_PlayerPing
{
    wchar_t field_0_name[260];
    s32 field_208_avg_ping;
};

struct Network_4
{
    wchar_t* field_0_allocated_str;
};

struct Network_8
{
    void* field_0_data;
    s32 field_4_len;
};

struct Network_18
{
    EnumeratedConnection* field_0_enumed_connections;
    Network_4* field_4_d_array_8_entries;
    u32 field_8_connections_count;
    u32 field_C_f4_d_array_count;
    s32 field_10_enum_success;
    s32 field_14_dplay_success;
};

struct Network_504
{
    DPSESSIONDESC2 field_C4_sessions[16];
    u32 field_5C4_session_count;
};

struct Network_Buffer_8
{
    void* field_0;
    s32 field_4_len;
};

struct Network_InputData_0x8
{
    u32 field_0_Inputs;
    u32 field_4_rng;
};

struct Network_14
{
    Network_InputData_0x8 field_0_inputs;
    s32 field_8_id;
    s32 field_C_len;
    u8 field_10_used;
    char_type field_11_game_tick; // mod 256
    char field_12;
    char field_13_unused;
};

struct Network_PlayerInputs_0x30
{
    Network_InputData_0x8 field_0_inputs[MAX_PLAYERS];
};

struct PacketHandlerSlot
{
    void* field_0_param_fn_callback; // fn ptr
    void* field_4_param_context; // this ptr
    s32 field_8_fn_type;
};

struct NetPlay
{
    NetPlay();
    void* vdtor_51D7B0(char_type flags);
    virtual ~NetPlay();
    void AddEnumeratedConnection_51D930(EnumeratedConnection* pConnectionInfo);
    static s32 EnumConnections_cb_51DA30(const GUID* lpguidSP,
                                         void* lpConnection,
                                         DWORD dwConnectionSize,
                                         const DPNAME* lpName,
                                         DWORD dwFlags,
                                         void* lpContext);
    s32 SetProtoAndConnection_51DAE0(GUID* pProtocolGuid, Network_Buffer_8* pUseThisConnection);
    void DirectPlayDestroy_51DC90();
    s32 DirectPlayCreate_51DCD0();
    s32 DirectPlayCreate_51DED0();
    static BOOL PASCAL EnumAddress_cb_51E030(const GUID& guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext);
    s32 PushConnection_51E0E0(wchar_t* Source);
    s32 CreateTcpIpAddress_51E140(wchar_t* pIpAddress, sptr* ppAddress, size_t* pAddressLen);
    s32 CreateModemAddress_51E2B0(wchar_t* pPhoneNumber, wchar_t* pModemName, sptr* ppAddress, size_t* pAddressLen);
    s32 CreateSerialAddress_51E450(DPCOMPORTADDRESS* pComPort, uptr* ppAddress, size_t* pAddressLen);
    s32 InitializeConnection_51E5C0();
    s32 EnumSessions_51E650();
    s32 HostSession_51E7A0(wchar_t* pSessionName, wchar_t* pPlayerName, Network_8* pGroupData, Network_8* pPlayerData);
    u32 AddPlayer_51E9C0(Network_8* pData, DPID player_id, DPNAME name, Network_Game_Session* pStru);
    static BOOL EnumSessions_cb_51EAE0(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext);
    s32 AddEnumeratedSession_51EB00(DPSESSIONDESC2* pSession);
    void Set15_51ECD0(sptr pFunc, Network_20324* pParam);
    void NetworkTick_51ED00();
    s32 SendPing_51EF60();
    char_type Receive_51F010(sptr* pOutData, s32* pOutDataLen, DWORD* recvId, DWORD* senderId);
    void SendOrReceivePacket_51F0D0(void* pPacket, s32 packetLen, s32 recvId, s32 bSendOrRecv);
    void ProcessPingOrHandshakeSend_51F110(void* pPacket, s32 packetLen, s32 recvId, s32 sendFlag);
    s32 CalcPacketLen_51F210(sptr pPacket, u32 packetLen); // Also converts enums back
    void MakeSendData_51F420(Packet_SubType_3* pPacket, sptr* pData, u32* pDataLen); // Format data before using IDirectPlay3::Send
    void OnPacketReceived_51F870(void* pPacket, s32 packetLen, s32 recvId, s32 sendFlag);
    s32 MovePlayerToGroup_520040(DPID toFind, Network_Game_Session* pStru, Network_Game_Session* pDst, u32* pOutIdx);
    void FreePlayerSlot_5201A0(s32 idx, Network_Game_Session* pStru);
    void ProcessIncomingPacket_520230(s32 idx, uptr pUnknown);
    void Set6_520530(void* pFunc, void* pParam);
    s32 JoinSession_520570(u32 session_idx, wchar_t* pPlayerName, Network_8* pPlayerData, Network_8* pOutGroupData);
    static BOOL EnumGroups_cb_520C20(DPID group_id, DWORD callback_type, LPCDPNAME pName, DWORD flags, LPVOID pContextArg);
    s32 AddEnumeratedGroupPlayer_520CA0(DPID player_id, DPNAME* pName);
    void SetGroupId_520D00(DPID group_id);
    void Disconnect_520D10();
    void ClearPlayersAndSession_520DE0(Network_Game_Session* pStru);
    u32 IndexOf_520E30(DPID toFind, Network_Game_Session* pObj);
    void Set9_520E60(sptr pFunc, sptr pParam);
    void Set3_Disconnect_520E80(sptr pCallback, sptr pContext);
    void NoRefs_null_520EA0();
    void UpdatePlayerPing_520EB0(DPID player_id, s32 ping, Network_Game_Session* pStru);
    void Set18_520F50(sptr pCallback, sptr pContext);
    s32 RemovePlayerByName_520F80(wchar_t* String2);
    s32 DeletePlayerFromGroup_521000(u32 idx);
    s32 SendChatMessage_521060(wchar_t* pMsg, s32 idx_always_m1);
    void Set21_5210D0(sptr pCallback, sptr pContext);
    void GetPlayerName_521100(wchar_t* Destination, u32 idx);
    void Set24_521140(sptr pCallback, sptr pContext);
    s32 SetGameSettings_521170(Network_8* pObj);
    void Set27SavePlayerName_5211F0(sptr pCallback, sptr pContext);
    void DisableJoining_521220();
    s32 IsUserHost_5212E0();
    void SetExitGameCallBack_521330(sptr pFunc, Game_0x40* pGame);
    s32 GetMaxPlayers_521350();
    void Send_521370();
    bool WaitForPlayersSync_5213E0();
    s32 CopyConnection_5215B0(u32 idx, uptr* ppConnection, size_t* pLen);
    s32 SendToPlayer_521630(Network_8* pSendData, s32 player_idx, char_type delayed_ticks);
    void Add_5216E0(Network_8* pData, s32 id, char_type game_tick);
    s32 GetLatestPacket_521770(Network_8* pOut, char_type* pSeq, u32* pPlayerId);
    void sub_521820(s32** ppData, s32 idx);
    void Remove_521870(s32 idx);
    char_type ReceiveGameMessage_521890(Network_8* pOut, s32* pPlayerIdx, u32* packet_subtype);
    void SendToAll_521B20(Network_8* pSendData);
    s32 NoRefs_Send_521BE0(Network_8* pSendData, s32 tick_delay);
    s32 NoRefs_Send_521C80(sptr pSendData);
    s32 SendKeepAlive_521D20();
    s32 SendGiveUp_521DB0(s32 value);
    s32 SendSyncCheckData_521E40(sptr pSendData);
    static void static_dtor_5E4DD0();

    u32 GetPlayerCount_461DA0()
    {
        return field_758_n2.field_4_players_count;
    }

    s32 GetPlayerIdx_409C40()
    {
        return field_5D4_player_idx;
    }

    //s32 field_0_vtbl;
    char field_4_bModem;
    char field_5_modem_num;
    char field_6_unused;
    char field_7_unused;
    GUID field_8_ip_or_ipx_guid;
    s32 field_18_unused;
    s32 field_1C_unused;
    s32 field_20_unused;
    s32 field_24_unused;
    s32 field_28_unused;
    s32 field_2C_ptrs;
    Network_18 field_30_enumed_connections;
    s32 field_48_net_state;
    PacketHandlerSlot field_4C_func_ptrs_and_params[10];
    Network_504 field_C4_sessions;
    s32 field_5C8_not_used;
    s32 field_5CC_bIsHost;
    s32 field_5D0_player_og_idx; // Only set for the host, but it's not used
    s32 field_5D4_player_idx; // 1, 2, ..., MAX_PLAYERS
    DPID field_5D8_player_id; // From DPMSG_ADDPLAYERTOGROUP::dpIdPlayer
    HANDLE field_5DC_handle;
    IDirectPlayLobby2* field_5E0_pDPlayLobby2;
    IDirectPlay3* field_5E4_pDPlay3;
    Network_Game_Session field_5E8_n1;
    Network_Game_Session field_758_n2;
    naughty_sinoussi_0x800* field_8C8_not_used[MAX_PLAYERS];
    sptr field_8E0_pSendDataBuffer; // 0x1800 of alloc size
    sptr field_8E4_pReceivedDataBuffer; // 0x1800 of alloc size
    s32 field_8E8_time;
    s32 field_8EC_unused;
    char field_8F0_disabled; // Receive_51F010 and ReceiveGameMessage_521890 always returns false if this var is true
    char field_8F1_unused;
    char field_8F2_unused;
    char field_8F3_unused;
    s32 field_8F4_time_diff;
    Network_14 field_8F8_packets[48];
    s32 field_CB8_count;
};

EXTERN_GLOBAL(NetPlay, gNetPlay_7071E8);
