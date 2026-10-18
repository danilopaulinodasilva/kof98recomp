# Roadmap

## Next

- **Widescreen outside fights**: extend the screens with patterned backgrounds (character select, mode select) using the game's own tiles; decide what illustrated screens (title, intro, win poses) show in the margins.
- **Sound**, once neogeorecomp has YM2610 synthesis. Nothing in this repo should need to change.
- **Script more modes** (versus, survivor, practice, the boss fight) into the soak and the profile, so their runtime-only code runs natively and `--verify` covers them.
- **Player 2 keys, gamepad input** and remappable keys (toolkit frontend).

## Later

- The MVS set `kof98`: it needs the 68000 descrambling and the protection device in the toolkit.
- Load `kof98h.zip` / `neogeo.zip` directly instead of unzipping (toolkit).
- A release process that ships the tool, never the game: the generator plus the runtime, which recompile the user's dump on first run.

## Out of scope

- Distributing ROMs, the recompiled C, or anything else derived from the game.
- Game-specific patches in the toolkit. If KOF98 needs something, it is hardware behaviour and goes in the toolkit.
