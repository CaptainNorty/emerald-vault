#!/usr/bin/env python3
"""Build art/pokemon_kit/: reference sprites to draw a new Pokémon from.

    python3 tools/art/make_pokemon_kit.py sableye [more species...]

For each species, copies its sprites from graphics/pokemon/ into
reference/<species>/ at 1x, adds a 4x preview sheet, and writes its palette as
an Aseprite/GIMP .gpl. Also writes the three shared icon palettes and blank
templates. Templates that already exist are never overwritten, so drawings in
them are safe.
"""

import sys
from pathlib import Path

from PIL import Image, ImageDraw

GRAPHICS = Path("graphics/pokemon")
KIT = Path("art/pokemon_kit")
SCALE = 4
# Gen 3 sprites treat palette color 0 as transparent; this cyan is the usual one.
TRANSPARENT = (98, 205, 205)


def read_jasc(path):
    lines = path.read_text().split()
    count = int(lines[2])
    nums = list(map(int, lines[3:3 + count * 3]))
    return [tuple(nums[i:i + 3]) for i in range(0, len(nums), 3)]


def write_gpl(path, name, colors):
    rows = "".join(f"{r:3} {g:3} {b:3}\t{'transparent' if i == 0 else f'color {i}'}\n"
                   for i, (r, g, b) in enumerate(colors))
    path.write_text(f"GIMP Palette\nName: {name}\nColumns: 16\n#\n{rows}")


def scaled(img):
    return img.convert("RGBA").resize((img.width * SCALE, img.height * SCALE), Image.NEAREST)


def preview_sheet(folder, species):
    """Front frames, back and icon frames side by side at 4x, labeled."""
    parts = [("front, frame 1 (top half)", Image.open(folder / "anim_front.png").crop((0, 0, 64, 64))),
             ("front, frame 2 (bottom half)", Image.open(folder / "anim_front.png").crop((0, 64, 64, 128))),
             ("back", Image.open(folder / "back.png")),
             ("icon frame 1", Image.open(folder / "icon.png").crop((0, 0, 32, 32))),
             ("icon frame 2", Image.open(folder / "icon.png").crop((0, 32, 32, 64)))]
    pad, label = 16, 20
    width = sum(p.width * SCALE + pad for _, p in parts) + pad
    sheet = Image.new("RGBA", (width, 64 * SCALE + label + pad * 2), (255, 255, 255, 255))
    draw = ImageDraw.Draw(sheet)
    x = pad
    for text, part in parts:
        big = scaled(part)
        sheet.paste(big, (x, pad + label), big)
        draw.rectangle((x - 1, pad + label - 1, x + big.width, pad + label + big.height), outline=(200, 200, 200))
        draw.text((x, pad), text, fill=(0, 0, 0))
        x += big.width + pad
    sheet.save(KIT / "reference" / species / f"{species}_sheet_4x.png")


def blank(path, size, palette):
    if path.exists():
        return
    img = Image.new("P", size, 0)
    img.putpalette([c for rgb in palette for c in rgb] + [0] * (768 - 3 * len(palette)))
    img.save(path, transparency=0)


def main():
    species_list = sys.argv[1:] or ["sableye"]
    for d in ("reference", "palettes", "templates"):
        (KIT / d).mkdir(parents=True, exist_ok=True)

    for species in species_list:
        src = GRAPHICS / species
        out = KIT / "reference" / species
        out.mkdir(exist_ok=True)
        for name in ("anim_front.png", "front.png", "back.png", "icon.png", "footprint.png"):
            Image.open(src / name).save(out / name)
        write_gpl(KIT / "palettes" / f"{species}.gpl", species.capitalize(), read_jasc(src / "normal.pal"))
        write_gpl(KIT / "palettes" / f"{species}_shiny.gpl", f"{species.capitalize()} shiny",
                  read_jasc(src / "shiny.pal"))
        preview_sheet(src, species)

    icon_palettes = sorted((GRAPHICS / "icon_palettes").glob("icon_palette_*.pal"))
    for pal in icon_palettes:
        write_gpl(KIT / "palettes" / f"{pal.stem}.gpl", f"Icon {pal.stem[-1]}", read_jasc(pal))

    # Blank templates start with the first species' palette so colors 0 (transparent) lines up.
    palette = read_jasc(GRAPHICS / species_list[0] / "normal.pal")
    blank(KIT / "templates" / "front_anim_64x128.png", (64, 128), palette)
    blank(KIT / "templates" / "back_64x64.png", (64, 64), palette)
    blank(KIT / "templates" / "icon_32x64.png", (32, 64), read_jasc(icon_palettes[0]))


if __name__ == "__main__":
    main()
