"""Build art/building_kit/: templates, references and palettes for drawing the
Littleroot farm buildings in the existing Gen 3 style.

Usage: python3 tools/art/make_building_kit.py
"""
import colorsys
import json
import re
import struct
import zipfile
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "art/building_kit"
LITTLEROOT_WIDTH = 60

# (file name, title, plot x, y, width, height in 16px map squares, terminal x, y)
# Plots match the ruins in data/layouts/LittlerootTown/map.bin.
BUILDINGS = [
    ("1_delivery_service", "Delivery Service", 24, 4, 6, 5, 27, 9),
    ("2_greenhouse", "Greenhouse", 49, 3, 7, 7, 55, 10),
    ("3_quarry", "Quarry", 49, 27, 7, 7, 55, 34),
    ("4_fishery", "Fishery", 23, 27, 7, 7, 29, 34),
    ("5_generator", "Generator", 50, 16, 5, 5, 54, 21),
    ("6_forge", "Forge", 36, 4, 7, 6, 42, 10),
    ("7_spirit_sanctum", "Spirit Sanctum", 36, 28, 7, 6, 42, 34),
    ("8_ice_dome", "Ice Dome", 23, 16, 7, 6, 29, 22),
    ("9_weather_stabilizer", "Weather Stabilizer", 34, 14, 11, 9, None, None),
]

# Existing buildings to match: (file name, layout, x, y, width, height)
BUILDING_REFERENCES = [
    ("brendans_house", "LittlerootTown", 2, 4, 5, 5),
    ("birchs_lab", "LittlerootTown", 3, 12, 7, 5),
    ("pokemon_center", "OldaleTown", 5, 13, 4, 4),
    ("poke_mart", "OldaleTown", 13, 3, 4, 4),
    ("oldale_house", "OldaleTown", 4, 4, 4, 4),
]

TOWN_REFERENCES = ["PetalburgCity", "RustboroCity", "SlateportCity", "MauvilleCity", "FortreeCity",
                   "LilycoveCity", "MossdeepCity", "FallarborTown", "LavaridgeTown", "DewfordTown"]

GRASS = 0x001
LITTLEROOT_PALETTES = [0, 1, 2, 3, 4, 5, 6, 8, 9, 10]  # Slots 7, 11 and 12 are free for new art


def snake(name):
    return re.sub(r"(?<!^)(?=[A-Z])", "_", name).lower()


def tileset_dir(symbol):
    name = snake(symbol.replace("gTileset_", ""))
    for kind in ("primary", "secondary"):
        if (ROOT / "data/tilesets" / kind / name).is_dir():
            return ROOT / "data/tilesets" / kind / name
    raise FileNotFoundError(symbol)


def load_palette(path):
    lines = path.read_text().split("\n")[3:19]
    return [tuple(int(v) for v in line.split()) for line in lines]


def load_tileset(path):
    img = Image.open(path / "tiles.png")
    px = img.load()
    tiles = [[[px[tx * 8 + x, ty * 8 + y] & 0xF for x in range(8)] for y in range(8)]
             for ty in range(img.size[1] // 8) for tx in range(img.size[0] // 8)]
    palettes = [load_palette(path / f"palettes/{i:02d}.pal") for i in range(16)]
    data = (path / "metatiles.bin").read_bytes()
    metatiles = [struct.unpack("<8H", data[i:i + 16]) for i in range(0, len(data), 16)]
    return tiles, palettes, metatiles


class Renderer:
    def __init__(self, primary, secondary):
        ptiles, ppals, pmetas = load_tileset(primary)
        stiles, spals, smetas = load_tileset(secondary)
        self.palettes = ppals[:6] + spals[6:13] + ppals[13:]
        self.tiles = (ptiles + [[[0] * 8] * 8] * 512)[:512] + stiles
        self.metatiles = (pmetas + [(0,) * 8] * 512)[:512] + smetas

    def render(self, cells, width):
        height = len(cells) // width
        img = Image.new("RGB", (width * 16, height * 16))
        px = img.load()
        for my in range(height):
            for mx in range(width):
                metatile = self.metatiles[cells[my * width + mx] & 0x3FF]
                for layer in range(2):
                    for i in range(4):
                        entry = metatile[layer * 4 + i]
                        tile = self.tiles[entry & 0x3FF] if (entry & 0x3FF) < len(self.tiles) else [[0] * 8] * 8
                        hflip, vflip, palette = entry & 0x400, entry & 0x800, self.palettes[entry >> 12]
                        ox, oy = mx * 16 + (i % 2) * 8, my * 16 + (i // 2) * 8
                        for y in range(8):
                            for x in range(8):
                                c = tile[7 - y if vflip else y][7 - x if hflip else x]
                                if c == 0 and layer == 1:
                                    continue
                                px[ox + x, oy + y] = palette[c]
        return img


def layout(name):
    layouts = json.loads((ROOT / "data/layouts/layouts.json").read_text())["layouts"]
    entry = next(l for l in layouts if l.get("name") == name + "_Layout")
    data = (ROOT / entry["blockdata_filepath"]).read_bytes()
    cells = list(struct.unpack(f"<{len(data) // 2}H", data))
    renderer = Renderer(tileset_dir(entry["primary_tileset"]), tileset_dir(entry["secondary_tileset"]))
    return renderer, cells, entry["width"]


def grid_overlay(width, height):
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    for x in range(0, width, 8):
        draw.line([(x, 0), (x, height - 1)], fill=(255, 0, 0, 255 if x % 16 == 0 else 90))
    for y in range(0, height, 8):
        draw.line([(0, y), (width - 1, y)], fill=(255, 0, 0, 255 if y % 16 == 0 else 90))
    return img


def write_palette_files(palettes):
    folder = OUT / "palettes"
    folder.mkdir(parents=True, exist_ok=True)
    # Swatch sheet: one row per palette, 16x16 per color; index 0 (transparent) crossed out.
    sheet = Image.new("RGB", (16 * 16 + 40, 16 * len(palettes)), (255, 255, 255))
    draw = ImageDraw.Draw(sheet)
    gpl = ["GIMP Palette", "Name: Littleroot Town", "Columns: 15", "#"]
    aco = struct.pack(">HH", 1, 15 * len(palettes))
    procreate = []
    for row, (slot, colors) in enumerate(palettes):
        draw.text((2, row * 16 + 3), f"{slot:2d}", fill=(0, 0, 0))
        for i, color in enumerate(colors):
            x0, y0 = 40 + i * 16, row * 16
            draw.rectangle([x0, y0, x0 + 15, y0 + 15], fill=color)
            if i == 0:
                draw.line([x0, y0, x0 + 15, y0 + 15], fill=(255, 0, 0))
                continue
            gpl.append(f"{color[0]:3d} {color[1]:3d} {color[2]:3d}\tpalette {slot} color {i}")
            aco += struct.pack(">HHHHH", 0, color[0] * 257, color[1] * 257, color[2] * 257, 0)
        swatches = []
        for color in colors[1:]:
            h, s, v = colorsys.rgb_to_hsv(*(c / 255 for c in color))
            swatches.append({"hue": h, "saturation": s, "brightness": v, "alpha": 1, "colorSpace": 0})
        procreate.append((slot, swatches))
    sheet.resize((sheet.width * 2, sheet.height * 2), Image.NEAREST).save(folder / "littleroot_palettes.png")
    (folder / "littleroot.gpl").write_text("\n".join(gpl) + "\n")
    (folder / "littleroot.aco").write_bytes(aco)
    for slot, swatches in procreate:
        with zipfile.ZipFile(folder / f"littleroot_palette_{slot:02d}.swatches", "w") as z:
            z.writestr("Swatches.json", json.dumps([{"name": f"Littleroot {slot}", "swatches": swatches}]))


def main():
    renderer, cells, width = layout("LittlerootTown")
    for kind in ("templates", "context", "reference/buildings", "reference/towns"):
        (OUT / kind).mkdir(parents=True, exist_ok=True)

    # Empty plots: grass, at exact size, plus a grid overlay to put on top.
    farm = list(cells)
    for _, _, px, py, pw, ph, _, _ in BUILDINGS:
        for y in range(py, py + ph):
            for x in range(px, px + pw):
                farm[y * width + x] = 0x3000 | GRASS
    farm_img = renderer.render(farm, width)
    for name, title, px, py, pw, ph, tx, ty in BUILDINGS:
        w, h = pw * 16, ph * 16
        plot = farm_img.crop((px * 16, py * 16, px * 16 + w, py * 16 + h))
        plot.save(OUT / f"templates/{name}_{w}x{h}.png")
        grid_overlay(w, h).save(OUT / f"templates/{name}_{w}x{h}_grid.png")
        # Context: the plot outlined in its surroundings, terminal spot marked.
        margin = 3
        box = (max(px - margin, 0) * 16, max(py - margin, 0) * 16,
               min(px + pw + margin, width) * 16, min(py + ph + margin, farm_img.height // 16) * 16)
        context = farm_img.copy()
        draw = ImageDraw.Draw(context)
        draw.rectangle([px * 16, py * 16, px * 16 + w - 1, py * 16 + h - 1], outline=(255, 0, 0))
        if tx is not None:
            draw.rectangle([tx * 16, ty * 16 - 16, tx * 16 + 15, ty * 16 + 15], outline=(0, 0, 255))
        context = context.crop(box)
        context.resize((context.width * 3, context.height * 3), Image.NEAREST).save(OUT / f"context/{name}.png")

    for name, layout_name, x, y, w, h in BUILDING_REFERENCES:
        if layout_name == "LittlerootTown":
            r, c, lw = renderer, cells, width
        else:
            r, c, lw = layout(layout_name)
        img = r.render(c, lw).crop((x * 16, y * 16, (x + w) * 16, (y + h) * 16))
        img.save(OUT / f"reference/buildings/{name}_{w * 16}x{h * 16}.png")

    for town in TOWN_REFERENCES:
        r, c, lw = layout(town)
        r.render(c, lw).save(OUT / f"reference/towns/{snake(town)}.png")

    write_palette_files([(slot, renderer.palettes[slot]) for slot in LITTLEROOT_PALETTES])


if __name__ == "__main__":
    main()
