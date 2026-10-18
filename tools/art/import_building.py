#!/usr/bin/env python3
"""Import a hand-drawn farm building into Littleroot's secondary tileset.

    python3 tools/art/import_building.py art/buildings/1_delivery_service.png \
        --palette 7 --name sDeliveryServiceTiles

The drawing is split into 8x8 tiles that go on the metatiles' top layer, over
the game's own grass on the bottom layer, so grass-colored pixels become
transparent and cost no palette colors. Tiles and metatiles that already exist
(including mirrored tiles) are reused, so running it again adds nothing.

Writes tiles.png, metatiles.bin, metatile_attributes.bin and the palette, then
prints the map entries (metatile plus collision) for src/data/vault_town_buildings.h.
"""

import argparse
import struct
import sys
from pathlib import Path

from PIL import Image

TILESET = Path("data/tilesets/secondary/petalburg")
NUM_TILES_IN_PRIMARY = 512
NUM_METATILES_IN_PRIMARY = 512
MAX_SECONDARY_TILES = 512
MAX_SECONDARY_METATILES = 512
FIRST_FREE_PALETTE = 6
LAST_PALETTE = 12

# Littleroot's grass (primary metatile 1), drawn on every bottom layer.
GRASS_COLORS = {(115, 197, 164), (65, 180, 131), (164, 213, 197)}
GRASS_LAYER = (0x2002, 0x2003, 0x2003, 0x2002)
EMPTY_TILE = 0  # Primary tile 0 is fully transparent.
COLLISION_IMPASSABLE = 1 << 10
# Bottom and middle background layers, so object events (the terminals, the
# player) always draw on top of the building.
METATILE_ATTRIBUTE_COVERED = 1 << 12


def gba_color(rgb):
    """Round to the GBA's 15-bit color, the way gbagfx does."""
    return tuple((c >> 3) << 3 for c in rgb)


def flips(tile):
    """Yield (pixels, hflip, vflip) for each way the tile can be stored."""
    rows = [tile[y * 8:(y + 1) * 8] for y in range(8)]
    for h in (0, 1):
        for v in (0, 1):
            r = rows[::-1] if v else rows
            r = [row[::-1] for row in r] if h else r
            yield tuple(p for row in r for p in row), h, v


def read_tiles(img):
    cols = img.width // 8
    tiles = []
    for i in range(cols * (img.height // 8)):
        x, y = (i % cols) * 8, (i // cols) * 8
        tiles.append(tuple(img.getpixel((x + dx, y + dy)) for dy in range(8) for dx in range(8)))
    return tiles


def write_tiles(path, tiles, palette):
    cols = 16
    rows = (len(tiles) + cols - 1) // cols
    img = Image.new("P", (cols * 8, rows * 8), 0)
    img.putpalette(palette)
    for i, tile in enumerate(tiles):
        x, y = (i % cols) * 8, (i // cols) * 8
        for p, value in enumerate(tile):
            img.putpixel((x + p % 8, y + p // 8), value)
    img.save(path)


def write_palette(path, colors):
    colors = [(0, 0, 0)] + colors + [(0, 0, 0)] * (15 - len(colors))
    path.write_text("JASC-PAL\r\n0100\r\n16\r\n" + "".join(f"{r} {g} {b}\r\n" for r, g, b in colors))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("png")
    parser.add_argument("--palette", type=int, required=True, help=f"BG palette slot, {FIRST_FREE_PALETTE}-{LAST_PALETTE}")
    parser.add_argument("--name", required=True, help="C array name to print")
    args = parser.parse_args()

    art = Image.open(args.png).convert("RGBA")
    if art.width % 16 or art.height % 16:
        sys.exit(f"{args.png} is {art.width}x{art.height}; both sides must be multiples of 16")
    if not FIRST_FREE_PALETTE <= args.palette <= LAST_PALETTE:
        sys.exit(f"--palette must be {FIRST_FREE_PALETTE}-{LAST_PALETTE}")

    # Palette: index 0 is transparent (grass shows through).
    pixels = []
    counts = {}
    for r, g, b, a in art.getdata():
        if a < 255 or (r, g, b) in GRASS_COLORS:
            pixels.append(None)
            continue
        color = gba_color((r, g, b))
        pixels.append(color)
        counts[color] = counts.get(color, 0) + 1
    colors = sorted(counts, key=lambda c: -counts[c])
    if len(colors) > 15:
        sys.exit(f"{len(colors)} colors (not counting grass); a palette holds 15")
    index = {c: i + 1 for i, c in enumerate(colors)}

    tiles_img = Image.open(TILESET / "tiles.png")
    tile_palette = tiles_img.getpalette()[:48]
    tiles = read_tiles(tiles_img)
    while tiles and not any(tiles[-1]):
        tiles.pop()
    known = {}
    for i, tile in enumerate(tiles):
        for pix, h, v in flips(tile):
            known.setdefault(pix, (i, h, v))
    tiles_before = len(tiles)

    metatiles_bin = bytearray((TILESET / "metatiles.bin").read_bytes())
    attributes_bin = bytearray((TILESET / "metatile_attributes.bin").read_bytes())
    metatile_ids = {bytes(metatiles_bin[i:i + 16]): NUM_METATILES_IN_PRIMARY + i // 16
                    for i in range(0, len(metatiles_bin), 16)}
    metatiles_before = len(metatile_ids)

    def tile_entry(tx, ty):
        tile = tuple(0 if pixels[(ty + dy) * art.width + tx + dx] is None
                     else index[pixels[(ty + dy) * art.width + tx + dx]]
                     for dy in range(8) for dx in range(8))
        if not any(tile):
            return EMPTY_TILE
        if tile not in known:
            known[tile] = (len(tiles), 0, 0)
            tiles.append(tile)
        i, h, v = known[tile]
        return (NUM_TILES_IN_PRIMARY + i) | (h << 10) | (v << 11) | (args.palette << 12)

    entries = []
    for my in range(0, art.height, 16):
        row = []
        for mx in range(0, art.width, 16):
            top = [tile_entry(mx + dx, my + dy) for dy in (0, 8) for dx in (0, 8)]
            data = struct.pack("<8H", *GRASS_LAYER, *top)
            if data not in metatile_ids:
                metatile_ids[data] = NUM_METATILES_IN_PRIMARY + len(metatiles_bin) // 16
                metatiles_bin += data
                attributes_bin += bytes(2)
            offset = (metatile_ids[data] - NUM_METATILES_IN_PRIMARY) * 2
            attributes_bin[offset:offset + 2] = struct.pack("<H", METATILE_ATTRIBUTE_COVERED)
            row.append(metatile_ids[data] | COLLISION_IMPASSABLE)
        entries.append(row)

    if len(tiles) > MAX_SECONDARY_TILES:
        sys.exit(f"Out of tiles: {len(tiles)} of {MAX_SECONDARY_TILES}")
    if len(metatiles_bin) // 16 > MAX_SECONDARY_METATILES:
        sys.exit(f"Out of metatiles: {len(metatiles_bin) // 16} of {MAX_SECONDARY_METATILES}")

    write_tiles(TILESET / "tiles.png", tiles, tile_palette)
    (TILESET / "metatiles.bin").write_bytes(metatiles_bin)
    (TILESET / "metatile_attributes.bin").write_bytes(attributes_bin)
    write_palette(TILESET / "palettes" / f"{args.palette:02}.pal", colors)

    print(f"// {len(colors)} colors in palette {args.palette}; "
          f"{len(tiles) - tiles_before} new tiles ({len(tiles)} of {MAX_SECONDARY_TILES} used); "
          f"{len(metatile_ids) - metatiles_before} new metatiles", file=sys.stderr)
    print(f"static const u16 {args.name}[] =\n{{")
    for row in entries:
        print("    " + ", ".join(f"0x{e:04X}" for e in row) + ",")
    print("};")


if __name__ == "__main__":
    main()
