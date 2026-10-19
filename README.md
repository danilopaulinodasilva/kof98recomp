# The King of Fighters '98: static recompilation

**The King of Fighters '98 (SNK, 1998) running as a native PC program: its 68000 code is recompiled to C and runs on the [neogeorecomp](https://github.com/sp00nznet/neogeorecomp) board model.**

The toolkit does all the work; this repo describes the ROM set and builds the game. It started as a fork of [metalslug](https://github.com/sp00nznet/metalslug), the toolkit's reference game.

> **No game data here.** You need your own KOF98 and Neo Geo system ROM dumps (MAME sets `kof98h` and `neogeo`). The recompiled C is generated from your dump on your machine, into `build/`. It is never committed or distributed, and neither is anything else derived from the ROMs.

## Status

**v0.1.0-dev, alpha.** KOF98 boots, runs the intro and attract demos, and plays team matches on recompiled code. **There is no sound yet.**

- Runs as an AES console with the Japanese AES system ROM (`neo-po.bin`): the AES cart refuses to run on an MVS board ([docs/game-notes.md](docs/game-notes.md)).
- The 5 MB program is banked: the four 1 MB banks behind `$200000` are recompiled separately and dispatched by the bank that is mapped.
- **Widescreen** (`--widescreen`, 400×224): fights draw the stage across the whole width, and menus, the character select and other backgrounds continue into the margins in their own graphics.
- A 5-minute scripted soak runs natively after the profile pass, apart from a routine the game runs from work RAM at boot. `--verify` checks **12.1M** recompiled blocks (11.7M in widescreen) against the Musashi interpreter with **0 mismatches**.
- Not yet: sound, player 2 keys, gamepad support, and a settings menu.

## Screenshots

All real output from the recompiled build (`--headless --screenshot`):

| | |
|---|---|
| ![AES boot](docs/screenshots/bios.png) | ![Intro](docs/screenshots/intro.png) |
| ![Title](docs/screenshots/title.png) | ![Mode select](docs/screenshots/mode.png) |
| ![Team select](docs/screenshots/select.png) | ![Order select](docs/screenshots/order.png) |
| ![Fight](docs/screenshots/fight.png) | |

Widescreen (`--widescreen`):

![Widescreen fight](docs/screenshots/fight-wide.png)

## Getting started

### Quick start (Windows)

1. Download this repo: **Code → Download ZIP**, then unzip it. A `git clone` works too.
2. Put your **`kof98h.zip`** and **`neogeo.zip`** (MAME sets) in the unzipped folder, or in `Downloads`.
3. Double-click **`Setup.cmd`**.

   It checks for Git, CMake, the Visual Studio C++ build tools and SDL2, and asks before installing anything that's missing (it says what and how big). It finds and checksums your ROMs, recompiles and builds the game (a few minutes), and leaves a **KOF98** launcher in the folder. If something fails, it stops with one sentence on what to do and keeps the details in `setup.log`. Running it again picks up where it stopped.
4. Double-click **KOF98**.

**On Linux or macOS**, run `./setup.sh` instead, optionally with `--rom-zip PATH --bios-zip PATH`. It does the same steps and offers the `apt` command for anything missing. When it finishes, run `./kof98.sh`.

Keys: **arrows** move, **Z X C V** buttons A B C D, **1** start, **Esc** quit. It runs as an AES, so there are no coins: press start on the title screen. Start pauses during a round. For widescreen, run `./kof98.sh --widescreen`.

### Step by step

These are the commands `Setup.cmd` runs.

**Prerequisites:** Git; CMake 3.21+; a C compiler (Visual Studio 2022 with *Desktop development with C++* on Windows, gcc or clang elsewhere); SDL2 for the window (optional, since headless builds need nothing). On Linux: `sudo apt install git cmake gcc libsdl2-dev`. On macOS: `brew install cmake sdl2`.

1. **Clone with the toolkit submodule:**
   ```
   git clone --recurse-submodules https://github.com/danilopaulinodasilva/kof98recomp
   cd kof98recomp
   ```
2. **Unzip your ROMs into `roms/`.** From `kof98h.zip`: `242-pn1.bin 242-p2.bin 242-s1.bin 242-mg1.bin 242-c1.bin`..`242-c8.bin 242-v1.bin`..`242-v4.bin`. From `neogeo.zip` (or a non-merged `kof98h.zip`): `neo-po.bin sfix.sfix sm1.sm1 000-lo.lo`. [docs/game-notes.md](docs/game-notes.md) lists the CRC32s.
3. **Configure.** On Windows, point CMake at SDL2: vcpkg's toolchain file, or `-DSDL2_DIR=<SDL2-devel-VC>/cmake`.
   ```
   cmake -S . -B build -DKOF98_ROM_DIR=roms
   ```
4. **Build.** The generator recompiles your ROMs into `build/generated` first:
   ```
   cmake --build build --config Release
   ```
   Expected in the output:
   ```
   [m68krecomp] pointer scan: 38567 seeds
   [m68krecomp] kof98: 1555625 instructions emitted across routines
   [m68krecomp] 39917 routines, 57050 dispatch entries -> .../build/generated
   ```
5. **Profile (optional).** This plays two scripted sessions headless to find code only reached at runtime, then rebuilds with it:
   ```
   cmake --build build --config Release --target profile
   cmake --build build --config Release
   ```
6. **Play:**
   ```
   build/Release/kof98 --rom-path roms          (Linux/macOS: build/kof98)
   ```

Trip-ups:
- *`error: cannot open roms/242-pn1.bin`*: the ROMs are not in `roms/`, or you are running from a different folder. Pass `--rom-path`.
- *A "FOR USE WITH ORIGINAL SNK BOARDS ONLY" screen*: you ran it with an MVS system ROM (`--bios sp-s2.sp1`). Leave `--bios` out.
- *"Submodules missing"*: you cloned without `--recurse-submodules`. Run `git submodule update --init --recursive`.
- *No window, and "built without SDL2; run with --headless"*: CMake didn't find SDL2 (see step 3).

## Usage

```
kof98 --rom-path roms                                      # play
kof98 --rom-path roms --widescreen                         # play in 16:9
kof98 --headless --frames 4400 --input tests/start_fight.txt --record run.mp4
kof98 --headless --verify --frames 19000 --input tests/play_long.txt
kof98 --headless --interp --frames 4400 --screenshot 4350:interp.png
```

The toolkit's [docs/running.md](https://github.com/sp00nznet/neogeorecomp/blob/master/docs/running.md) covers every option and the input-script format. `tests/start_fight.txt` reaches the first round, and `tests/play_long.txt` is the 5-minute soak.

## Building from source

See *Step by step* above. `KOF98_ROM_DIR` is the only game-specific option. Without it, the build is interpreter-only, which is useful for working on the toolkit without regenerating.

## Documentation

- [docs/game-notes.md](docs/game-notes.md): the ROM set and what the game needed from the toolkit
- [neogeorecomp](https://github.com/sp00nznet/neogeorecomp): the architecture and recompiler docs
- [CHANGELOG.md](CHANGELOG.md) · [ROADMAP.md](ROADMAP.md) · [CONTRIBUTING.md](CONTRIBUTING.md)

## License

The code in this repo is MIT ([LICENSE](LICENSE)). *The King of Fighters '98* is © SNK; this project contains none of its code or data and needs your own dump to do anything.
