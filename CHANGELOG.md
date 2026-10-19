# Changelog

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow SemVer.

## [Unreleased]

### Added
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
