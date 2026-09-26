# Mac build and emulator workflow

This project now has a command-line build and an automated boot test. Porymap
edits map source files; the mGBA desktop app is for interactive playtesting.
The test runner uses a separate, local build of mGBA 0.10.5's emulator core.

## Install Porymap

1. Visit <https://github.com/huderlem/porymap#download>.
2. Download **macOS latest (arm)** for this Apple Silicon Mac.
3. Extract the ZIP and open `porymap.dmg` inside the extracted folder.
4. Drag `porymap.app` into Applications and open it there.
5. If Gatekeeper blocks it, use System Settings → Privacy & Security → Open
   Anyway for this download from the official project.
6. Choose File → Open Project and select
   `/Users/adamnorton/Documents/misc/emerald-hack` (the folder containing Makefile).
7. Open Littleroot Town to confirm that maps load.

The inspected 6.3.1 ARM application declares macOS 13 as its minimum, compatible
with this Mac's macOS 15.7.4. Newer releases may change their requirements.

On this Mac, the official 6.3.1 ARM release also produced a “damaged” warning:
its app bundle had an incomplete ad-hoc signature. The installed files were
compared with the official download (ZIP SHA-256
`e9b62e40829f3bd55e9454bb4e783413b94a4edc69775c64cad007c248990619`) and matched.
The app was backed up to `/tmp/emerald-porymap-before-signing.app`, locally
re-signed, and its quarantine attribute removed. Signature verification passed
and Launch Services accepted the repaired app. This repair is already applied;
it does not establish Apple notarization. Recheck provenance before applying a
similar repair to any future download.

After saving map changes, rebuild the ROM to see them in the emulator.

## Dependencies

The machine has Xcode developer tools, Homebrew libpng/pkgconf, devkitARM,
agbcc installed into `tools/agbcc`, and the mGBA desktop app. CMake was also
installed for the automated test runner. No Python packages or Lua installation
are required; the runner uses `/usr/bin/python3` and the mGBA C API.

To restore the Homebrew dependencies on another Apple Silicon Mac:

```sh
brew install libpng pkgconf cmake
brew install --cask mgba-app
```

Follow INSTALL.md for devkitARM and agbcc. Use `sudo dkp-pacman`, not the raw
pacman executable: the wrapper sets the path required for signature verification.

## One-time emulator setup

Already completed on this Mac. To recreate it, run from the repository:

```sh
bash scripts/setup-emulator.sh
```

This downloads the official 0.10.5 source archive, verifies its pinned SHA-256,
and compiles the emulator core and local runner into `build/automation/`.
It does not install another desktop app. Downloading needs network access; normal
test runs work offline and do not need GUI automation or screen-recording access.
The cached archive can be reused for subsequent setup runs.

## Build and test

```sh
cd ~/Documents/misc/emerald-hack
bash scripts/test-rom.sh
```

This builds the ROM, then runs `tests/emulator/boot.json` from a fresh boot with
blank save data. It checks that the game reaches the title screen, responds to
Start by opening the main menu, and responds to A by starting Birch's introduction.
It captures a screenshot at each checkpoint. The RTC uses a fixed time and the
runner uses mGBA's built-in BIOS implementation, without desktop settings or cheats.

The process returns exit code 0 for a pass and nonzero for build errors, failed
checks, or an unresponsive emulator. A timeout produces a failure, not a pass.
It reads the current ELF's symbols to locate functions and RAM, so relocated
addresses after source changes do not need to be hardcoded.

Every run prints its unique directory under `build/test-runs/`. It contains:

- `result.json`: pass/fail, checked tasks, frame numbers, ROM hash, timing, errors.
- `01-title.png`, `02-new-game-menu.png`, `03-birch-introduction.png` on success.
- `failure.png` when a failed check can still capture a frame.
- `emulator.log`: emulator diagnostics.
- `test.gba`, `test.elf`, `scenario.json`: the exact inputs used in that run.

Tests use their own ROM copy and do not load or modify your normal playthrough
save. Generated tools, ROMs, and test artifacts live under the ignored `build/`
directory. If that directory is removed, run setup again.

Build only:

```sh
bash scripts/build-rom.sh
```

Play the current build:

```sh
open -a mGBA pokeemerald.gba
```

The scripts set Homebrew and devkitARM paths explicitly; they do not require
loading `.zshrc`. Build parallelism defaults to eight jobs; override with `JOBS=4`.
For an unchanged baseline, `bash scripts/build-rom.sh compare` should report
`pokeemerald.gba: OK`. That original-ROM checksum is expected to differ after edits.

## Adding tests

Copy `tests/emulator/boot.json` and run the new scenario with:

```sh
bash scripts/test-rom.sh --scenario tests/emulator/your-test.json
```

Actions currently supported:

| Action | Parameters | Meaning |
| --- | --- | --- |
| `run` | `frames`, optional `keys` | Run frames with those buttons held; omitted keys releases all buttons. |
| `wait_task` | `task`, optional `timeout_frames` | Run with no buttons until a named game task is active. |
| `assert_task` | `task` | Fail immediately unless a named task is active. |
| `screenshot` | `name` | Write a PNG into the current run directory. |

Button names: A, B, SELECT, START, RIGHT, LEFT, UP, DOWN, R, L. Release buttons
between presses to create another key-down event. Task checks follow
`include/task.h` (16 tasks, 40 bytes each); update the reader if that layout changes.

The initial test checks boot/menu/input behavior only. Screenshots are evidence
for visual inspection, not automatic assertions about every pixel. Changes to
NPCs, battles, maps, saving, audio, or later gameplay need targeted scenarios and
possibly additional runner actions. Intentional intro changes may require updating
the baseline scenario. Desktop audio, display, and controller integration remain
manual checks in the mGBA app.

An assistant can run the same command and inspect `result.json`, screenshots,
and logs, then add a scenario for the behavior changed. No desktop control is
needed for these tests.

Sources: <https://github.com/mgba-emu/mgba/tree/0.10.5>,
<https://github.com/huderlem/porymap>.
