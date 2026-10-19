/*
 * game.c — the King of Fighters '98 (NGH-2420, AES cart) ROM set, shared
 * by the player and the generator so both load the program identically.
 */
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Widescreen. Stage backgrounds are layers of 32 sprite columns (a 512
 * pixel ring); the game streams tiles into the columns near the camera
 * and hides the rest by giving them zero height. Both ranges are
 * immediates sized for 320 pixels, so a wider screen widens them by the
 * margin on each side. Each entry: the immediate's address, its original
 * value, how much one pixel of margin adds, the slack in pixels, and
 * whether it takes the margin rounded up to a whole 16-pixel column.
 *
 * The visible range gets 8 pixels of slack: the game leaves the last
 * column before the right edge hidden, which the overscan mask covered.
 * The streaming window is 352 pixels plus both margins and must fit the
 * ring, so margins stop at 80 pixels (480 wide).
 *
 * The streaming window starts at the camera minus 16, unaligned, which
 * only works because the camera starts on a column; so the window grows
 * by whole columns.
 *
 * Stages are 768 pixels wide and the camera stops where a 320-pixel view
 * reaches either end. Past the ends the map repeats, and fetching a column
 * left of the stage costs the game frames (it takes the column modulo the
 * map width by repeated subtraction). So the camera stops a rounded
 * margin earlier: the window then never passes the stage. */
#define WIDE_PATCHES(X) \
    /* tile streaming window: set up (two layer kinds), then followed */ \
    X(0x007B58, 0x0010, 1, 0, 1) X(0x007B60, 0x0160, 2, 0, 1) \
    X(0x007BF4, 0x0010, 1, 0, 1) X(0x007BFE, 0x0150, 1, 0, 1) \
    X(0x0083D6, 0x0150, 1, 0, 1) X(0x0083EE, 0x0010, 1, 0, 1) \
    X(0x008694, 0x0150, 1, 0, 1) X(0x0086AC, 0x0010, 1, 0, 1) \
    /* visible columns: x + offset must be within limit (as x << 7, and x) */ \
    X(0x007FCA, 0x0780, 128, 8, 0) X(0x007FCE, 0xA600, 256, 8, 0) \
    X(0x007FEE, 0x0780, 128, 8, 0) X(0x007FF2, 0xA600, 256, 8, 0) \
    X(0x00802C, 0x000F, 1, 8, 0)   X(0x008030, 0x014C, 2, 8, 0) \
    X(0x008054, 0x000F, 1, 8, 0)   X(0x008058, 0x014C, 2, 8, 0) \
    /* the same for layers with a height per column (railings, floors) */ \
    X(0x00824C, 0x0780, 128, 8, 0) X(0x008250, 0xA600, 256, 8, 0) \
    X(0x008278, 0x0780, 128, 8, 0) X(0x00827C, 0xA600, 256, 8, 0) \
    X(0x0082C2, 0x000F, 1, 8, 0)   X(0x0082C6, 0x014C, 2, 8, 0) \
    X(0x0082F2, 0x000F, 1, 8, 0)   X(0x0082F6, 0x014C, 2, 8, 0) \
    /* camera limits, set up by each stage (the high word of 16.16 x) */ \
    X(0x091182, 0x0000, 1, 0, 1)   X(0x09118A, 0x01C0, -1, 0, 1) \
    X(0x091504, 0x0000, 1, 0, 1)   X(0x09150C, 0x01C0, -1, 0, 1) \
    X(0x09192A, 0x0000, 1, 0, 1)   X(0x091932, 0x01C0, -1, 0, 1) \
    X(0x091A6E, 0x0000, 1, 0, 1)   X(0x091A76, 0x01C0, -1, 0, 1) \
    X(0x0920BA, 0x0000, 1, 0, 1)   X(0x0920C2, 0x01C0, -1, 0, 1) \
    X(0x0922B0, 0x0000, 1, 0, 1)   X(0x0922B8, 0x01C0, -1, 0, 1) \
    X(0x092656, 0x0000, 1, 0, 1)   X(0x09265E, 0x01C0, -1, 0, 1) \
    X(0x092824, 0x0000, 1, 0, 1)   X(0x09282C, 0x01C0, -1, 0, 1) \
    X(0x092AE6, 0x0000, 1, 0, 1)   X(0x092AEE, 0x01C0, -1, 0, 1)

#define MAX_MARGIN 80

static const struct { uint32_t addr; uint16_t orig; int per_px, slack, round; } k_wide[] = {
#define ENTRY(a, o, k, sl, r) {a, o, k, sl, r},
    WIDE_PATCHES(ENTRY)
#undef ENTRY
};

static const uint32_t k_live_words[] = {
#define ADDR(a, o, k, sl, r) a,
    WIDE_PATCHES(ADDR)
#undef ADDR
    0
};

static void patch(uint8_t *prom, size_t size, int screen_w) {
    int margin = (screen_w - 320) / 2;
    if (margin > MAX_MARGIN) margin = MAX_MARGIN;
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
        int m = k_wide[i].round ? (margin + 15) & ~15 : margin;
        uint16_t v = (uint16_t)(k_wide[i].orig + (m + k_wide[i].slack) * k_wide[i].per_px);
        prom[k_wide[i].addr] = (uint8_t)(v >> 8);
        prom[k_wide[i].addr + 1] = (uint8_t)v;
    }
}

/* Screens other than fights are drawn for 320 pixels. A background that
 * fills one is 20 columns of 16 pixels at X 0..304, so the margins get
 * more of it, as copies of its columns: mirrored at each edge, which
 * continues textures without a seam, or repeated with the period of a
 * pattern that mirroring would turn backwards. Only the backgrounds
 * listed are extended, known by their first tile: mirrored text or a
 * panel sliding in from the edge would look wrong.
 *
 * The character select's gradient and lettering pick their colour by
 * position: palette index 1 at the outer edge, rising toward the middle.
 * Their copies are drawn in index 1 alone, so the gradient carries on
 * from the edge and copied letters keep the edge's colour. Index 1 also
 * fills the 8 pixels the overscan used to hide.
 *
 * A background can instead take its margins from art of the user's own:
 * an indexed PNG in the asset folder (KOF98_ASSETS, default assets/ in
 * the repo), painted in the background's palette, which the copies draw
 * through palette RAM so it fades with the screen. `art_x` is where the
 * background's X 0 falls in the image. Without the file, the margins are
 * mirrored. */
static const struct { uint32_t tile; int period, colour; const char *art; int art_x; } k_backgrounds[] = {
    {0x07417, 0, 0, "stone.png", 80},                      /* stone: how to play, ranking */
    {0x07425, 0, 0, "stone.png", 96},                      /* the same 16 pixels on: mode select */
    {0x07416, 0, 0, NULL, 0},                              /* continue */
    {0x0787E, 0, 1, NULL, 0},                              /* character select: blue to red */
    {0x07824, 10, 1, NULL, 0},                             /* and its "K.O.F." lettering */
    {0x097B6, 0, 0, NULL, 0},                              /* title logo */
    {0x094FC, 0, 0, NULL, 0}, {0x09B72, 0, 0, NULL, 0},    /* intro */
    {0x0D23F, 0, 0, NULL, 0}, {0x0D526, 0, 0, NULL, 0},
    {0x08DD0, 0, 0, NULL, 0}, {0x08EE8, 0, 0, NULL, 0},    /* team story */
    {0x0E15F, 0, 0, NULL, 0}, {0x0E1A7, 0, 0, NULL, 0}, {0x0E1F3, 0, 0, NULL, 0},   /* continue countdown */
    {0x0E237, 0, 0, NULL, 0}, {0x0E273, 0, 0, NULL, 0}, {0x0E29B, 0, 0, NULL, 0},
    {0x0E2C3, 0, 0, NULL, 0}, {0x0E2D7, 0, 0, NULL, 0}, {0x0E2EB, 0, 0, NULL, 0},
    {0x07612, 0, 0, NULL, 0},                              /* the white flash of a K.O. */
};
#define N_BACKGROUNDS (sizeof(k_backgrounds) / sizeof(k_backgrounds[0]))

#ifdef KOF98_ASSET_DIR
/* Loaded art, and the 16-pixel columns cut from it (16 indices per line,
 * 512 lines), made once per background and X. */
static struct { uint8_t *png; int w, h, tried; } s_art[N_BACKGROUNDS];
static struct { int bg, x; uint8_t px[16 * 512]; } *s_cols;
static int s_ncols;

static const uint8_t *art_column(int bg, int x) {
    if (!s_art[bg].tried) {
        s_art[bg].tried = 1;
        for (size_t i = 0; i < (size_t)bg; i++)   /* backgrounds sharing a file share the load */
            if (k_backgrounds[i].art && !strcmp(k_backgrounds[i].art, k_backgrounds[bg].art) && s_art[i].png) {
                s_art[bg] = s_art[i];
                break;
            }
        if (!s_art[bg].png) {
            const char *dir = getenv("KOF98_ASSETS") ? getenv("KOF98_ASSETS") : KOF98_ASSET_DIR;
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, k_backgrounds[bg].art);
            FILE *f = fopen(path, "rb");
            if (f) {
                fclose(f);
                uint8_t rgb[768];
                uint8_t *png = ng_read_indexed_png(path, &s_art[bg].w, &s_art[bg].h, rgb);
                /* Editors add colours of their own to the palette: those
                 * become the nearest of the 15 the background has. */
                for (int i = 0; png && i < s_art[bg].w * s_art[bg].h; i++) {
                    if (png[i] < 16) continue;
                    const uint8_t *c = rgb + png[i] * 3;
                    int best = 1, best_d = 1 << 30;
                    for (int k = 1; k < 16; k++) {
                        int dr = c[0] - rgb[k * 3], dg = c[1] - rgb[k * 3 + 1], db = c[2] - rgb[k * 3 + 2];
                        int d = dr * dr + dg * dg + db * db;
                        if (d < best_d) { best_d = d; best = k; }
                    }
                    png[i] = (uint8_t)best;
                }
                s_art[bg].png = png;
            }
        }
    }
    if (!s_art[bg].png) return NULL;
    for (int i = 0; i < s_ncols; i++)
        if (s_cols[i].bg == bg && s_cols[i].x == x) return s_cols[i].px;
    s_cols = realloc(s_cols, sizeof(*s_cols) * (size_t)(s_ncols + 1));
    uint8_t *px = s_cols[s_ncols].px;
    s_cols[s_ncols].bg = bg; s_cols[s_ncols].x = x; s_ncols++;
    memset(px, 0, 16 * 512);
    int x0 = k_backgrounds[bg].art_x + x;
    for (int y = 0; y < s_art[bg].h && y < 512; y++)
        for (int i = 0; i < 16; i++)
            if (x0 + i >= 0 && x0 + i < s_art[bg].w) px[y * 16 + i] = s_art[bg].png[y * s_art[bg].w + x0 + i];
    return px;
}
#else
static const uint8_t *art_column(int bg, int x) { (void)bg; (void)x; return NULL; }
#endif

static uint32_t first_tile(const uint16_t *vram, int n, int rows) {
    if (rows > 32) rows = 32;
    for (int i = 0; i < rows; i++) {
        uint32_t code = vram[n * 64 + i * 2] | (((uint32_t)vram[n * 64 + i * 2 + 1] << 12) & 0xF0000u);
        if (code) return code;
    }
    return 0;
}

static int extend(const uint16_t *vram, int screen_w, ng_extra_sprite_t *out, int max) {
    int sx[381], sy[381], srows[381], full[381];
    int x = 0, y = 0, rows = 0, zoom_x = 0, n_out = 0;
    int margin = (screen_w - 320) / 2;
    for (int n = 0; n < 381; n++) {
        uint16_t ctl = vram[0x8200 + n], zoom = vram[0x8000 + n];
        if (ctl & 0x40) {
            x = (x + zoom_x + 1) & 0x1FF;
        } else {
            y = ctl >> 7;
            x = vram[0x8400 + n] >> 7;
            rows = ctl & 0x3F;
        }
        zoom_x = (zoom >> 8) & 0x0F;
        sx[n] = x; sy[n] = y; srows[n] = rows; full[n] = zoom_x == 15;
    }
    for (int s = 0; s + 20 <= 381; s++) {
        if (sx[s] != 0 || srows[s] == 0 || !full[s]) continue;
        int k = 1;
        while (k < 20 && sx[s + k] == 16 * k && sy[s + k] == sy[s] && srows[s + k] == srows[s] && full[s + k]) k++;
        if (k < 20) continue;
        /* A layer that already reaches into the margins (a fight's stage). */
        if ((s > 0 && sx[s - 1] == 496 && srows[s - 1] && sy[s - 1] == sy[s]) ||
            (s + 20 < 381 && sx[s + 20] == 320 && srows[s + 20] && sy[s + 20] == sy[s])) continue;
        /* Art may leave an edge column empty, for the overscan to hide:
         * the picture's edge is then the next column in. */
        int a = 0, b = 19;
        while (a < b && !first_tile(vram, s + a, srows[s])) a++;
        while (b > a && !first_tile(vram, s + b, srows[s])) b--;
        if (a > 2 || b < 17) continue;
        uint32_t t = first_tile(vram, s + a, srows[s]);
        int bg = -1;
        for (size_t i = 0; i < N_BACKGROUNDS; i++)
            if (k_backgrounds[i].tile == t) bg = (int)i;
        if (bg < 0) continue;
        int period = k_backgrounds[bg].period, colour = k_backgrounds[bg].colour;
        uint16_t pal = vram[(s + a) * 64 + 1] >> 8;
        for (int c = 1; 16 * (a - c) + 16 > -margin && n_out < max; c++) {
            ng_extra_sprite_t *e = &out[n_out++];
            *e = (ng_extra_sprite_t){0};
            e->colour = (uint8_t)colour;
            e->x = (uint16_t)((16 * (a - c)) & 0x1FF);
            if (k_backgrounds[bg].art && (e->art = art_column(bg, 16 * (a - c)))) {
                e->slot = (uint16_t)(s + a); e->pal = pal;
                continue;
            }
            if (period >= c) { e->slot = (uint16_t)(s + a + period - c); e->flip = 0; }
            else             { e->slot = (uint16_t)(s + a + c - 1);      e->flip = 1; }
        }
        for (int c = 1; 16 * (b + c) < 320 + margin && n_out < max; c++) {
            ng_extra_sprite_t *e = &out[n_out++];
            *e = (ng_extra_sprite_t){0};
            e->colour = (uint8_t)colour;
            e->x = (uint16_t)((16 * (b + c)) & 0x1FF);
            if (k_backgrounds[bg].art && (e->art = art_column(bg, 16 * (b + c)))) {
                e->slot = (uint16_t)(s + a); e->pal = pal;
                continue;
            }
            if (period >= c) { e->slot = (uint16_t)(s + b - period + c); e->flip = 0; }
            else             { e->slot = (uint16_t)(s + b - c + 1);      e->flip = 1; }
        }
        s += 19;
    }
    return n_out;
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
    .extend = extend,
};
