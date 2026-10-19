# Roadmap

## Next

- **Sound**: YM2610 synthesis in neogeorecomp. Nothing in this repo should need to change.
- **Player 2 keys, gamepad input** and remappable keys (toolkit frontend).
- **Script more modes** (versus, survivor, practice, the boss fight) into the soak and the profile, so their runtime-only code runs natively and `--verify` covers them.
- **Widescreen outside fights**: the intro's text collages and illustrations narrower than the screen still have black margins; decide what they show, possibly with more art in `assets/`.

## Launcher and video options

A start screen, before the game boots, to set things up without the command line:

- **Display**: 4:3 or widescreen, integer scaling, fullscreen or window, which monitor, vsync.
- **Filters**: CRT scanlines, shadow mask or aperture grille, screen curvature, bloom and glow, and the smoother scalers (xBR, hqx), as shaders, so they cost almost nothing.
- **Colour**: brightness, gamma and saturation, and a "PVM" preset.
- **Input**: key and gamepad mapping for both players, turbo and macros off by default.
- **Game**: region (Japan, US, Europe system ROMs), difficulty and the other options the game's own menu has, and a soft reset.
- **Unlock all characters**: the hidden fighters (the Orochi versions, Omega Rugal and the EX versions) available from the start, without the code at the character select. It means finding the flag the code sets and setting it at boot.
- **Recording**: screenshots and clips with a key, and the `--record` options.
- **Quality of life**: save states, fast-forward, a frame counter and input display for practice, and remembering the last settings.

## The rest of the series

One repo per game, on the same toolkit:

- **KOF '94, '95, '96, '97**: unencrypted, close to '98. Each needs its ROM layout, its own reverse engineering of the stage engine for widescreen, and test scripts.
- **KOF '99, 2000, 2001, 2002, 2003**: encrypted graphics, and program or sound encryption and protection chips depending on the game. The toolkit needs each scheme first.

## Later

- The MVS set `kof98`: it needs the 68000 descrambling and the protection device in the toolkit.
- Load `kof98h.zip` / `neogeo.zip` directly instead of unzipping (toolkit).
- A release process that ships the tool, never the game: the generator plus the runtime, which recompile the user's dump on first run.

## Out of scope

- Distributing ROMs, the recompiled C, or anything else derived from the game.
- Game-specific patches in the toolkit. If KOF98 needs something, it is hardware behaviour and goes in the toolkit.
