# Pokémon art kit

What you need to draw a new Pokémon in the Gen 3 style. Add more reference
species with `python3 tools/art/make_pokemon_kit.py sableye mawile ...`
(it never overwrites the templates).

## What's here

| Folder | What it is |
|---|---|
| `reference/<species>/` | The game's own sprites at 1x, exactly as the game stores them, plus a labeled 4x preview sheet. |
| `palettes/` | Each reference species' palette (normal and shiny) and the three shared icon palettes, as `.gpl` for Aseprite. |
| `templates/` | Blank canvases at the right sizes. Copy one before drawing in it. |

## The files a Pokémon needs

| File | Size | What it is |
|---|---|---|
| `anim_front.png` | 64 × 128 | Front sprite, two frames stacked. **Top 64×64 is the main pose** (also used everywhere the sprite doesn't move, like the summary screen). **Bottom 64×64 is the second frame**, shown when it animates on entering battle. |
| `back.png` | 64 × 64 | What you see of your own Pokémon in battle. Usually cut off at the bottom. |
| `icon.png` | 32 × 64 | Party menu and PC icon, two frames stacked (it bobs between them). |
| `footprint.png` | 16 × 16 | Optional, black and white. We can reuse one. |

## Rules

1. **One palette for the front and back:** 15 colors plus a transparent
   background color (color 0). The front and back must use the same colors.
   A shiny palette is optional: same pixels, 15 swapped colors.
2. **The icon uses one of three shared palettes** (`icon_palette_0/1/2.gpl`).
   Pick the one closest to your colors. Sableye's icon uses palette 2.
3. **Draw at 1x, pixel by pixel:** no anti-aliasing or soft brushes. Export
   PNG.
4. **Fill the background with color 0** (the cyan `98 205 205` in Sableye's
   palette) or leave it fully transparent.
5. **Frame 2 is a small change from frame 1:** a raised arm, a squash, a head
   tilt. Keep the feet in the same place so it doesn't jump.
6. **Size:** the Pokémon sits in the bottom middle of its 64×64 square. Small
   Pokémon leave lots of empty space; big ones nearly fill it. An evolution
   should look bigger than Sableye.
7. **Light from the top left**, dark outlines (not pure black for most parts),
   2–3 shades per color.

## Aseprite setup

Open `templates/front_anim_64x128.png` (save a copy first, like
`art/pokemon/<name>/anim_front.png`). Load the reference palette with Palette →
Load Palette, set View → Grid Settings to 64 × 64, and open
`reference/sableye/anim_front.png` beside it. Draw frame 1 in the top half,
then copy it to the bottom half and change it. Hand the folder back and I'll
put it in the game.

## In the game

Finished art lives in `art/pokemon/<name>/` (`anim_front.png`, `back.png`,
`icon.png`) and is copied, re-saved as 4-bit PNGs, into
`graphics/pokemon/<name>/` along with `front.png` (the top frame), the
palettes and a footprint. Sableer is the first: `art/pokemon/sableer/`.
