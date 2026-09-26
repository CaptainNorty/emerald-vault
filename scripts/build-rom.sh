#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="/opt/homebrew/bin:$DEVKITARM/bin:$PATH"
pkg-config --exists libpng || { echo 'Install Homebrew libpng and pkgconf first.' >&2; exit 1; }
# Build helpers first so a helper failure stops before dependency scanning.
make tools
make -j"${JOBS:-8}" "$@"
