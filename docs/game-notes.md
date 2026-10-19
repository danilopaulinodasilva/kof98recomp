# The King of Fighters '98: notes

What this title needs from the toolkit, and what bringing it up showed. These notes cover behaviour. Nothing from the ROM is published, apart from the widescreen patch in `src/game.c`: the addresses of 42 immediate operands and their original values, which it checks before changing them.

## The set

MAME set `kof98h` (NGH-2420, the AES cartridge), plus the AES system ROM from the `neogeo` set. The file names below are the ones in a non-merged `kof98h.zip`.

| File | Size | CRC32 | Role |
|---|---|---|---|
| `242-pn1.bin` | 1 MB | `61ac868a` | 68000 program, the fixed `$000000` area; stored byte-swapped |
| `242-p2.bin` | 4 MB | `980aba4c` | 68000 program, four 1 MB banks for `$200000`; stored byte-swapped |
| `242-s1.bin` | 128 KB | `7f7b4805` | fix layer tiles |
| `242-c1.bin`..`242-c8.bin` | 8 × 8 MB | `e564ecd6` `bd959b60` `22127b4f` `0b4fa044` `9d10bed3` `da07b6a2` `f6d7a38a` `c823e045` | sprite tiles, four odd/even pairs |
| `242-mg1.bin` | 256 KB | `4e7a6b1b` | Z80 sound driver |
| `242-v1.bin`..`242-v4.bin` | 4 × 4 MB | `b9ea8051` `cc11106e` `044ea4e1` `7985ea30` | ADPCM samples |
| `neo-po.bin` | 128 KB | `16d0c132` | AES system ROM (Japan) |
| `sfix.sfix`, `sm1.sm1`, `000-lo.lo` | | `c2ea0cfd` `94416d67` `5a86cff2` | MVS fix tiles and sound driver (unused on an AES, but loaded), sprite shrink table |

The AES set has no encryption. The MVS set `kof98` does: its program is scrambled and the cartridge has a protection device, which the toolkit doesn't support.

## Why it runs as an AES

On an MVS board the AES cartridge plays one attract cycle and then stops at a "FOR USE WITH ORIGINAL SNK BOARDS ONLY" screen. When the game hands control back to an MVS system ROM, it leaves a flag in backup RAM; the next time the system ROM enters the game, it finds the flag and halts. So the game repo sets `.aes = 1` and `.bios = "neo-po.bin"`.

Bringing it up as an AES found a toolkit bug: the AES has no board SFIX or SM1, so the fix layer and the Z80 always see the cartridge's S and M1 ROMs. The runtime used to switch to the board ROMs anyway, which left the screen full of the wrong fix tiles.

## What it exercises

- **P ROM banking.** The program is 5 MB, with four 1 MB banks behind the `$200000` window. The game runs code from the banks, so the recompiler analyses each bank in its own context and the runtime picks a dispatch table by the bank that is mapped. References from the fixed area into the window are tried in every bank.
- **64 MB of sprite tiles**, which needs tile numbers up to 19 bits.
- **AES boot**: the AES system ROM, no coins or backup RAM.

## Widescreen

`--widescreen` (or any `--width` over 320) patches the program at load time through `ng_game_t.patch`. Stage backgrounds are layers of 32 sprite columns, a 512-pixel ring. The game streams tiles into the columns near the camera and hides the rest by giving them zero height, and both ranges are sized for 320 pixels. Two routines draw layers, one with a height per column (railings, floors), and both carry the same visibility test. The patch widens the streaming window and the visibility tests by the margin on each side, and narrows the camera range to match. The values are immediate operands, listed in `ng_game_t.live_words`, so the same recompiled build runs either way, and `--verify` checks both.

- The game left the last column before the right edge hidden, behind the overscan mask, so the visible range gets 8 pixels of slack.
- The streaming window (352 pixels plus both margins) has to fit the 512-pixel ring, so the patch stops at 80-pixel margins: wider output leaves the outermost pixels black.
- The window starts at the camera minus 16 without being aligned to a column, which works because the camera starts on one. It grows by whole columns (48 pixels per side at 400 wide); a misaligned window fetched columns 8 pixels off.
- Stages are 768 pixels wide, and each stage's setup routine (nine of them) sets the camera range to 0..448, where a 320-pixel view reaches either end. Past the ends the map repeats. Worse, a column left of the stage costs the game frames: it takes the column modulo the map width by repeated subtraction, and index -1 is 65,535. So the camera range shrinks by the rounded margin on each side. The stage ends sit 48 pixels further in at 400 wide, and the view always shows stage art.
- All nine stages (stage index at `$10A7EA`) were checked with the camera against both ends. A fighter entering between rounds is visible a little earlier.
- Other screens are drawn for 320 pixels. Their backgrounds are 20 sprite columns at X 0..304, and `ng_game_t.extend` fills the margins with copies of those columns: mirrored at the edge for textures and gradients (the stone menus, the character select, the title logo, intro and story backdrops, the continue screen, the K.O. flash), or repeated every 10 columns for the "K.O.F." lettering behind the character select. When the art's outer column is empty (left for the overscan), the next column in is the edge.
- The character select's gradient and lettering take their colour from the palette index, which rises from 1 at the outer edge toward the middle. Their copies are drawn in index 1 alone, so the gradient continues from the edge and copied letters keep the edge's colour. Index 1 also fills the 8 pixels the overscan used to hide, which were a lighter stripe at the edge of the picture.
- Mirroring shows on the stone backgrounds, so their margins come from new art in `assets/`, painted for this project. The images hold only the margins: the middle, where the game's picture goes, is empty.
  - `stone.png`: the green stone behind the menus, 496×224 in its 16-colour palette (`$50`), with the picture's place at X 80..399. "How to play" and the ranking show it as it is; mode select shows it 16 pixels further on, hence the extra 16 pixels on the right. Mode select's left margin meets the picture where it is (`art_xl`).
  - `stone-red.png`: the red stone behind the title logo, 480×224 in its palette (`$93`), the picture's place at X 80..399.
  - The copies draw the art through palette RAM, so it fades and dims with the screen. Colours an editor adds to the palette become the nearest of the background's. Without a file, the margins are mirrored. `KOF98_ASSETS` points at another folder.
- Only a list of backgrounds, known by their first tile, is extended. The text collages of the intro and a white panel that slides in from the edge would look wrong mirrored, so their margins stay black, as do illustrations narrower than the screen.
- The 8-pixel black columns at the edges of the original picture come from the fix layer, which CRTs hid in the overscan. The runtime doesn't draw them when the output is wider than 320.

## What discovery had to learn

- The pointer scan finds most code: about 39,900 routines from 38,600 pointer seeds.
- The profile pass adds about 530 runtime-only entry points across attract and a play session.
- At boot the game runs a routine from work RAM, which the static recompiler cannot see. It is about 11.7M interpreted instructions, all during start-up; after that the soak runs natively.

End state, with the profile pass (which plays attract and the soak in widescreen too): 40,775 routines and 57,944 dispatch entries. `--verify` finds 0 mismatches over 12.1M blocks of the soak, and over 11.7M in widescreen.

## Not yet

- **Sound**: the Z80 driver runs, but the YM2610 is silent (neogeorecomp ROADMAP).
- **Coverage**: the soak plays team matches against the CPU with player 1. Other modes (versus, survivor, practice, the boss) have not been scripted yet; any runtime-only code they reach runs in the interpreter until profiled.
