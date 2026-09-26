#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
export PATH="/opt/homebrew/bin:$PATH"
command -v cmake >/dev/null || { echo 'Run: brew install cmake' >&2; exit 1; }
version=0.10.5
checksum=91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14
base="$PWD/build/automation"
archive="$base/vendor/mgba-$version.tar.gz"
mkdir -p "$base/vendor"
if [ ! -f "$archive" ]; then
    curl -fL --retry 2 "https://github.com/mgba-emu/mgba/archive/refs/tags/$version.tar.gz" -o "$archive.download"
    mv "$archive.download" "$archive"
fi
printf '%s  %s\n' "$checksum" "$archive" | shasum -a 256 -c -
if [ ! -d "$base/vendor/mgba-$version" ]; then
    tar -xzf "$archive" -C "$base/vendor"
fi
cmake -S tools/emulator -B "$base/native" \
    -DMGBA_SOURCE="$base/vendor/mgba-$version" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew
cmake --build "$base/native" --target emerald-emulator -j "${JOBS:-8}"
echo 'Ready. Run: bash scripts/test-rom.sh'
