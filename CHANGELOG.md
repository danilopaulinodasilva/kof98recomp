# Changelog

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow SemVer.

## [Unreleased]

### Added
- The King of Fighters '98 (MAME set `kof98h`, the AES cart) on neogeorecomp: AES boot, intro, attract demos and team matches, running on recompiled 68000 code. It needs neogeorecomp's multi-bank P ROM support and AES mode.
- `kof98_recomp`, the generator that recompiles the user's own ROMs into `build/generated` at build time (`KOF98_ROM_DIR`).
- `tests/start_fight.txt` (reach the first round) and `tests/play_long.txt` (5-minute soak), which the `profile` target plays.
- `Setup.cmd` / `setup.sh` find and checksum the `kof98h` and `neogeo` sets and write a KOF98 launcher.
- README, screenshots and `docs/game-notes.md` for KOF98.

### Removed
- Metal Slug's ROM layout, input scripts and screenshots, inherited from the fork.

## [0.0.1]
- Initial scaffolding (as metalslug).
