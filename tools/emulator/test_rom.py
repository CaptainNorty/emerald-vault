#!/usr/bin/env python3
"""Run frame-controlled scenarios against a fresh ROM using the mGBA core."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import select
import struct
from collections import deque
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
KEYS = {name: 1 << bit for bit, name in enumerate(
    ["A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN", "R", "L"])}
DIRECTIONS = {"UP": (0, -1), "DOWN": (0, 1), "LEFT": (-1, 0), "RIGHT": (1, 0)}
# Built ROMs: "normal" is the real game; "test_start" (make TEST_START=1) boots
# straight into Littleroot Town.
BUILDS = {"normal": "pokeemerald", "test_start": "pokeemerald_teststart"}
# Starting setups for the test-only ROM: TEST_PRESET_* in include/constants/vault_town.h.
PRESETS = {"new_game": 0, "no_flying_type": 1, "has_flying_type": 2, "oldale_no_balls": 3}
MAP_OFFSET = 7  # include/fieldmap.h: object coordinates include this border.


class MapGrid:
    def __init__(self, width, cells, triggers):
        self.width, self.cells, self.triggers = width, cells, triggers
        self.height = len(cells) // width

    def passable(self, x, y):
        return (0 <= x < self.width and 0 <= y < self.height
                and not self.cells[y * self.width + x] & 0x0C00)

    def route(self, start, target, blocked):
        if not self.passable(*target):
            raise ValueError(f"{target} is not walkable")
        prev = {start: None}
        queue = deque([start])
        while queue:
            point = queue.popleft()
            if point == target:
                break
            for name, (dx, dy) in DIRECTIONS.items():
                nxt = (point[0] + dx, point[1] + dy)
                if (nxt not in prev and self.passable(*nxt) and nxt not in blocked
                        and (nxt not in self.triggers or nxt == target)):
                    prev[nxt] = (point, name)
                    queue.append(nxt)
        if target not in prev:
            raise AssertionError(f"No route from {start} to {target}")
        path, point = [], target
        while prev[point]:
            point, name = prev[point]
            path.append(name)
        return path[::-1]


class Maps:
    # Map layouts from data/maps/*/map.json and data/layouts/layouts.json.
    def __init__(self, map_ids):
        self.by_id = {ids: name for name, ids in map_ids.items()}
        self.files = {}
        for path in (ROOT / "data/maps").glob("*/map.json"):
            data = json.loads(path.read_text())
            self.files[data["id"]] = data
        self.layouts = {layout["id"]: layout for layout in
                        json.loads((ROOT / "data/layouts/layouts.json").read_text())["layouts"]
                        if "id" in layout}

    def load(self, ids):
        data = self.files[self.by_id[ids]]
        layout = self.layouts[data["layout"]]
        raw = (ROOT / layout["blockdata_filepath"]).read_bytes()
        cells = struct.unpack(f"<{len(raw) // 2}H", raw)
        triggers = {(event["x"], event["y"]) for event in data.get("coord_events", [])}
        return MapGrid(layout["width"], cells, triggers)


def load_map_ids():
    # include/constants/map_groups.h: MAP_NAME = (num | (group << 8)).
    text = (ROOT / "include/constants/map_groups.h").read_text()
    return {name: (int(group), int(num)) for name, num, group in
            re.findall(r"(MAP_\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)", text)}


class Emulator:
    def __init__(self, rom, output, symbols):
        self.symbols = symbols
        self.output = output
        self.frame = 0
        self.log = (output / "emulator.log").open("w")
        self.proc = subprocess.Popen(
            [str(ROOT / "build/automation/native/emerald-emulator"), str(rom)],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self.log,
            cwd=output, text=True, bufsize=1)
        try:
            if self.receive() != "READY":
                raise RuntimeError("Emulator failed to initialize; see emulator.log")
        except BaseException:
            self.close()
            raise

    def receive(self):
        if not select.select([self.proc.stdout], [], [], 15)[0]:
            raise TimeoutError("Emulator did not respond within 15 seconds")
        response = self.proc.stdout.readline().strip()
        if not response or response.startswith("ERROR"):
            raise RuntimeError(response or "Emulator exited; see emulator.log")
        return response

    def command(self, command):
        self.proc.stdin.write(command + "\n")
        self.proc.stdin.flush()
        return self.receive()

    def run(self, frames, keys=()):
        if not isinstance(frames, int) or not 0 <= frames <= 36000:
            raise ValueError("frames must be an integer from 0 to 36000")
        mask = 0
        for key in keys:
            mask |= KEYS[key.upper()]
        self.frame = int(self.command(f"run {frames} {mask}").split()[1])

    def read(self, address, bits=32):
        return int(self.command(f"read {bits} {address:x}").split()[1])

    def write(self, address, value, bits=8):
        self.command(f"write {bits} {address:x} {value}")

    def money(self):
        # gSaveBlock1Ptr->money (+0x490) is XORed with gSaveBlock2Ptr->encryptionKey (+0xAC).
        save1 = self.read(self.symbols["gSaveBlock1Ptr"])
        save2 = self.read(self.symbols["gSaveBlock2Ptr"])
        return self.read(save1 + 0x490) ^ self.read(save2 + 0xAC)

    def choose_test_preset(self, preset):
        # The test-only ROM waits briefly at boot (src/test_start.c) for these writes.
        waiting = self.symbols["CB2_TestStartWaitForHarness"] & ~1
        for _ in range(600):
            if self.read(self.symbols["gMain"] + 4) & ~1 == waiting:
                self.write(self.symbols["gTestStartPreset"], preset)
                self.write(self.symbols["gTestStartHarnessReady"], 1)
                return
            self.run(1)
        raise AssertionError("Test-only ROM never waited for the harness at boot")

    def task_active(self, name):
        # include/task.h: 16 entries, 40-byte Task, function at +0, isActive at +4.
        expected = self.symbols[name] & ~1
        for index in range(16):
            address = self.symbols["gTasks"] + index * 40
            if self.read(address + 4, 8) and self.read(address) & ~1 == expected:
                return True
        return False

    def overworld_ready(self):
        # Overworld main loop running, no fade or script holding the player.
        main = self.symbols["gMain"]
        return (self.read(main) & ~1 == self.symbols["CB1_Overworld"] & ~1
                and self.read(main + 4) & ~1 == self.symbols["CB2_Overworld"] & ~1
                and not self.read(self.symbols["sLockFieldControls"], 8))

    def player_position(self):
        # gPlayerAvatar.objectEventId at +5; ObjectEvent is 0x24 bytes with
        # currentCoords (two s16) at +0x10.
        object_id = self.read(self.symbols["gPlayerAvatar"] + 5, 8)
        coords = self.symbols["gObjectEvents"] + object_id * 0x24 + 0x10
        x, y = (self.read(coords + offset, 16) for offset in (0, 2))
        return (x - 0x10000 if x & 0x8000 else x) - MAP_OFFSET, (y - 0x10000 if y & 0x8000 else y) - MAP_OFFSET

    def current_map(self):
        # gSaveBlock1Ptr->location: mapGroup at +4, mapNum at +5.
        save = self.read(self.symbols["gSaveBlock1Ptr"])
        return self.read(save + 4, 8), self.read(save + 5, 8)

    def player_settled(self):
        # gPlayerAvatar.runningState (+2) and tileTransitionState (+3) are 0 when still.
        avatar = self.symbols["gPlayerAvatar"]
        return not self.read(avatar + 2, 8) and not self.read(avatar + 3, 8)

    def walk(self, direction, tiles, timeout_frames=60):
        dx, dy = DIRECTIONS[direction]
        for _ in range(tiles):
            start = self.player_position()
            start_map = self.current_map()
            target = (start[0] + dx, start[1] + dy)
            waited = 0
            # Stepping off the map's edge onto a connected map also counts as a step.
            while self.player_position() != target and self.current_map() == start_map:
                if waited >= timeout_frames:
                    self.run(0)
                    raise AssertionError(
                        f"Blocked walking {direction} from {start}; still at {self.player_position()}")
                self.run(2, [direction])
                waited += 2
        self.run(1)  # Release the button so the final step doesn't chain into another.
        waited = 0
        while not (self.player_settled() and self.overworld_ready()):
            if waited >= timeout_frames:
                raise AssertionError(f"Player did not stop after walking {direction}")
            self.run(2)
            waited += 2

    def text_settled(self):
        # include/text.h: 32 TextPrinters of 0x24 bytes; active at +0x1B, state at
        # +0x1C. Settled when no printer is mid-message: each is idle or waiting for
        # a button (RENDER_STATE_WAIT, _CLEAR or _SCROLL_START, values 1 to 3).
        for index in range(32):
            base = self.symbols["sTextPrinters"] + index * 0x24
            if self.read(base + 0x1B, 8) and self.read(base + 0x1C, 8) not in (1, 2, 3):
                return False
        return True

    def wait_text(self, timeout_frames=600):
        # Settled for 8 frames in a row, so a message that is only just starting
        # to print doesn't count as finished.
        waited = steady = 0
        while steady < 4:
            if waited >= timeout_frames:
                raise AssertionError(f"Text was still printing after {timeout_frames} frames")
            steady = steady + 1 if self.text_settled() else 0
            self.run(2)
            waited += 2

    def object_positions(self):
        # Active object events other than the player. ObjectEvent: active is bit 0 at +0.
        player = self.read(self.symbols["gPlayerAvatar"] + 5, 8)
        positions = set()
        for index in range(16):
            base = self.symbols["gObjectEvents"] + index * 0x24
            if index == player or not self.read(base, 8) & 1:
                continue
            x, y = (self.read(base + 0x10 + offset, 16) for offset in (0, 2))
            positions.add((x - MAP_OFFSET, y - MAP_OFFSET))
        return positions

    def walk_to(self, target, maps, attempts=8):
        # Plan on the current map, avoiding walls, NPCs where they stand now and
        # trigger tiles; replan if a wandering NPC gets in the way.
        for _ in range(attempts):
            grid = maps.load(self.current_map())
            path = grid.route(self.player_position(), target, self.object_positions())
            try:
                for direction in path:
                    self.walk(direction, 1, timeout_frames=40)
                if self.player_position() == target:
                    return
            except AssertionError:
                self.run(10)
        raise AssertionError(f"Could not walk to {target}; stuck at {self.player_position()}")

    def screenshot(self, name):
        if not name or any(c not in "abcdefghijklmnopqrstuvwxyz0123456789-_" for c in name):
            raise ValueError("Screenshot names must contain lowercase letters, digits, - or _")
        path = self.output / (name + ".png")
        self.command("screenshot " + str(path))
        if not path.is_file() or path.read_bytes()[:8] != b"\x89PNG\r\n\x1a\n":
            raise RuntimeError("Emulator did not produce a PNG screenshot")
        return path.name

    def close(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
        self.proc.stdin.close()
        self.proc.stdout.close()
        self.log.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scenario", type=Path, default=ROOT / "tests/emulator/boot.json")
    args = parser.parse_args()
    scenario = json.loads(args.scenario.read_text())
    executable = ROOT / "build/automation/native/emerald-emulator"
    if not executable.is_file():
        parser.error("Run bash scripts/setup-emulator.sh first")
    build = scenario.get("build", "normal")
    if build not in BUILDS:
        parser.error(f"Unknown build {build!r}; expected one of {', '.join(BUILDS)}")
    preset = scenario.get("preset")
    if preset is not None and (build != "test_start" or preset not in PRESETS):
        parser.error(f"preset needs the test_start build and one of {', '.join(PRESETS)}")
    rom = ROOT / (BUILDS[build] + ".gba")
    elf = ROOT / (BUILDS[build] + ".elf")
    if not rom.is_file() or not elf.is_file():
        parser.error("Run bash scripts/test-rom.sh to build the ROMs first")
    map_ids = load_map_ids()
    maps = Maps(map_ids)
    devkitarm = Path(os.environ.get("DEVKITARM", "/opt/devkitpro/devkitARM"))
    nm = subprocess.check_output([str(devkitarm / "bin/arm-none-eabi-nm"), "-n", str(elf)], text=True)
    symbols = {}
    for line in nm.splitlines():
        fields = line.split()
        if len(fields) == 3:
            symbols[fields[2]] = int(fields[0], 16)
    runs = ROOT / "build/test-runs"
    runs.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix=time.strftime("%Y%m%d-%H%M%S-"), dir=runs))
    test_rom = output / "test.gba"
    shutil.copyfile(rom, test_rom)
    shutil.copyfile(elf, output / "test.elf")
    (output / "scenario.json").write_text(json.dumps(scenario, indent=2) + "\n")
    result = {"status": "FAIL", "scenario": scenario["name"], "build": build, "emulator": "mGBA 0.10.5",
              "rom_sha256": hashlib.sha256(test_rom.read_bytes()).hexdigest(),
              "checks": [], "screenshots": []}
    emulator = None
    started = time.monotonic()
    print(f"Test artifacts: {output}", flush=True)
    try:
        emulator = Emulator(test_rom, output, symbols)
        if preset is not None:
            emulator.choose_test_preset(PRESETS[preset])
            result["preset"] = preset
        saved_money = None
        for step in scenario["steps"]:
            action = step["action"]
            if action == "run":
                emulator.run(step["frames"], step.get("keys", []))
            elif action == "wait_task":
                task = step["task"]
                limit = step.get("timeout_frames", 1800)
                if not isinstance(limit, int) or not 1 <= limit <= 36000:
                    raise ValueError("timeout_frames must be from 1 to 36000")
                waited = 0
                while not emulator.task_active(task):
                    if waited >= limit:
                        raise AssertionError(f"{task} not reached within {limit} frames")
                    count = min(10, limit - waited)
                    emulator.run(count)
                    waited += count
                result["checks"].append({"task": task, "frame": emulator.frame})
                print(f"PASS: {task} at frame {emulator.frame}", flush=True)
            elif action == "assert_task":
                if not emulator.task_active(step["task"]):
                    raise AssertionError(f"Task not active: {step['task']}")
                result["checks"].append({"task": step["task"], "frame": emulator.frame})
            elif action == "wait_overworld":
                limit = step.get("timeout_frames", 1800)
                if not isinstance(limit, int) or not 1 <= limit <= 36000:
                    raise ValueError("timeout_frames must be from 1 to 36000")
                waited = 0
                while not emulator.overworld_ready():
                    if waited >= limit:
                        raise AssertionError(f"Overworld not ready within {limit} frames")
                    count = min(10, limit - waited)
                    emulator.run(count)
                    waited += count
                result["checks"].append({"overworld": True, "frame": emulator.frame})
                print(f"PASS: overworld ready at frame {emulator.frame}", flush=True)
            elif action == "walk":
                for move in step["path"]:
                    direction, _, count = move.upper().partition(" ")
                    tiles = int(count or 1)
                    if direction not in DIRECTIONS or not 1 <= tiles <= 100:
                        raise ValueError(f"Invalid walk move {move!r}; use e.g. \"DOWN 3\"")
                    emulator.walk(direction, tiles)
                result["checks"].append({"walked": step["path"], "position": emulator.player_position(),
                                         "frame": emulator.frame})
            elif action == "dismiss_text":
                # Press A every 30 frames until the message closes and the player can move.
                limit = step.get("timeout_frames", 1200)
                if not isinstance(limit, int) or not 1 <= limit <= 36000:
                    raise ValueError("timeout_frames must be from 1 to 36000")
                waited = 0
                while not emulator.overworld_ready():
                    if waited >= limit:
                        raise AssertionError(f"Text not dismissed within {limit} frames")
                    emulator.run(2, ["A"])
                    emulator.run(28)
                    waited += 30
                result["checks"].append({"text_dismissed": True, "frame": emulator.frame})
            elif action == "assert_blocked":
                direction = step["direction"].upper()
                if direction not in DIRECTIONS:
                    raise ValueError(f"Invalid direction {direction!r}")
                start = emulator.player_position()
                emulator.run(40, [direction])
                emulator.run(20)
                if emulator.player_position() != start:
                    raise AssertionError(f"Walked {direction} from {start} to {emulator.player_position()}; expected a wall")
                result["checks"].append({"blocked": direction, "position": start, "frame": emulator.frame})
                print(f"PASS: blocked moving {direction} at {start}", flush=True)
            elif action == "walk_to":
                emulator.walk_to((step["x"], step["y"]), maps)
                result["checks"].append({"walked_to": emulator.player_position(), "frame": emulator.frame})
            elif action == "next_message":
                # Wait for the current text to finish, press A, and wait for the next
                # text to finish; `times` messages in a row. `wait` adds frames after
                # each press for fades or fanfares.
                for _ in range(step.get("times", 1)):
                    emulator.wait_text()
                    emulator.run(2, ["A"])
                    emulator.run(6 + step.get("wait", 0))
                emulator.wait_text()
            elif action == "press":
                # Press a button `times` times, waiting `wait` frames after each press.
                for _ in range(step.get("times", 1)):
                    emulator.run(2, [step["key"]])
                    emulator.run(step.get("wait", 30))
            elif action == "press_until_task":
                # Keep pressing a button (default A) until a game task is active.
                limit, waited = step.get("timeout_frames", 1200), 0
                while not emulator.task_active(step["task"]):
                    if waited >= limit:
                        raise AssertionError(f"{step['task']} not reached within {limit} frames")
                    emulator.run(2, [step.get("key", "A")])
                    emulator.run(28)
                    waited += 30
                result["checks"].append({"task": step["task"], "frame": emulator.frame})
            elif action == "remember_money":
                saved_money = emulator.money()
                result["checks"].append({"money": saved_money, "frame": emulator.frame})
                print(f"Money: ${saved_money}", flush=True)
            elif action == "assert_money_gained":
                if saved_money is None:
                    raise ValueError("assert_money_gained needs an earlier remember_money")
                gained = emulator.money() - saved_money
                low, high = step.get("min", 1), step.get("max")
                if gained < low or (high is not None and gained > high):
                    raise AssertionError(f"Gained ${gained}; expected ${low} to ${high if high is not None else 'any'}")
                result["checks"].append({"money_gained": gained, "frame": emulator.frame})
                print(f"PASS: gained ${gained}", flush=True)
            elif action == "assert_position":
                expected = (step["x"], step["y"])
                actual = emulator.player_position()
                if actual != expected:
                    raise AssertionError(f"Player at {actual}, expected {expected}")
                if "map" in step:
                    if step["map"] not in map_ids:
                        raise ValueError(f"Unknown map {step['map']}")
                    if emulator.current_map() != map_ids[step["map"]]:
                        raise AssertionError(f"Player on map {emulator.current_map()}, expected {step['map']}")
                result["checks"].append({"position": actual, "map": step.get("map"), "frame": emulator.frame})
                print(f"PASS: player at {actual}", flush=True)
            elif action == "screenshot":
                result["screenshots"].append(emulator.screenshot(step["name"]))
            else:
                raise ValueError(f"Unknown action: {action}")
        emulator.command("quit")
        if emulator.proc.wait(timeout=3) != 0:
            raise RuntimeError("Emulator exited with an error")
        result["status"] = "PASS"
    except Exception as error:
        result["error"] = str(error)
        print(f"FAIL: {error}", file=sys.stderr)
        if emulator and emulator.proc.poll() is None and not isinstance(error, TimeoutError):
            try:
                result["screenshots"].append(emulator.screenshot("failure"))
            except Exception:
                pass
    finally:
        if emulator:
            emulator.close()
        result["elapsed_seconds"] = round(time.monotonic() - started, 2)
        (output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(f"{result['status']}: {output / 'result.json'}")
    return 0 if result["status"] == "PASS" else 1


if __name__ == "__main__":
    sys.exit(main())
