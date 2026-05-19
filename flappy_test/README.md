# Flappy Test

A complete Flappy Bird-style Nintendo DS homebrew game built with BlocksDS.

The full game runs on the top screen. The bottom touch screen is used as the
flap button and also shows score, best score for the current session, and
controls.

## Controls

- Touch the lower screen: flap / start / restart
- A or D-Pad Up: flap / start / restart
- START: pause or resume while playing
- SELECT: return to the title screen after a game over

## Architecture

- `source/main.c`: Nintendo DS setup, framebuffer/console creation, and the
  frame loop.
- `source/flappy_game.*`: game state, input rules, physics, scoring, collision,
  and pipe recycling.
- `source/flappy_render.*`: top-screen framebuffer drawing, tiny bitmap font,
  overlays, and bottom-screen HUD caching.

## Build Instructions

From this directory:

```bash
make clean
make
```

Expected output:

```text
flappy_test.nds
```

To open the ROM with melonDS from the repository root:

```bash
scripts/run_melonds.sh flappy_test
```

The script first uses
`~/.local/share/hellonds/emulators/melonds/1.1/melonDS`, then a local `melonDS`
or `melonds` executable, the executable set in `MELONDS_BIN`, or the Flatpak app
`net.kuribo64.melonDS` if installed. Download melonDS from
<https://melonds.kuribo64.net/downloads.php> or install it with:

```bash
flatpak install flathub net.kuribo64.melonDS
```

For the official Ubuntu x86_64 ZIP, this repository expects the extracted
binary at `~/.local/share/hellonds/emulators/melonds/1.1/melonDS`. On Ubuntu
24.04/Noble, install the runtime libraries if melonDS reports missing shared
objects:

```bash
sudo apt-get install -y libqt6core6t64 libqt6gui6t64 libqt6network6t64 libqt6widgets6t64 libqt6multimedia6 libenet7 libfaad2
```

To capture the running emulator window for visual verification:

```bash
scripts/capture_melonds.sh flappy_test
```

The default screenshot is written to `captures/flappy_test_melonds.png`.

Copy `flappy_test.nds` to your Nintendo DS or DSi SD card and launch it with
your preferred homebrew loader, such as TWiLight Menu++.

If your environment variables are not already loaded, source Wonderful
Toolchain first:

```bash
source /opt/wonderful/bin/wf-env
cd flappy_test
make clean
make
```

The project only requires BlocksDS core and `libnds`; no Wi-Fi or external
libraries are needed.
