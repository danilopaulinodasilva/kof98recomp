/*
 * game.c — the King of Fighters '98 (NGH-2420, AES cart) ROM set, shared
 * by the player and the generator so both load the program identically.
 */
#include "game.h"
#include <stdio.h>

/* Widescreen. Stage backgrounds are layers of 32 sprite columns (a 512
 * pixel ring); the game streams tiles into the columns near the camera
 * and hides the rest by giving them zero height. Both ranges are
 * immediates sized for 320 pixels, so a wider screen widens them by the
 * margin on each side. Each entry: the immediate's address, its original
 * value, and how much one pixel of margin adds. */
#define WIDE_PATCHES(X) \
    /* tile streaming window: set up (two layer kinds), then followed */ \
    X(0x007B58, 0x0010, 1) X(0x007B60, 0x0160, 2) \
    X(0x007BF4, 0x0010, 1) X(0x007BFE, 0x0150, 1) \
    X(0x0083D6, 0x0150, 1) X(0x0083EE, 0x0010, 1) \
    X(0x008694, 0x0150, 1) X(0x0086AC, 0x0010, 1) \
    /* visible columns: x + offset must be within limit (as x << 7, and x) */ \
    X(0x007FCA, 0x0780, 128) X(0x007FCE, 0xA600, 256) \
    X(0x007FEE, 0x0780, 128) X(0x007FF2, 0xA600, 256) \
    X(0x00802C, 0x000F, 1)   X(0x008030, 0x014C, 2) \
    X(0x008054, 0x000F, 1)   X(0x008058, 0x014C, 2)

static const struct { uint32_t addr; uint16_t orig; int per_px; } k_wide[] = {
#define ENTRY(a, o, k) {a, o, k},
    WIDE_PATCHES(ENTRY)
#undef ENTRY
};

static const uint32_t k_live_words[] = {
#define ADDR(a, o, k) a,
    WIDE_PATCHES(ADDR)
#undef ADDR
    0
};

static void patch(uint8_t *prom, size_t size, int screen_w) {
    int margin = (screen_w - 320) / 2;
    size_t n = sizeof(k_wide) / sizeof(k_wide[0]);
    (void)size;
    if (margin <= 0) return;
    for (size_t i = 0; i < n; i++) {
        const uint8_t *p = prom + k_wide[i].addr;
        if (((p[0] << 8) | p[1]) != k_wide[i].orig) {
            fprintf(stderr, "kof98: unexpected program at $%06X; widescreen is off\n", k_wide[i].addr);
            return;
        }
    }
    for (size_t i = 0; i < n; i++) {
        uint16_t v = (uint16_t)(k_wide[i].orig + margin * k_wide[i].per_px);
        prom[k_wide[i].addr] = (uint8_t)(v >> 8);
        prom[k_wide[i].addr + 1] = (uint8_t)v;
    }
}

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
    .patch = patch,
    .live_words = k_live_words,
};
