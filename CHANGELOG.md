# Changelog

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow SemVer.

## [Unreleased]

## [0.1.1] - 2026-10-21

Installers for Windows, Linux and macOS. They hold no game data: pick the folder with your own `kof98h.zip` and `neogeo.zip` and they build the game on your machine.

### Added
- **Installer** (recomp-launcher): `launcher/launcher.json` and the `installer` workflow build KOF98-Installer for Windows, Linux and macOS on every push; a `v*` tag attaches them to the release. It holds no game data: the player picks the folder with their MAME zips and it runs setup on them.
- CI: builds the interpreter-only player on Ubuntu, Windows and macOS on every push and PR.
- `setup.sh` installs what's missing with Homebrew on macOS and checks for a C++ compiler (ymfm needs one); both setup scripts print `[n/N]` steps for the installer's progress bar. On Windows, installing Git or CMake no longer stops setup for a rerun.

## [0.1.0] - 2026-10-19

First release, alpha: source only. Build it with your own `kof98h` and `neogeo` dumps (README).

### Added
- The pause menu (from neogeorecomp): Esc for volume, fullscreen, scanlines, 4:3 or 16:9, both players' keys, reset and quit.
- Player 2 and gamepads (from neogeorecomp).
- `tests/modes/`: one script per mode (team play, team VS, single play, single VS, survivor, practice), from the title screen into a fight where both players attack and do special moves; the profile pass plays each, at 320 and 400 wide. `tools/mode_scripts.py` writes them.
- Sound, from neogeorecomp's YM2610 synthesis: music, voices and effects.
- The King of Fighters '98 (MAME set `kof98h`, the AES cart) on neogeorecomp: AES boot, intro, attract demos and team matches, running on recompiled 68000 code. It needs neogeorecomp's multi-bank P ROM support and AES mode.
- Widescreen (`--widescreen`): fights draw the stage 400 pixels wide, by patching the background column ranges and the camera range at load time; the profile pass also plays the soak in widescreen. Menus and other backgrounds continue into the margins, mirrored, repeated or in their edge colour (`ng_game_t.extend`), the character select in its edge colour, letterbox bars while they are plain bands, and the stone behind the menus and the title from new margin art in `assets/`.
- `kof98_recomp`, the generator that recompiles the user's own ROMs into `build/generated` at build time (`KOF98_ROM_DIR`).
- `tests/start_fight.txt` (reach the first round) and `tests/play_long.txt` (5-minute soak), which the `profile` target plays.
- `Setup.cmd` / `setup.sh` find and checksum the `kof98h` and `neogeo` sets and write a KOF98 launcher.
- README, screenshots and `docs/game-notes.md` for KOF98.

### Removed
- Metal Slug's ROM layout, input scripts and screenshots, inherited from the fork.

## [0.0.1]
- Initial scaffolding (as metalslug).
