#!/bin/sh
# Builds the bigmaps module for the Linux x86 (32-bit) dedicated server:
#   work/module/bigmaps/linux/gmsv_bigmaps_linux.dll   (GMod loads Linux binary modules with a .dll name)
# Needs g++ with 32-bit multilib (Ubuntu: apt install g++-multilib). From Windows, run from the repository root:
#   wsl -d Ubuntu-20.04 -- sh tools/build-module-linux.sh
# Any distribution with an equal or older glibc than the server (here 2.31, server 2.36) is fine.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT="$ROOT/work/module/bigmaps/linux"
mkdir -p "$OUT"
g++ -m32 -std=c++14 -O2 -Wall -Wextra -Werror -fPIC -shared -fvisibility=hidden -fno-exceptions -fno-rtti \
    -static-libgcc -Wl,--no-undefined -Wl,-z,defs \
    "$ROOT/module/bigmaps/bigmaps.cpp" -o "$OUT/gmsv_bigmaps_linux.dll" -ldl
{
    file "$OUT/gmsv_bigmaps_linux.dll"
    echo "--- exported symbols"
    nm -D --defined-only "$OUT/gmsv_bigmaps_linux.dll"
    echo "--- needed libraries"
    readelf -d "$OUT/gmsv_bigmaps_linux.dll" | grep NEEDED
    echo "--- glibc symbol versions"
    objdump -T "$OUT/gmsv_bigmaps_linux.dll" | grep -o 'GLIBC_[0-9.]*' | sort -u -V
    echo "--- sha256"
    sha256sum "$OUT/gmsv_bigmaps_linux.dll"
} > "$OUT/inspection.txt"
cat "$OUT/inspection.txt"
echo "Linux module build OK: $OUT"
