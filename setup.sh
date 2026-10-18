#!/usr/bin/env bash
# setup.sh - the Quick start for Linux/macOS: the same steps as Setup.cmd.
# Checks prerequisites (and offers the apt command when one is missing),
# finds and checksums your ROMs, builds, profiles, and writes ./kof98.sh.
# Re-running resumes. Details go to setup.log.
#
#   ./setup.sh [--rom-zip kof98h.zip] [--bios-zip neogeo.zip] [--yes] [--no-profile]
set -u
ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG="$ROOT/setup.log"
ROMS="$ROOT/roms"
BUILD="$ROOT/build"
ROM_ZIP="" BIOS_ZIP="" YES=0 PROFILE=1
while [ $# -gt 0 ]; do
    case "$1" in
        --rom-zip) ROM_ZIP="$2"; shift ;;
        --bios-zip) BIOS_ZIP="$2"; shift ;;
        --yes) YES=1 ;;
        --no-profile) PROFILE=0 ;;
        *) echo "usage: $0 [--rom-zip FILE] [--bios-zip FILE] [--yes] [--no-profile]"; exit 2 ;;
    esac
    shift
done
echo "setup started $(date)" > "$LOG"

say() { echo "$*"; echo "$*" >> "$LOG"; }
step() { echo; echo "== $*"; echo "== $*" >> "$LOG"; }
fail() { echo; echo "Setup stopped: $*"; echo "Details are in $LOG"; echo "FAILED: $*" >> "$LOG"; exit 1; }
run() { echo "> $*" >> "$LOG"; "$@" >> "$LOG" 2>&1; }
ask() {
    if [ "$YES" = 1 ]; then return 0; fi
    printf "%s [y/N] " "$1"; read -r a; case "$a" in y|Y|yes) return 0 ;; *) return 1 ;; esac
}

step "Checking prerequisites"
missing=""
for tool in git cmake cc unzip; do command -v "$tool" >/dev/null || missing="$missing $tool"; done
pkg-config --exists sdl2 2>/dev/null || missing="$missing sdl2"
if [ -n "$missing" ]; then
    say "  missing:$missing"
    if command -v apt-get >/dev/null && ask "Install git cmake gcc unzip libsdl2-dev with apt (sudo, about 200 MB)?"; then
        run sudo apt-get update && run sudo apt-get install -y git cmake gcc unzip libsdl2-dev pkg-config \
            || fail "apt could not install the prerequisites."
    else
        fail "install:$missing (e.g. sudo apt install git cmake gcc unzip libsdl2-dev) and run ./setup.sh again."
    fi
fi
say "  $(cmake --version | head -1)"

step "Fetching the neogeorecomp toolkit"
if [ ! -f "$ROOT/neogeorecomp/ext/musashi/m68k_in.c" ]; then
    if [ -d "$ROOT/.git" ]; then
        run git -C "$ROOT" submodule update --init --recursive || fail "git could not fetch the toolkit submodule."
    else
        ref="$(head -1 "$ROOT/toolkit.ref" | tr -d '\r ')"
        rm -rf "$ROOT/neogeorecomp"
        run git clone https://github.com/danilopaulinodasilva/neogeorecomp.git "$ROOT/neogeorecomp" || fail "git could not clone the toolkit."
        run git -C "$ROOT/neogeorecomp" checkout "$ref" || fail "the toolkit has no commit $ref."
        run git -C "$ROOT/neogeorecomp" submodule update --init --recursive || fail "git could not fetch the toolkit's submodules."
    fi
fi
say "  toolkit ready"

step "Checking your ROM files"
GAME="242-pn1.bin:61ac868a 242-p2.bin:980aba4c 242-s1.bin:7f7b4805 242-mg1.bin:4e7a6b1b
      242-c1.bin:e564ecd6 242-c2.bin:bd959b60 242-c3.bin:22127b4f 242-c4.bin:0b4fa044
      242-c5.bin:9d10bed3 242-c6.bin:da07b6a2 242-c7.bin:f6d7a38a 242-c8.bin:c823e045
      242-v1.bin:b9ea8051 242-v2.bin:cc11106e 242-v3.bin:044ea4e1 242-v4.bin:7985ea30"
BIOS="neo-po.bin:16d0c132 sfix.sfix:c2ea0cfd sm1.sm1:94416d67 000-lo.lo:5a86cff2"
crc() { python3 -c 'import sys,zlib; print("%08x" % (zlib.crc32(open(sys.argv[1],"rb").read()) & 0xffffffff))' "$1" 2>/dev/null \
        || gzip -c "$1" | tail -c8 | od -An -tx4 -N4 | tr -d ' \n'; }
missing_of() { for e in $1; do f="${e%%:*}"; c="${e##*:}"; [ -f "$ROMS/$f" ] && [ "$(crc "$ROMS/$f")" = "$c" ] || echo "$f"; done; }
find_zip() {
    [ -n "$2" ] && { echo "$2"; return; }
    for d in "$ROOT" "$ROMS" "$HOME/Downloads"; do [ -f "$d/$1" ] && { echo "$d/$1"; return; }; done
    printf "  Where is your %s? (path, or Enter to skip) " "$1" >&2; read -r p; echo "$p"
}
mkdir -p "$ROMS"
if [ -n "$(missing_of "$GAME")" ]; then
    z="$(find_zip kof98h.zip "$ROM_ZIP")"
    if [ -f "$z" ]; then
        run unzip -o -j "$z" $(missing_of "$GAME") -d "$ROMS"
        # a non-merged set carries the system ROMs too
        run unzip -o -j "$z" $(missing_of "$BIOS") -d "$ROMS"
    fi
fi
if [ -n "$(missing_of "$BIOS")" ]; then
    z="$(find_zip neogeo.zip "$BIOS_ZIP")"; [ -f "$z" ] && run unzip -o -j "$z" $(missing_of "$BIOS") -d "$ROMS"
fi
gone="$(missing_of "$GAME") $(missing_of "$BIOS")"
gone="$(echo $gone)"
[ -z "$gone" ] || fail "these ROM files are missing or don't match the MAME sets: $gone. Point setup.sh at your kof98h.zip and neogeo.zip."
say "  all 20 ROM files present and verified"

step "Building (the recompiler runs on your ROMs; a few minutes)"
run cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DKOF98_ROM_DIR="$ROMS" || fail "CMake could not configure the build."
run cmake --build "$BUILD" --parallel || fail "the build failed."
say "  built"

if [ "$PROFILE" = 1 ]; then
    step "Profiling (finds code only seen at runtime; about a minute)"
    run cmake --build "$BUILD" --target profile || fail "the profiling run failed."
    run cmake --build "$BUILD" --parallel || fail "the rebuild after profiling failed."
    say "  profiled and rebuilt"
fi

step "Creating the launcher"
cat > "$ROOT/kof98.sh" <<EOF
#!/bin/sh
exec "$BUILD/kof98" --rom-path "$ROMS" "\$@"
EOF
chmod +x "$ROOT/kof98.sh"
echo
echo "Done. Run ./kof98.sh to play."
echo "Keys: arrows move, Z/X/C/V buttons A-D, 1 start (no coin: it runs as an AES), Esc quit. No sound yet."
