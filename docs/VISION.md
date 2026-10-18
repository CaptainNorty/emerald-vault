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
3. **Earn:** The business sells its product, earns money and stocks items
   based on the **steps you take**. Its worker gains EXP.
4. **Check in:** Each business has a **terminal** that reports how it's going
   (originally pitched as a manager):
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

Two or more types per business, so all 17 types have a home. Prices rise
exponentially. Earnings per step start at the Delivery Service's rate, double
for the Greenhouse, then scale with price (to be tuned).

| Position | Building | Types | Sells | Price | Per 20 steps at 1.0x | Requires (besides money) | Specialist |
|---|---|---|---|---|---|---|---|
| Center | Weather Stabilizer | Dragon | Capstone | Not set | — | Rain Badge (8th Gym) and all four feeders operational | Castform, Altaria, Flygon, Salamence |
| N (feeder) | Forge | Fire / Fighting | Poké Balls | ₽250,000 | ₽1,200 | Heat Badge (4th Gym) | Blaziken |
| E (feeder) | Generator | Electric / Steel | Power cells | ₽80,000 | ₽384 | Dynamo Badge (3rd Gym) | Magneton |
| W (feeder) | Ice Dome | Ice | Ice blocks | ₽1,500,000 | ₽7,200 | An Ice-type Pokémon | Glalie |
| S (feeder) | Spirit Sanctum | Ghost / Psychic / Poison | Charms | ₽750,000 | ₽3,600 | Having visited Mt. Pyre | Dusclops, Claydol |
| NW | Delivery Service | Flying (Normal 1.5x) | Deliveries | Free | ₽12 | A Flying-type | Swellow |
| NE | Greenhouse | Grass / Bug | Berries | ₽5,000 | ₽24 | Nothing | Ludicolo |
| SW | Fishery | Water / Dark | Caught Pokémon | ₽45,000 | ₽216 | Old Rod (from Dewford) | Sharpedo |
| SE | Quarry | Rock / Ground | Evolution stones | ₽15,000 | ₽72 | Stone Badge (1st Gym) | Golem |

Every business also needs a Pokémon of one of its types in the party before the
Builder will rebuild it; that Pokémon becomes its first worker. After that,
anyone can work there, but those types earn 2x, others 0.5x, scaled by level /
10, doubled for a specialist. Up to 100 uncollected sales pile up per business.

### The Builder and the terminals (built)
After the free Delivery Service, the Builder stands in the yard and lists the
businesses whose requirements you've met, cheapest first, with prices. He takes
the money (shown in a money box), rebuilds the plot with placeholder tiles
(existing house, Lab, Pokémon Center and Mart tiles; a pond and a quarry pit),
and its terminal appears. Each terminal takes the first worker, shows the
worker's picture, level and speed, collects money and items, and can change or
return the worker. Code: `src/vault_town.c`
(one table, `sBusinesses`, describes every business) and
`src/data/vault_town_buildings.h` (the placeholder tiles).

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
opened from the Delivery Service terminal.

### Unlocking the Delivery Service (built)
The Delivery Service is the first business and is introduced before the first
Gym, so the player learns early that earnings depend on the Pokémon doing the
work.

- **The Builder** (a Black Belt) waits at the Delivery Service ruins from the
  start of the game. The first building is free: he only needs to see a
  Flying-type in the party. He then rebuilds it and moves to the yard between
  the old town and the farm, where he'll offer future buildings.
- **Oldale's west road is closed** after the player gets the Pokédex until the
  Delivery Service is rebuilt. The Builder's assistant points the player to
  Wingull on Route 103 (20% of encounters) and hands out 5 Poké Balls to anyone
  who has none, so the player can't get stuck.
- **The terminal** handles staffing, reports and collection. The first worker
  must be a Flying-type; after that, any Pokémon can work there, but it can be
  swapped or taken back anytime. The terminal also gives the **Bird Whistle**
  (flies you back to Littleroot from anywhere outdoors) and can fly you to any
  visited town.
- **Earnings:** one delivery per 20 steps at 1.0x efficiency, ₽12 each, up to
  100 uncollected. Efficiency is type fit (2x Flying, 1.5x Normal, 0.5x other)
  x level / 10, doubled for Swellow. A level 5 Wingull runs at 1.0x.
- **Code:** `src/vault_town.c` (businesses, earnings, workers, flights),
  `include/constants/vault_town.h`, Littleroot and Oldale map scripts. Workers
  and earnings are saved at the end of PC storage (`struct TownSaveData`, about
  940 bytes of the ~2 KB free).

### Story note
In Emerald, Team Magma awakens Groudon and Team Aqua awakens Kyogre. Their clash
causes extreme weather around Sootopolis, and the player calls Rayquaza from Sky
Pillar to calm them. The Weather Stabilizer could replace or support that
ending. To be decided later.

## Tying the farm to the adventure (built)

The first playtest (about an hour) found that the farm and the adventure felt
like **two disconnected gameplay loops**. The farm only produced money, money
was only useful for more buildings, and it wasn't visible without opening a
menu. Giving a good Flying-type to the Delivery Service also meant losing a
team member for nothing but cash.

Three directions were considered:

1. **The farm is the item source.** Poké Marts are closed (or nearly empty, or
   priced terribly, or battle income is cut), and 90%+ of items come from the
   farm.
2. **Money gates progression now.** Gym entry, boat passage or Pokémon Center
   healing cost money, so money matters from early on (see below).
3. **The world needs what the farm makes.** Like Stardew Valley's bundles,
   people ask for things only the farm can produce (a peach, not just money),
   so progression isn't only gated by money.

**Chosen direction: #1 now, #3 layered on later.** Terminals, items, closed
Marts, worker EXP, upgrades and the FARM box are built.

- **Buildings produce items as well as money (built).** Each building stocks
  one item per 50 steps at 1.0x efficiency, rolled from its weighted item table
  (`sDeliveryServiceItems` and friends in `src/vault_town.c`), up to 30
  uncollected. Tier 1 tables:

  | Building | Items |
  |---|---|
  | Delivery Service | Potion, Poké Ball, Antidote, Repel, Escape Rope |
  | Greenhouse | Oran, Pecha, Cheri, Rawst, Chesto, Aspear, Persim, Sitrus Berries |
  | Quarry | Stardust, Hard Stone, Everstone, Soft Sand, Fire/Water/Thunder/Leaf Stones (rare) |
  | Fishery | Pearl, Shoal Salt, Shoal Shell, Heart Scale, Net Ball, Dive Ball, Big Pearl |
  | Generator | Super Potion, Super Repel, Paralyze Heal, Awakening, Magnet, Thunder Stone |
  | Forge | Great Ball, Ultra Ball, Timer/Nest/Repeat Balls, Burn Heal, Charcoal |
  | Spirit Sanctum | Revive, Full Heal, Hyper Potion, Cleanse Tag, Spell Tag, Max Revive |
  | Ice Dome | Max Potion, Max Repel, Ice Heal, Full Restore, Ultra Ball, Never-Melt Ice |
- **Marts sell nothing, but buy (built).** The `pokemart` script command now
  jumps to `EventScript_MartClosed` (`data/scripts/marts_closed.inc`): every
  item shop's clerk says the shelves are empty, points to the farm in
  Littleroot, and opens a SELL / QUIT menu (`CreateSellOnlyMartMenu` in
  `src/shop.c`) at the usual half price. Decoration shops still work. They may
  be replaced with something else later.
- **Littleroot's Poké Mart (built).** When the Devon Goods are returned, Mr.
  Stone thanks the player for bringing goods back to Hoenn and places a buyer in
  Littleroot (`FLAG_LITTLEROOT_MART_OFFERED`; saves already past him count the
  PokéNav instead). The Builder then offers a POKé MART for ₽3,000 on a 4x4
  plot south of Birch's Lab (`TOWN_MART`, a plot but not a business: no workers
  or terminal). Inside (`MAP_LITTLEROOT_TOWN_MART`), Devon's buyer only buys.
- **Upgrades (built).** The Builder sells upgrades to Lv. 2 (2x the building's
  price) and Lv. 3 (6x); the free Delivery Service is priced as ₽2,500, so
  ₽5,000 and ₽15,000. His menu lists new buildings first, then upgrades, each
  cheapest first. Each level adds a worker slot (1, 2, 3 workers, whose speeds
  add up) and switches to a better item table (`sDeliveryServiceItems2` and
  `...3`; for example the Delivery Service moves on to Super Potions and Great
  Balls, then Hyper Potions and Ultra Balls). The building's tiles don't change
  yet. No badge requirements for upgrades for now.
- **Workers gain EXP (built)** at half the Day Care's rate, 1 EXP per 2 steps
  (`WORKER_STEPS_PER_EXP`), so the Pokémon you leave working grows instead of
  being a pure loss to your team. Like the Day Care, they don't learn moves or
  evolve while working. A Lv. 5 Lotad reaches Lv. 7 in about 200 steps; still
  a tuning knob.
- **Workers live in the FARM box (built).** Storing a full Pokémon per slot
  wouldn't fit in the save, so the last PC box (box 14) is named FARM and holds
  every worker: building *b*'s slot *s* is box position *b* × 3 + *s*. The PC
  shows it, and Summary works, but moving, withdrawing, depositing and
  releasing there say "Use the farm terminals!", and caught Pokémon never go
  there. On older saves, anything kept in box 14 moves to free spots in other
  boxes and each building's worker moves into the FARM box when the save loads
  (checked against a copy of a real playthrough save).
- **Terminals instead of managers (built).** Each building has a terminal (the
  farm PC sprite) where the manager stood. It shows the worker's front sprite,
  level and speed, and how much money and how many items are in stock. Menu:
  COLLECT (money, then each kind of item; items that don't fit in the bag stay
  in stock) / FLY (Delivery Service only) / CHANGE WORKER / TAKE WORKER BACK /
  LOG OFF. Items are stored as a count per item-table entry (12 bytes per
  building, in a separate array so saves from before items still load). Managers could come back later as flavor NPCs.
- **Later:** Overworld sprites of workers walking around their building
  (pokeemerald-expansion has overworld sprites for every species), Stardew-style
  requests from townspeople (for example, a Slateport sailor wants Fishery
  catches before the boat sails), money gates, and real building art.

### Quick wins from the playtest
- Fix the cut-off and floating tree tops around the farm.
- A Pokémon Storage PC on the farm, near the buildings.
- The money box is shown whenever the Start menu is open.

### Order
1. Quick wins (above). Done.
2. Terminals with the worker's sprite. Done.
3. Items: buildings stock items, Marts close, workers gain EXP. Done.
4. Upgrades: the FARM box, extra worker slots, higher item tiers. Done.
5. Building art: the Delivery Service is drawn and in the game (see
   `art/building_kit/README.md`); eight to go.
6. Next: rebalance the farm (below).

## Rebalancing: make the farm a game (next)

The second playtest (about 20 minutes) found the farm **far too generous and
almost thoughtless**:

- By the end, items were piling up faster than they could be used. Evolution
  stones and berries came especially fast.
- With Marts closed, there was nowhere to sell evolution stones, so they just
  stacked up in the bag.
- One or two laps around the farm filled the Delivery Service, Greenhouse and
  Quarry to their 100-sale caps, with thousands in uncollected money. Running
  in circles would pay for every upgrade and fill the bag.
- The Greenhouse and Quarry upgrades were affordable in under five minutes.

**Why:** worker speed grows in step with level, so a Lv. 20 worker of a suited
type runs at 4x. That's a sale every 5 steps and an item every 12–13 steps, each
item a random roll from the whole table. Production runs on any step, including
laps of the farm itself.

**The goal is Stardew Valley's "Hmm."** What should I plant, how will it get
watered, what do I do while it grows, and do I sell it or turn it into jam?
Even on a smaller scale, the farm should ask for decisions and patience. More
broadly: the game is too easy, and the farm is the main lever for fixing that.

### The plan

1. **Buildings make items, never money.** Marts reopen **for selling only**
   (buying stays closed; built, along with Littleroot's own Mart), so money
   comes only from what you choose to sell.
   Every item becomes "use it or sell it?", and evolution stones have value (a
   Fire Stone sells for ₽1,050). Upgrade prices start to mean something again.
2. **Steps on the farm don't count.** Production only moves forward on steps
   taken **outside Littleroot**, so running laps does nothing: you start
   something, go adventure, and come back. (Alternative to consider: count
   battles won or new places visited, which ties the pace to the story.)
   Production also becomes much slower overall. Want more Poké Balls? Bring a
   higher-level worker and go do something while you wait. Want an evolution
   stone? Those are valuable; bring a great worker and get busy.
3. **Work orders.** At the terminal you choose what the building makes next,
   for example "5 Super Potions" or "1 Fire Stone". It makes that order, then
   stops and waits for you. Each product has a step cost (roughly: Potion 100,
   Super Potion 400, Fire Stone 3,000), which worker speed shortens. Upgrades
   unlock better products and possibly bigger orders. The decision becomes:
   what will I need by the time I'm back?
4. **Story trades.** Farm goods change hands at story moments:
   - **Steven in Granite Cave** collects stones. He asks for something from the
     Quarry and in return unlocks the Quarry's evolution-stone orders, or gives
     a Beldum that's a Quarry specialist.
   - **Captain Stern**, after he's rescued from Team Aqua, hands over something
     for the Fishery: a sonar upgrade that unlocks better catches, or a rod.
   These are the first of the "the world needs what the farm makes" requests.
   Later, townspeople could pay more than the Mart for specific goods, adding
   "who do I sell to?".
5. **Processing (later): the "jam or sell?" choice.** One building turns
   another's output into something worth more: berries into Berry Juice or
   Pokéblock ingredients, Hard Stone plus Stardust into an evolution stone at
   the Forge, and so on.

### Order

1. Items only, Marts reopened for selling (done, plus a Mart in Littleroot),
   steps count only outside Littleroot, slower production. Small changes that fix most of the
   generosity. Playtest again before going further.
2. Work orders.
3. The Steven and Captain Stern trades.
4. Processing and townspeople requests.

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

- **Where assigned Pokémon live:** Each business stores its worker as a full
  Pokémon (80 bytes) in its own save slot at the end of PC storage, so no PC box
  had to be locked. Room is reserved for one worker per building.
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
- **Money cap:** Raised from vanilla's ₽999,999 to **₽9,999,999**
  (`MAX_MONEY` in `src/money.c`); money boxes and the trainer card show 7
  digits. Standard Poké Mart prices are 16-bit (max ₽65,535), so expensive
  purchases use scripted purchases, like the Builder's. Late-game earnings may
  hit the cap quickly at high efficiency; tune when balancing.
- **If HMs are sold:** Vanilla checks badges before letting you use HM field
  moves (`src/party_menu.c`, `src/field_control_avatar.c`). Decide whether those
  checks go away. Waterfall isn't in the HM price list yet.
- **Changing the town as you rebuild:** Scripts can swap individual map tiles
  based on story flags (`setmetatile`, used about 600 times in vanilla map
  scripts). Pre-built stages unlocked by flags are the simplest approach.
  Buildings with interiors need new interior maps.
