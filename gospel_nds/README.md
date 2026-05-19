# Gospel NDS

A four-gospel reader for Nintendo DS, built with BlocksDS and the JSON files in
`res` using the Spanish Episcopal Conference text.

Signature: Angel R.

## Modes

- Continuous reading: starts at Matthew 1:1 and advances verse by verse through
  the four gospels.
- Citation search: selector for gospel, chapter, and verse.
- Random gospel: opens a random citation.
- Horizontal book mode: uses both screens as full pages with multiple verses.
- Vertical book mode: rotates the pages for a Hotel Dusk-style sideways DS.

## Controls

- Menu: `A` continuous reading, `B` citation search, `X` random citation, `Y`
  horizontal book mode, `SELECT` vertical book mode.
- Reader: `A`, `R`, or right advances; `L` or left goes back; up/down scroll
  long text; `Y` opens horizontal book mode; `START` opens vertical book mode;
  `SELECT` opens citation search; `B` returns to the menu.
- Book mode: `A`, `R`, or right turns the page forward; `L` or left turns the
  page back; `START` switches between horizontal and vertical layouts; `Y`
  returns to the normal reader; `SELECT` opens citation search; `X` opens a
  random citation; `B` returns to the menu.
- Search: left/right changes the field, up/down adjusts the value, `L`/`R`
  make larger jumps, `A` opens the citation, `Y` opens it in horizontal book
  mode, and `START` opens it in vertical book mode.

## Architecture

- `source/main.c`: hardware startup, timers, button scanning, and the main loop.
- `source/gospel_app.*`: application state machine, menus, reader, book mode,
  and citation search.
- `source/gospel_text.*`: text wrapping to 31 columns and two-screen page
  composition.
- `source/gospel_console.*`: console initialization and the custom enye glyph.
- `source/gospel_random.*`: pseudo-random generator and timer/input entropy
  mixing.
- `source/gospel_data.*`: generated data from the JSON files in `res`.
- `tools/generate_gospel_data.py`: regenerates `gospel_data.c`.

## Generate Data And Build

If you change the JSON files, regenerate the C table:

```bash
python3 tools/generate_gospel_data.py
```

Build from this directory:

```bash
make clean
make
```

Expected output:

```text
gospel_nds.nds
```

To open the ROM with melonDS from the repository root:

```bash
scripts/run_melonds.sh gospel_nds
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
scripts/capture_melonds.sh gospel_nds
```

The default screenshot is written to `captures/gospel_nds_melonds.png`.

If the environment is not already loaded:

```bash
source /opt/wonderful/bin/wf-env
cd gospel_nds
python3 tools/generate_gospel_data.py
make clean
make
```
