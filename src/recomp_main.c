/*
 * recomp_main.c — kof98_recomp, the generator: reads the user's ROM files
 * and writes the recompiled C into the build tree.
 */
#include <neogeorecomp/recomp.h>
#include "game.h"

int main(int argc, char **argv) { return ng_recomp_main(argc, argv, &k_kof98); }
