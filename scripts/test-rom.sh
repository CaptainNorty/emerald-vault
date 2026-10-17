#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash scripts/build-rom.sh
# Test-only ROM that boots straight into Littleroot Town (separate build dir).
bash scripts/build-rom.sh TEST_START=1
# Keep the emulator runner up to date with tools/emulator/runner.c.
PATH="/opt/homebrew/bin:$PATH" cmake --build build/automation/native --target emerald-emulator > /dev/null
exec /usr/bin/python3 tools/emulator/test_rom.py "$@"
