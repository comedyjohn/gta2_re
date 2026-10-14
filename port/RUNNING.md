# Running GTA2 natively on Linux

You need your own copy of GTA2: an installed game folder, or its installer (an Inno Setup
`setup.exe`, plus any `setup-1.bin`, `setup-2.bin`, ... files that come with it).

1. Install the 32-bit libraries the game needs (once). On Debian/Ubuntu:

   ```bash
   sudo dpkg --add-architecture i386 && sudo apt update
   sudo apt install libsdl2-2.0-0:i386 libgl1:i386 libgl1-mesa-dri:i386 innoextract
   ```

2. Put `gta2` and `gta2-launch` in one folder together with either the game's installer files or
   the contents of an installed GTA2 folder (the one with `data/` in it).

3. Run `./gta2-launch`. The first time, it unpacks the installer into `game/` (this takes a minute);
   after that it starts straight away.

Settings are kept in `~/.config/gta2-native/registry.txt`; delete it to go back to the defaults.
Alt+Enter switches between a window and fullscreen. Keyboards and game controllers work; the
controls are set in the game's own options menu.

The game plays its music from a `GTAUDIO` folder, which came on the original CD (the game looks for
it as `GTAUDIO` on every drive letter). Copy that folder into the game folder, next to `data/`, to
hear the radio; without it everything else still has sound.

## Playing together

Everyone needs the same build of `gta2` (the same version of the game; 32-bit, 64-bit and ARM
builds of one version play together) and the same game data.

- **On one network:** one player runs `./gta2-launch --host`, the others `./gta2-launch --join`.
  They find the game on their own.
- **Over the internet:** the host's UDP port 2302 has to be reachable: forward it on the host's
  router. The others run `./gta2-launch --join <the host's address>` (a name or an IP address).
- **Without opening a port at home:** run `gta2-server` (it comes with `gta2` and needs no game data)
  on any machine everyone can reach, such as a small rented server, with UDP port 2302 open. The host
  runs `./gta2-launch --host --server <its address>`, the others
  `./gta2-launch --join <its address>`. One server holds any number of games.

The host's game starts once two players are in, or as many as `--players N` asks for; pressing Enter
in the host's terminal starts it sooner. While waiting, whatever is typed in a terminal goes to the
others as chat. The host picks the game with `--map` (a number or part of a map's name), `--mode
frags|points|tag`, `--limit N`, `--time-limit MINUTES`, `--police on|off` and `--speed
slow|normal|fast`; the last choices are remembered. `--name` sets your player name and `--port` the
UDP port (the same for everyone). `./gta2-launch --help` lists them all. Started with `-n` alone, the
game asks on the terminal.

A player running a different version is turned away with a message saying so, and a player without
the host's map is told which one is missing.

## Not there yet

- The intro movies are skipped.
- No 3D sound: the options menu's 3D setting stays off, and sounds are panned in stereo as the game does
  without it.
- The network lobby is the command line and the terminal, not a window.
- The picture is the game's original 640x480, scaled to the window.

If the game stops with an error, run `./gta2-launch` from a terminal and keep what it prints.
