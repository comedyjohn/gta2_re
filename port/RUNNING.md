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

Settings are kept in `~/.config/gta2-native/registry.txt` and the pad's layouts in
`~/.config/gta2-native/pad.txt`; delete them to go back to the defaults. Alt+Enter switches between a
window and fullscreen.

The keyboard and a game controller work at the same time: every control has a key and a pad input,
and either one works. The keys are the game's own (set in its options menu or in the overlay); the pad
has a layout for walking and one for driving. In the menus the pad works as the arrow keys (stick or
d-pad), Enter (A or Start) and Esc (B or Back).

The pad's defaults:

| | On foot | In a car |
|---|---|---|
| Left stick, d-pad | walk the way it points, as fast as it is pushed | steer |
| RT / LT | special 3 / special 2 | gas / brake and reverse, analog |
| X | fire (punch) | special 2 |
| Y | enter a car | leave the car |
| A | jump | handbrake |
| B | special 1 | horn |
| RB / LB | next / previous weapon | fire / previous weapon |
| Right stick | | turns a tank's, fire truck's or gun jeep's turret |

The port's overlay:

- **Esc**, or **Start** on the pad, pauses the game and opens a menu: **Settings** (the controls, see
  below) and **Quit game** (back to the main menu, as answering yes to the game's own quit question).
  Esc, B or Start goes back to the game. Not in a network game, where Esc asks the game's question as
  before. To get the original Esc back everywhere, switch the menu off in the settings (**E**, or Back
  on the pad) or put `esc_menu = 0` in `pad.txt`.
- **F1** shows or hides a legend of the controls (shown by default when a controller is connected).
- **F2**, the controller's **Guide / Home** button, or Settings in the Esc menu opens the settings:
  pick a control with Up/Down (or the d-pad), press Enter (or A) and then the key or pad button to
  bind: a key sets the control's key, a pad button its pad input on the page shown. The pages are
  **On foot** and **In a car** (LB/RB or Left/Right); keys are the same on both. Delete (or X) resets
  the control, **N** (or L3) takes its pad input away, **P** (or Y) puts both pad layouts back to the
  defaults, **K** the keys, **M** (or R3) switches the stick between walking the way it points and the
  game's own turn-left/turn-right controls, **E** (or Back) the Esc menu on and off. A button still held
  when the settings close does nothing until it is let go.
- In a single player game the game stands still, sound included, while any of these menus is open. Pad buttons are named by position,
  as on an Xbox pad: A bottom, B right, X left, Y top (8BitDo pads in Switch mode label them
  differently).
- **F5**, or **Back** on the pad (Start too when the Esc menu is off), opens a list of cars: Up/Down (or the stick) to choose, LB/RB to page,
  Enter or A to spawn it next to you (on open ground, in front of you if there is room), B, Back or
  Esc to close. Not in a network game.
- Getting into or out of a car switches the pad layout. A button held at that moment does nothing until
  it is let go, so the Y that got you in does not get you straight out again.
- Everything the pad does, the stick's direction and speed, the pedals, the turret and a spawned car
  included, is in a recorded replay (`gta2 --record`) and plays back exactly.

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
