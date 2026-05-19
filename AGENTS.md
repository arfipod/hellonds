# Agent Notes

This repository contains Nintendo DS and DSi homebrew projects built with
BlocksDS. Keep repository documentation in English.

## Build Environment

The Wonderful Toolchain is expected at `/opt/wonderful`. If a shell does not
already have the toolchain variables, source them before building:

```bash
source /opt/wonderful/bin/wf-env
```

Useful checks:

```bash
scripts/verify_blocksds_env.sh
scripts/build_all.sh
```

## melonDS

Use melonDS for local emulator checks. Do not use Wine/no$gba for this repo.

The preferred local melonDS binary is:

```text
~/.local/share/hellonds/emulators/melonds/1.1/melonDS
```

It comes from the official Ubuntu x86_64 release ZIP:

```text
https://melonds.kuribo64.net/downloads/melonDS-1.1-ubuntu-x86_64.zip
```

The expected SHA256 for that ZIP is:

```text
99465129f5413b2aad332e4377e523cf3cda905dc329d47dcb1ad01ce2cb3f66
```

If the binary is missing, download and extract it:

```bash
mkdir -p ~/.local/share/hellonds/emulators/melonds/1.1
curl -L --fail -o ~/.local/share/hellonds/emulators/melonds/melonDS-1.1-ubuntu-x86_64.zip https://melonds.kuribo64.net/downloads/melonDS-1.1-ubuntu-x86_64.zip
unzip -o ~/.local/share/hellonds/emulators/melonds/melonDS-1.1-ubuntu-x86_64.zip -d ~/.local/share/hellonds/emulators/melonds/1.1
chmod +x ~/.local/share/hellonds/emulators/melonds/1.1/melonDS
```

On Ubuntu 24.04/Noble, the official binary needs these runtime libraries:

```bash
sudo apt-get install -y libqt6core6t64 libqt6gui6t64 libqt6network6t64 libqt6widgets6t64 libqt6multimedia6 libenet7 libfaad2
```

## Running ROMs

From the repository root:

```bash
scripts/run_melonds.sh flappy_test
scripts/run_melonds.sh gospel_nds
scripts/run_melonds.sh path/to/file.nds
```

The launcher builds known project targets before opening the ROM. It searches
for melonDS in this order:

1. `MELONDS_BIN`, if set.
2. `~/.local/share/hellonds/emulators/melonds/1.1/melonDS`.
3. `melonDS` or `melonds` on `PATH`.
4. Flatpak app `net.kuribo64.melonDS`.

## Capturing Screenshots

Use the capture helper when a visual check is useful:

```bash
scripts/capture_melonds.sh flappy_test
scripts/capture_melonds.sh gospel_nds
scripts/capture_melonds.sh --output captures/gospel.png --delay 3 gospel_nds
```

The capture helper forces `QT_QPA_PLATFORM=xcb`, starts melonDS in the
foreground, waits for a visible X11 melonDS window, writes a PNG, and then
closes the emulator. It does not require `gnome-screenshot`, `grim`, ImageMagick,
Pillow, or `pip`; it uses Python's standard library plus `libX11`.

Default capture output:

```text
captures/<target>_melonds.png
```

If melonDS opens under native Wayland, regular X11 window capture cannot see it.
That is why the helper forces Qt/XCB.
