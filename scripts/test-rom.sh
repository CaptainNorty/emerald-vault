#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash scripts/build-rom.sh
exec /usr/bin/python3 tools/emulator/test_rom.py "$@"
