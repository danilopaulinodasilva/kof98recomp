/*
 * game.c — the King of Fighters '98 (NGH-2420, AES cart) ROM set, shared
 * by the player and the generator so both load the program identically.
 */
#include "game.h"

/* MAME set "kof98h", the unencrypted AES dump, plus the AES system ROM
 * neo-po.bin from the "neogeo" set. The 1 MB P1 is the fixed
 * $000000 area; the 4 MB P2 holds four banks for $200000. Both are stored
 * byte-swapped. */
const ng_game_t k_kof98 = {
    .name = "kof98",
    .title = "The King of Fighters '98 - The Slugfest",
    .prom_size = 0x500000,
    .prom = {
        {"242-pn1.bin", 0x000000, 0x000000, 0x100000, 1},
        {"242-p2.bin",  0x000000, 0x100000, 0x400000, 1},
    },
    .crom_size = 0x4000000,
    .crom_pairs = {
        {"242-c1.bin", "242-c2.bin"},
        {"242-c3.bin", "242-c4.bin"},
        {"242-c5.bin", "242-c6.bin"},
        {"242-c7.bin", "242-c8.bin"},
    },
    .srom = "242-s1.bin",
    .m1 = "242-mg1.bin",
    .vrom = {"242-v1.bin", "242-v2.bin", "242-v3.bin", "242-v4.bin"},
    /* The AES cart refuses to run on an MVS board after one attract cycle
     * (it leaves a flag in backup RAM), so it runs as a console. */
    .bios = "neo-po.bin",
    .aes = 1,
};
