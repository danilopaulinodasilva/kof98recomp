# Contributing

Bug reports from playing are the most useful thing right now: what you did, what you expected, and a screenshot or `--record` clip. Include the input script if you have one, since a script plus the frame number reproduces a run exactly.

## Pull requests

- Most fixes belong in the toolkit, [neogeorecomp](https://github.com/danilopaulinodasilva/neogeorecomp) (this project's fork of sp00nznet's): this repo describes the ROM set, the widescreen patches and the test scripts. Nothing game-specific goes into the toolkit, and nothing hardware-specific goes here.
- Run `kof98 --headless --verify` with the scripts in `tests/` and include the `[verify]` line.
- **Never include ROMs, the recompiled C from `build/generated`, entry-point lists, or disassembly.** Everything derived from the game stays on your machine. Margin art in `assets/` holds only new pixels, never the game's own picture.

## Where your code comes from

Contributions must be original or MIT-compatible. Don't port code from GPL or non-commercial emulators (MAME, FBNeo, Mednafen and others); use them to understand the hardware and write the code yourself. The toolkit's CONTRIBUTING.md has more detail. AI-assisted contributions are welcome if a human understood and verified them.
