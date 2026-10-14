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

## Not there yet

- The intro movies are skipped.
- No 3D sound: the options menu's 3D setting stays off, and sounds are panned in stereo as the game does
  without it.
- No multiplayer.
- The picture is the game's original 640x480, scaled to the window.

If the game stops with an error, run `./gta2-launch` from a terminal and keep what it prints.
