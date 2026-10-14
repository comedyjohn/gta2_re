# Native multiplayer

Players on any system the port builds for play together: Linux PCs at 32 or 64 bits and ARM
machines (handhelds, a Raspberry Pi), over a LAN or the internet. Every player needs the same
version of the game code (see "Which builds play together") and the same game data. Players say how
in [RUNNING.md](RUNNING.md); this is how it works.

## How the game does multiplayer

- **Lockstep:** every machine runs the whole simulation and only inputs are exchanged
  (`Packet_SubType_3`, 4 bytes a frame per player, sequence numbers mod 256). Each frame waits for
  every other player's input (`Net_Receive_Players_Inputs_4DA9F0`). At the start, every machine
  checks the others have the same map, script, `nyc.gci` and language (`WaitForPlayersSync_5213E0`,
  `MakeInitialSyncData_4DB2E0`).
- **Transport:** DirectPlay (`NetPlay.cpp`). The calls it makes: `EnumConnections`,
  `InitializeConnection`, `EnumSessions`, `Open`, `CreatePlayer`, `CreateGroup`, `AddPlayerToGroup`,
  `EnumGroups`, `EnumGroupPlayers`, `Get/SetGroupData`, `GetPlayerData`, `Send`, `Receive`,
  `SendChatMessage`, `SetSessionDesc`, `DeletePlayerFromGroup`, `DestroyPlayer`, `DestroyGroup`,
  `Close`, and the lobby's `CreateCompoundAddress`/`EnumAddress`. The system messages it handles:
  `DPSYS_CREATEPLAYERORGROUP`, `DESTROYPLAYERORGROUP`, `ADDPLAYERTOGROUP`, `DELETEPLAYERFROMGROUP`,
  `SETPLAYERORGROUPDATA`, `CHAT`, `SESSIONLOST`.
- **Lobby:** `Network_20324` runs dialog 103 from the original executable: host or join, the map and
  rules (sent to the others as the group's data), the player list, chat, and Start, which sends
  "disable joining" to everyone and closes the dialog on every machine.

## What the port does

| Piece | Where |
|---|---|
| DirectPlay (`IDirectPlay3`, `IDirectPlayLobby2`) | `platform/dplay.cpp`, over `platform/net/` |
| The session protocol: reliable UDP, sessions, relaying | `platform/net/` (plain C++ and POSIX sockets, no game or SDL types) |
| A session server on its own | `gta2-server` (`platform/net/server_main.cpp`) |
| The lobby, without the dialog | `platform/lobby.cpp`: `DialogBoxParamA(103)` lands there |
| Command line | `--host`, `--join`, `--server`, ... (`gta2 --help`, `platform/lobby.cpp`) |
| Tests without game data | `gta2 --nettest`, `gta2 --simcheck`, `tools/nettest.sh` |

**A star around a session server.** The machine that hosts runs a server inside the game, on UDP
port 2302; everyone, the host included, connects to it, and it relays every message. `gta2-server`
is the same server as a program of its own, for a machine everyone can reach: then only that machine
needs a reachable port, and it can hold any number of games at once. A player's own copy of the
session (players, the group, its data) is kept by `net::Client` and updated by the server's messages,
in the order the server sent them, so DirectPlay's synchronous calls (`EnumGroupPlayers`,
`GetGroupData`, ...) answer from it at once.

**Every message is delivered, in order.** Each client-server link (`net::Link`) numbers its
datagrams, acknowledges with a cumulative ack plus a 64-bit selective ack, and resends what isn't
acknowledged: at once when a later datagram's ack shows a gap, otherwise after a timeout from the
measured round trip (at least 10 ms; lockstep waits for every input, so a short timeout matters more
than a few spare bytes). The game's unguaranteed sends are delivered reliably too: a lost input
would otherwise stall every player for the game's own one-second resend. Measured on one machine,
1000 frames of 4 players took 4.0 s with nothing lost, 6.7 s with 2% of datagrams lost, 10 s at 5%,
35 s at 25%.

**DirectPlay's rules, as the game relies on them.** A player's own `AddPlayerToGroup` comes back to
it as `DPSYS_ADDPLAYERTOGROUP` (that is how the host's own player reaches `field_758_n2`), but its own
`CreatePlayer` doesn't. System messages only go to machines with a player. Messages to the group or to
everyone never go back to their sender. A player created while ours was being created, and only then
added to the group, is announced to the game before its adding (`Client::Queue`). A new session is
only listed once its group has a player in it, so nobody joins before there is a group to join. When
the host leaves, the session ends for everyone (`DPSYS_SESSIONLOST`), as DirectPlay without host
migration does. When a player's machine goes silent for 15 s, the server removes its player, and a
client whose server goes silent for 15 s loses the session; the network threads keep the link alive
while a game is busy loading.

**Connections.** `EnumConnections` offers one connection under DirectPlay's TCP/IP and IPX names (the
game picks IPX when no protocol is set in the registry). An address in the connection
(`DPAID_INet`/`INetW`, as the original GTA2 Manager wrote it) is where to look for games; without one,
the LAN is asked by broadcast. Host names work; IPv6 addresses (`[2001:db8::1]:2302`) are taken and
the server listens on IPv6 too, but that hasn't been tried: the machine it was written on has no
IPv6.

**The lobby** (`lobby.cpp`) does what the dialog's buttons do, through the same `Network_20324` and
`NetPlay` calls: the saved settings (`LoadGameSettingsFromRegistry_51C830`) changed by the command
line, `HostSession_51E7A0`, `JoinSession_520570` and `ApplyHostGameSettings_51B810`, and Start
(`DisableJoining_521220`). It prints the players and chat, and takes chat (and Enter, for the host)
from the terminal. A joining player who doesn't have the host's map is told so. Without `--host` or
`--join`, `gta2 -n` asks on the terminal.

## Which builds play together

Lockstep only works if every machine computes exactly the same thing. The port makes that true across
systems, and refuses players it can't vouch for:

- **The game code must be the same.** `tools/game_hash.cmake` hashes `Source/`, the port's headers
  and its C runtime into `PortNetCompat()`. A session carries its host's hash and the server turns
  away anyone else with "the game was started by a different version of GTA2 ... the host has linux
  x86-64, game 1a2b3c4d". The game's own start-up check still compares the data files.
- **rand() is the same everywhere.** The simulation draws from the C library's `rand()`
  (`rng.cpp`). glibc's generator isn't musl's, and neither is the original's, so the port uses MSVC's
  (`port_rand` in `platform/crt.cpp`): the same numbers on every system, and the ones the original
  game drew.
- **Floating point is the same everywhere.** The game's maths is fixed-point (`Fix16`); floating point
  only fills the sine, cosine and tangent tables and takes square roots. Every table entry is at least
  1e-10 (relative) away from a rounding edge, so any libm gives the same tables, and IEEE square roots
  are exact. 32-bit x86 builds use SSE2 instead of x87 and no build fuses multiply-adds (GCC does by
  default on ARM), so nothing changes that later.
- **Checked.** `gta2 --simcheck` hashes the tables, 100000 draws of `rand()`, square roots and
  arctangents over their range. x86, x86-64 and ARM64 (under qemu) print the same hashes, and
  `tools/nettest.sh` compares two builds' output.

Unchecked: code that reads uninitialised memory or depends on pointer values could still differ
between 32 and 64 bits. The game's own sync check (`do_sync_check` in the registry's debug settings)
compares a random number every frame and stops with "SyncErrorRandom" when two machines drift apart.

## Tests

`tools/nettest.sh BUILD [OTHER_BUILD]` runs `gta2 --nettest` processes against each other through
the port's DirectPlay, the way NetPlay calls it, and needs no game data:

- two players, finding each other on the LAN;
- four players with 25% of all datagrams dropped (`GTA2_NET_DROP`): every frame's message from every
  player arrives, once, in order, with the right contents;
- a player leaving during the game; the host quitting (the others' session ends at once); the host's
  machine dying (the session ends after the timeout, `GTA2_NET_TIMEOUT_MS`);
- a different build turned away with the reason (`GTA2_NET_TEST_COMPAT`);
- three players through a separate `gta2-server`;
- with a second build, players mixed across the two and their `--simcheck` compared.

CI runs it for the 32-bit and the 64-bit build, and once with the two mixed. By hand it has passed
x86 against x86-64, x86 against ARM64 and x86-64 against ARM64 (the ARM build cross-compiled with
`cmake/aarch64-linux-gnu.cmake` and run under qemu-user).

The lobby has been run between two copies of the game with stand-in data files (an empty `e.gxt`, a
`.mmp` naming empty map files): host and join, the player list on both sides, Start on both. The
games then stop loading the stand-in map. A real match needs game data, which hasn't been tried yet.

## Limits and what's left

- **Native builds only.** Original Windows GTA2 can't join: DirectPlay's wire protocol is a different
  one, and the original's x87 maths would have to match the port's.
- **Reachability.** The host's UDP port (2302) has to be reachable by the others: on a LAN it is; over
  the internet, forward it on the host's router, or run `gta2-server` on a machine with a public
  address. Getting through routers without any of that (NAT hole punching) isn't built.
- **A graphical lobby.** The lobby is the command line and the terminal. The original dialog could
  come back by emulating its Win32 controls (list view, button, edit, combo box, list box, trackbar)
  in `win32.cpp` and drawing them with Dear ImGui, so `Network_20324`'s own code runs unchanged; that
  needs ImGui's OpenGL backend, which the debug overlay also waits for.
- **A real match** with game data, first two copies on one PC, then two machines, then mixed systems.
- `Network_20324`'s dialog procedures (`OnWmCommand_519FE0`, `OnTimer_51A9D0`, `subclass_proc_51BDD0`,
  `CreateMainUi_51AA90`) still keep pointers in `LONG`s (`GetWindowLongA`); at 64 bits they would
  truncate them, but the port's lobby never calls them.
