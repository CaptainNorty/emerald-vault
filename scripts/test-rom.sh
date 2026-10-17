#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash scripts/build-rom.sh
# Test-only ROM that boots straight into Littleroot Town (separate build dir).
bash scripts/build-rom.sh TEST_START=1
exec /usr/bin/python3 tools/emulator/test_rom.py "$@"
