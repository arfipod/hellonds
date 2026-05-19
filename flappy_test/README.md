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
