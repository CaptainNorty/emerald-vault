# Building art kit

Everything you need to draw the nine Littleroot farm buildings in the existing
Gen 3 style. Regenerate the kit with `python3 tools/art/make_building_kit.py`.

## What's here

| Folder | What it is |
|---|---|
| `templates/` | One blank plot per building at **exact size** (grass), plus a `_grid` overlay with 16px (solid) and 8px (faint) lines. Draw on these. |
| `context/` | Each plot in its spot on the farm, 3x zoom. Red box: the plot. Blue box: the terminal, which stands in front of the building. |
| `reference/buildings/` | Existing buildings at 1x to match: Brendan's house, Birch's Lab, Pokémon Center, Poké Mart, Oldale house. |
| `reference/towns/` | Whole towns at 1x (Petalburg, Rustboro, Slateport, Mauville, Fortree, Lilycove, Mossdeep, Fallarbor, Lavaridge, Dewford) for more building styles. |
| `palettes/` | Littleroot's colors: `.aco` (Photoshop), `.swatches` (Procreate, one per palette), `.gpl` (Aseprite, Pixelorama, GIMP, Krita), and a swatch sheet PNG. |

## Sizes

The map is built from 16×16 px squares, each made of four 8×8 px tiles.

| # | Building | Plot (squares) | Canvas (px) |
|---|---|---|---|
| 1 | Delivery Service | 6 × 5 | 96 × 80 |
| 2 | Greenhouse | 7 × 7 | 112 × 112 |
| 3 | Quarry | 7 × 7 | 112 × 112 |
| 4 | Fishery | 7 × 7 | 112 × 112 |
| 5 | Generator | 5 × 5 | 80 × 80 |
| 6 | Forge | 7 × 6 | 112 × 96 |
| 7 | Spirit Sanctum | 7 × 6 | 112 × 96 |
| 8 | Ice Dome | 7 × 6 | 112 × 96 |
| 9 | Weather Stabilizer | 11 × 9 | 176 × 144 |

For scale: Brendan's house is 80 × 80, Birch's Lab 112 × 80, and the Pokémon
Center and Poké Mart 64 × 64. A building doesn't have to fill its plot; grass
shows around it.

## Rules the Game Boy Advance imposes

1. **Draw at 1x, pixel by pixel.** Zoom in to work, but the file is the real
   size. No anti-aliasing, soft brushes, blur or partial transparency.
2. **Export PNG.** Never JPEG: it smears colors and ruins pixel art. RGB or
   indexed PNG both work. Leave the background as grass or fully transparent.
3. **Colors per 8×8 tile:** Every 8×8 tile (the faint grid) can use at most 15
   colors, all from one palette. In practice: keep each building to about 15
   colors. Reusing colors from `palettes/` is free. All nine buildings together
   can add **up to 45 new colors** (3 free palettes of 15).
4. **Unique tiles:** The town's tileset has room for about **350 new 8×8
   tiles** across all nine buildings: roughly 35 per building and 70 for the
   Weather Stabilizer. Tiles that repeat exactly, or are mirror images, cost
   nothing. That's why Gen 3 buildings use repeating roof shingles, wall panels
   and symmetrical shapes. A roof made of one repeated pattern is cheap; a
   hand-painted unique roof is expensive. I'll count tiles and colors for each
   drawing and tell you if one goes over.
5. **No interiors yet.** Doors are decoration, so put them anywhere along the
   bottom. The player can't walk through any part of the plot.

## Matching the style

- Three-quarter top-down view: the roof takes the top half or more, and the
  front wall with the door and windows sits along the bottom.
- Light comes from the top left. Shadows fall on the right and bottom edges.
- Outlines are dark navy, not black (`41 49 90` and `65 74 106` in the
  palettes), with darker shades of each color for edges.
- Roofs are 2–4 tone bands with a repeating texture. Walls are 2–3 tones.
- Compare against `reference/buildings/` at the same zoom while drawing.

## Tools

- **Aseprite (recommended, Mac):** Built for pixel art; about $20 from
  aseprite.org or Steam. Free look-alikes: **LibreSprite** and
  **Pixelorama**. Open a template, choose Palette → Load Palette →
  `littleroot.gpl`, set View → Grid Settings to 16 × 16 (or 8 × 8), and use the
  Pencil. Your Wacom Intuos works as a mouse/pen here. Export with File →
  Export As → PNG at 100%.
- **Photoshop:** Use the **Pencil** tool (not the Brush) at 1 px. Under
  Preferences → General, set Image Interpolation to Nearest Neighbor. Under
  Guides, Grid & Slices, set a gridline every 16 px with 2 subdivisions. Load
  swatches from `littleroot.aco`. Export As PNG at 100%.
- **Procreate (iPad):** Good for sketching concepts at large size, but its
  brushes blend edges, so final 1x pixels are hard. For pixel art on iPad, try
  **Resprite** (Aseprite-like) or **Pixaki**. If you do use Procreate: make a
  canvas at the exact size, import the template as a layer, turn on Drawing
  Guide → 2D Grid at 8 px, and use a hard square 1 px brush. Import the
  `.swatches` files through the Files app.

## Handing art back

Save each finished building as a PNG with the same name as its template (for
example `templates/2_greenhouse_112x112.png` → `art/buildings/2_greenhouse.png`).
I'll convert it to tiles and palettes, add it to the map, check the tile and
color budgets, and show you an in-game screenshot.

The conversion is `tools/art/import_building.py <png> --palette <6-12> --name
<array>`. Grass-colored pixels become transparent over the game's own grass, so
they cost no colors. It reuses tiles that already exist, so re-running it after
a small edit only adds the changed tiles. It prints the map entries for
`src/data/vault_town_buildings.h`.
