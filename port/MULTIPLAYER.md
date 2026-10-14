# Native multiplayer: the plan

Not started. This is what implementing it takes, worked out from `Source/NetPlay.cpp` and
`Source/Network_20324.cpp`. Update it as parts land, and tick the item in `TODO.md` when it's done.

## How the game does multiplayer

- **Lobby:** `Network_20324::Start_*` opens dialog resource 103 from the original exe with
  `DialogBoxParamA` (`Network_20324::DialogFunc`). The dialog creates its own child controls from a
  table in the code (`data[page][n]`: position, style, `hMenu` id, class name such as
  `SysListView32`, button, edit, combo box, trackbar) and drives them with `SendDlgItemMessageA`,
  `LVM_*`, `CB_*`, `LB_*`, `BM_*` and `TBM_*`. `NetPlay` reports back through callbacks registered with
  `Set15_51ECD0` and its siblings: fill the session list, add a player, chat, ping, disconnect, save
  the player name.
- **Transport:** DirectPlay only. The calls the game makes (count of call sites):
  `Send` 11, `CreateCompoundAddress` 6, `EnumSessions` 5, `DeletePlayerFromGroup` 3, `AddPlayerToGroup`,
  `Close`, `CreatePlayer`, `DestroyGroup`, `DestroyPlayer`, `GetGroupData`, `GetPlayerAddress`,
  `GetPlayerData`, `Open`, `Receive`, `QueryInterface` (2 each), `CreateGroup`, `EnumAddress`,
  `EnumConnections`, `EnumGroupPlayers`, `EnumGroups`, `InitializeConnection`, `SendChatMessage`,
  `SetGroupData`, `SetSessionDesc` (1 each), plus `Release`.
- **System messages it handles:** `DPSYS_CREATEPLAYERORGROUP`, `DPSYS_DESTROYPLAYERORGROUP`,
  `DPSYS_ADDPLAYERTOGROUP`, `DPSYS_DELETEPLAYERFROMGROUP`, `DPSYS_SETPLAYERORGROUPDATA`,
  `DPSYS_CHAT`, `DPSYS_SESSIONLOST`.
- **Gameplay is lockstep:** every machine runs the whole simulation and only inputs are exchanged
  (`Packet_SubType_3`, sequence numbers mod 256), plus sync checks (`WaitForPlayersSync_5213E0`,
  `MakeInitialSyncData_4DB2E0`, `SyncCheckData_*`). Packets go to the players' group, some with
  `DPSEND_GUARANTEED` and some without.
- In the port today: `DirectPlayCreate` and `DirectPlayLobbyCreateW` fail (`platform/directx.cpp`),
  so the game finds no connections, and the dialog functions are stubs (`platform/win32.cpp`).

## What to build

1. **A DirectPlay replacement over UDP** (`port/platform/dplay.cpp`; the interfaces are already
   declared in `include/VC98/Include/DPLAY.H` and `DPLOBBY.H`). The biggest part.
   - The host owns the session and relays packets (a star, simpler than DirectPlay's peer-to-peer).
   - `EnumConnections` offers one connection, "TCP/IP"; `CreateCompoundAddress`/`EnumAddress`/
     `InitializeConnection` carry the host's address.
   - `EnumSessions`: LAN broadcast plus a direct query to a typed IP address, honouring
     `DPENUMSESSIONS_ASYNC`/`_RETURNSTATUS`; `Open` to create or join, keeping `DPSESSIONDESC2`
     (max and current players, the `dwUser*` fields, `DPSESSION_JOINDISABLED`); `SetSessionDesc`.
   - Players with names and data, the one group, group data, `GetPlayerAddress`.
   - `Send` to a player or the group. Guaranteed packets get sequence numbers, acks, resends and
     in-order delivery per sender; unguaranteed ones are plain datagrams.
   - `Receive` returns queued packets and the system messages above; `SendChatMessage`; a timeout
     that reports `DPSYS_SESSIONLOST`; `Close`.
   - Plain POSIX sockets: the same code serves Linux PCs and the ARM handhelds.
2. **The lobby.** The dialog's template is in the original exe, which the port doesn't have.
   - (a) First: no dialog. `gta2 --host` / `--join <ip> --name <name>` (and `gta2-launch` passing them
     on); a port-side function performs the steps the dialog would (choose the connection, find or
     create the session, create the player, start when everyone is in) through the same `NetPlay`
     calls and callbacks.
   - (b) Later, if wanted: the original lobby. Emulate the few Win32 controls the game creates
     (list view, button, edit, combo box, list box, trackbar) in `win32.cpp` and draw them with Dear
     ImGui, so `Network_20324`'s own logic runs unchanged. Needs ImGui's OpenGL backend, which the
     debug overlay (`TODO.md`) also waits for.
3. **64-bit:** `NetPlay.cpp` and `Network_20324.cpp` still pass callbacks and pointers through
   `int`/`s32` in about 80 places (the casts GCC flags at 64 bits; see `TODO.md`). Make them `sptr`, as
   the rest of the game was done, checking each with `tools/check_matching.sh` ("changed: 0").
4. **Same build on every machine:** lockstep only holds if every machine computes the same thing.
   Different builds, 32 against 64-bit, or x86 against ARM can desync through floating-point
   differences. Put a build and architecture id in the session description and refuse a join that
   doesn't match; the game's own sync check is the backstop. Whether a PC and a handheld can play
   together has to be tested before it's allowed.
5. **Tests.**
   - Automated, no game data, run in CI: two processes on one machine through the replacement:
     find the session, join, guaranteed delivery and order under induced loss, a player leaving,
     session lost.
   - Then real matches with game data: two instances on one PC, then two machines on a LAN.

## Limits

- Native builds only. Original Windows GTA2 can't join: DirectPlay's wire protocol isn't
  documented.
- LAN, or the internet with the host's port forwarded. Getting through home routers without that
  (NAT traversal, a relay server) is a separate project.

## Order

1. Single player confirmed with real game data (a multiplayer bug is far easier to find on top of a
   working single player).
2. Part 1 with its automated test (can be done without game data).
3. Lobby (a), then a first two-instance match.
4. Parts 3 and 4; lobby (b) if wanted.
