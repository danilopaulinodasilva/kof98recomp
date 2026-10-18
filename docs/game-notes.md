# The King of Fighters '98: notes

What this title needs from the toolkit, and what bringing it up showed. These notes cover behaviour. Nothing from the ROM is published, apart from the widescreen patch in `src/game.c`: the addresses of 16 immediate operands and their original values, which it checks before changing them.

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

`--widescreen` (or any `--width` over 320) patches the program at load time through `ng_game_t.patch`. Stage backgrounds are layers of 32 sprite columns, a 512-pixel ring. The game streams tiles into the columns near the camera and hides the rest by giving them zero height, and both ranges are sized for 320 pixels. The patch widens both by the margin on each side. The values are immediate operands, listed in `ng_game_t.live_words`, so the same recompiled build runs either way, and `--verify` checks both.

- Fights use the full width. With the camera against either end of a stage, the floor runs out in the outer margin, and a fighter entering between rounds is visible a little earlier.
- Menus, the character select and the win screens are fixed 320-pixel pictures; their margins stay black.
- The 8-pixel black columns at the edges of the original picture come from the fix layer, which CRTs hid in the overscan. The runtime doesn't draw them when the output is wider than 320.

## What discovery had to learn

- The pointer scan finds most code: about 39,900 routines from 38,600 pointer seeds.
- The profile pass adds about 530 runtime-only entry points across attract and a play session.
- At boot the game runs a routine from work RAM, which the static recompiler cannot see. It is about 11.7M interpreted instructions, all during start-up; after that the soak runs natively.

End state, with the profile pass (which plays the soak in widescreen too): 40,459 routines and 57,604 dispatch entries. `--verify` finds 0 mismatches over 12.1M blocks of the soak, and over 11.8M in widescreen.

## Not yet

- **Sound**: the Z80 driver runs, but the YM2610 is silent (neogeorecomp ROADMAP).
- **Coverage**: the soak plays team matches against the CPU with player 1. Other modes (versus, survivor, practice, the boss) have not been scripted yet; any runtime-only code they reach runs in the interpreter until profiled.
