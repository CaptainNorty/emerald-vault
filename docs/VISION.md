# Emerald Vault: Vision & Goals

## The pitch

Emerald Vault makes **earning money** a core focus of Pokémon Emerald. Your home
town, Littleroot, is surrounded by ruined buildings. You rebuild them as
**businesses** that earn money as you walk, staffed by Pokémon you assign. The
better suited and stronger the Pokémon, the more the business earns. Key steps
in the game's progression cost money, so building up your town's income is how
you move forward.

Ideas that were explored and set aside are kept in
[UNUSED_IDEAS.md](UNUSED_IDEAS.md).

## Core loop

1. **Rebuild:** Pay money and contribute one or more relevant Pokémon to rebuild
   a ruined plot as a business.
2. **Staff:** Assign Pokémon to the business. Their species, type and level set
   its **efficiency**.
3. **Earn:** The business sells its product and earns money based on the
   **steps you take**.
4. **Check in:** Each business has a **manager** who reports how it's going:
   - "Business is going great! I've sold 73 evolution stones for a profit of
     $12,683."
   - "Hey boss, we've sold 892 Cheri Berries for $26,984."
   - "Yo! We've caught a total of 18 Pokémon and sold them for $4,802."
5. **Collect:** Collect the earnings and spend them on progression and more
   buildings.

Training a Pokémon now has two uses: your team, or a business. Choosing between
them is part of the game.

## Efficiency from assigned Pokémon

How well a business runs depends on the Pokémon you assign. Examples:

- A level 3 Seedot runs the Greenhouse very slowly. A level 40 Ludicolo runs it
  at 10× the efficiency.
- A level 3 Skitty barely powers the Generator (0.3×). A leveled-up Manectric
  gets a 15× return.

Draft formula: **type fit × level factor × specialist bonus**.

| Pokémon | Type fit | Level factor | Specialist bonus | Result |
|---|---|---|---|---|
| Skitty, Lv 3, at the Generator | 0.5× (Normal) | 0.6× | none | **0.3×** |
| Manectric, Lv 40, at the Generator | 2× (Electric) | 4× | ~2× | **~15×** |

A specialist bonus for a few ideal species per building (for example, Ludicolo
for the Greenhouse) rewards players who hunt for the right Pokémon. Exact numbers
are for tuning.

## The town

The farm sits on the **east side of Littleroot**, right next to the Route 101
entrance, so it's the first thing you see coming home. The original town (both
houses and Birch's Lab) stays exactly where it is on the west side, so none of
the story's triggers or events move. The open area south of Birch's Lab is
reserved for possible post-Elite Four buildings.

- **Any order:** Like the Community Center bundles in Stardew Valley, some
  buildings are cheap and easy to rebuild and others are expensive or need rare
  Pokémon. The player chooses what to rebuild first, within each building's
  requirements.
- **Requirements:** Any mix of money, specific Pokémon, items, badges, or
  having visited a place.
- **Operational** means rebuilt with at least one Pokémon assigned.

### Layout

The Weather Stabilizer is in the center. The four buildings north, east, south
and west of it are **feeders** that power it. The four corner buildings are
independent businesses.

```
 Delivery Service  |      Forge         |  Greenhouse
   (NW)            |   (N, feeder)      |    (NE)
-------------------+--------------------+------------------
   Ice Dome        | WEATHER STABILIZER |   Generator
  (W, feeder)      |     (center)       |  (E, feeder)
-------------------+--------------------+------------------
    Fishery        |  Spirit Sanctum    |    Quarry
   (SW)            |   (S, feeder)      |    (SE)
```

The feeders pair with the Groudon/Kyogre conflict: the Forge provides heat
against Kyogre's endless rain, the Ice Dome provides cold against Groudon's
drought, the Generator provides power, and the Spirit Sanctum provides soul
energy. Mt. Pyre is Hoenn's resting place for Pokémon souls and where the Red
and Blue Orbs are kept, so the Sanctum ties into the main story.

### Building lineup

Two or more types per business, so all 17 types have a home.

| Position | Building | Types | Sells | Requires (besides money) | Specialist |
|---|---|---|---|---|---|
| Center | Weather Stabilizer | Dragon | Capstone | Rain Badge (8th Gym) and all four feeders operational | Castform, Altaria, Flygon, Salamence |
| N (feeder) | Forge | Fire / Fighting | Poké Balls, held items | Heat Badge (4th Gym) | Blaziken |
| E (feeder) | Generator | Electric / Steel | Power | Dynamo Badge (3rd Gym) | Magneton |
| W (feeder) | Ice Dome | Ice | Ice, cold storage | An Ice-type Pokémon | Glalie |
| S (feeder) | Spirit Sanctum | Ghost / Psychic / Poison | Remedies, blessings | Having visited Mt. Pyre | Dusclops, Claydol |
| NW | Delivery Service | Flying / Normal | Deliveries | Nothing (early) | Swellow |
| NE | Greenhouse | Grass / Bug | Berries | Nothing (early) | Ludicolo |
| SW | Fishery | Water / Dark | Caught Pokémon | Old Rod (from Dewford) | Sharpedo |
| SE | Quarry | Rock / Ground | Evolution stones | Stone Badge (1st Gym) | Golem |

All specialists can be caught or obtained in Hoenn. Hoenn's Ice types (Snorunt,
Spheal) live in Shoal Cave, which needs Surf, so the Ice Dome naturally comes
late.

### Delivery Service flights
Once built, the Delivery Service flies you **from anywhere back to
Littleroot**, and **from Littleroot to any city you've visited**, even before
you have the Fly HM. It doesn't replace Fly, which still matters for travel
between other cities, and it gives a strong reason to build it early.

Technically: flying home is a key item with a Fly-style warp, usable outdoors
and reusing an existing item icon. Flying out reuses vanilla's Fly map screen,
opened from the Delivery Service manager's dialogue.

### Story note
In Emerald, Team Magma awakens Groudon and Team Aqua awakens Kyogre. Their clash
causes extreme weather around Sootopolis, and the player calls Rayquaza from Sky
Pillar to calm them. The Weather Stabilizer could replace or support that
ending. To be decided later.

## Progression gated by money

Key progression events are blocked until you pay. That gives you the reason to
build income.

- **Gym entry fees:** For example, $3,000 to enter the first Gym, $10,000 for
  the second, and so on.
- **Paid passage:** A person blocks the entrance to the next area and asks for,
  say, $50,000. Once paid, they leave.

### HMs for sale (option)
All the HMs you need could be for sale in the first city, with costs that rise
exponentially. This would teach you what the game is about right away. Draft
prices:

| HM         | Price      |
|------------|------------|
| Flash      | $200       |
| Cut        | $1,000     |
| Rock Smash | $10,000    |
| Surf       | $50,000    |
| Fly        | $100,000   |
| Strength   | $500,000   |
| Dive       | $1,000,000 |

## Economy design principles

- **Exponential growth:** The economy should scale exponentially over the
  course of the game.
- **Stardew Valley feel:** You have to invest before cash starts coming in.
  Every investment has a **break-even point** and a **profit point**.
- **Many valid strategies:** Some strategies should make more money than others,
  but you should always have several options.

## Non-goals

The economy systems are a big enough job on their own, so to keep the project
manageable:

- No new Pokémon
- No changes to music or sprites
- Map changes limited to Littleroot Town and what money gates need

## Milestones

### First playable
- One business can be rebuilt with money and a Pokémon.
- Assign Pokémon; efficiency depends on them.
- The business earns money per step.
- The manager reports totals, and you can collect the earnings.
- Progression doesn't have to be gated by money yet.

### First release
- The full building lineup.
- Progression gated by money (Gym fees, paid passage, possibly the HM shop).

## Open questions & technical notes

- **Where assigned Pokémon live:** Reserve one PC box as a locked "town box,"
  with each building's Pokémon in a fixed slot. This needs no new save space,
  keeps the Pokémon real (stats, nature), and lets you take them back. A real
  Pokémon takes 80 bytes, and the save file is nearly full: about 120 bytes free
  in the main save block, about 84 in the player block, and about 2 KB in the PC
  storage block.
- **Per-building save data:** Step count at last collection, items sold, and
  money earned. A few bytes per building, which fits in the PC storage block's
  free space.
- **Steps:** The game already counts steps (`GAME_STAT_STEPS`, incremented in
  `src/field_control_avatar.c`). Each business stores the step count from its
  last collection, and earnings are worked out from the difference.
- **Step farming:** Earning per step invites players to ride their bike in
  circles. A stockroom cap per building handles it ("Boss, the stockroom's full,
  come collect!").
- **Collecting:** Visit each manager in person early on, and later unlock one
  central place to collect from all of them?
- **Who the managers are:** Possibly trainers you've beaten and hired (see
  [UNUSED_IDEAS.md](UNUSED_IDEAS.md)).
- **Manager dialogue:** Scripts can already insert numbers and item names into
  text, so the reports are formatted messages.
- **Money gates:** Vanilla already has NPCs who block a path until a flag is
  set. Paying one is: check money, show the money box, take the payment, set a
  flag, remove the NPC.
- **Money cap:** Vanilla caps money at **$999,999** (`MAX_MONEY` in
  `src/money.c`). Money is already saved as a 32-bit number, so raising the cap
  is a small, save-compatible change, mostly to the displays that assume 6
  digits (the money box and the trainer card). Standard Poké Mart prices are
  16-bit (max $65,535), so expensive purchases should use scripted purchases,
  which already handle 32-bit amounts.
- **If HMs are sold:** Vanilla checks badges before letting you use HM field
  moves (`src/party_menu.c`, `src/field_control_avatar.c`). Decide whether those
  checks go away. Waterfall isn't in the HM price list yet.
- **Changing the town as you rebuild:** Scripts can swap individual map tiles
  based on story flags (`setmetatile`, used about 600 times in vanilla map
  scripts). Pre-built stages unlocked by flags are the simplest approach.
  Buildings with interiors need new interior maps.
