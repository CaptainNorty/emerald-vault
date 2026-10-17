# Emerald Vault: Unused Ideas

Ideas that were explored and set aside, kept here so they can be revisited.
Each one notes why it was set aside. The current plan is in
[VISION.md](VISION.md).

## Hiring trainers (original core mechanic)

*Set aside because buildings run as businesses became the core loop. Could come
back as how buildings get their managers: beat a trainer, then hire them to run
a store.*

1. **Hire:** After beating a trainer in battle, you can hire them.
2. **Staff:** A hired trainer needs Pokémon to battle with, and you provide them.
3. **Earn:** Once a hired trainer has Pokémon, they build up money over time,
   measured in steps.
4. **Collect:** You go back to the trainer to collect what they've earned.
5. **Reclaim:** You can take back Pokémon you've given to a hired trainer at any
   time.

Details that were decided:
- Early-game trainers earn a little; late-game trainers earn more.
- The game decides whether a hired trainer wins, and how often, based on the
  quality of the Pokémon you gave them.
- Early on you visit each hired trainer to collect. Later you pay to unlock a
  building where you collect from and manage all hires in one place.

Open questions it left:
- How is a hired trainer's win rate calculated (level, stats, type matchups)?
- How many hires, and how many lent Pokémon each, can the save hold? Do lent
  Pokémon gain experience while hired out?

## Production line (Factorio-style chaining)

*Set aside because it adds complexity without giving players a reason to care:
another way to get eggs or items isn't more interesting than the routes and
shops that already exist.*

Buildings pass their output to each other, with the player choosing where
everything goes:

1. **Day Care:** "What do you want to do with the eggs?" → Hatch them / Sell
   them.
2. Once the Dojo is unlocked: "What do you want to do with the hatched Pokémon?"
   → Send to the Dojo for training.
3. **Dojo:** "What level do you want us to raise them to?" → options depend on
   badge count. "Once trained, what do you want us to do with them?" → Sell
   them / Send to the Day Care for breeding / Quarry for mining / Generator for
   power.

How it would work technically:
- **Batches, not real Pokémon:** Track groups such as "12 Zigzagoon, level 15,
  going to the Market," about 8 bytes each, instead of 80-byte real Pokémon.
  About 8 queued batches for each of 8 buildings is about 500 bytes.
- **Real Pokémon on demand:** Taking one out creates a real Pokémon of that
  species and level at that moment. Individual stats and natures aren't tracked
  while in the pipeline.
- **Routing:** Each building stores where its output goes and, for the Dojo, a
  target level. Short choices use vanilla's multiple-choice menus. Lists that
  grow as buildings unlock need a small custom scrolling list.
- **Catch-up simulation:** Each building stores the step count from its last
  update, and everything since then is worked out in one pass when the player
  visits.

Hard parts: balancing rates, showing the player what's flowing where (a summary
screen), and anonymous batches sitting awkwardly next to individually lent
Pokémon.

Smallest prototype: a Day Care with "Hatch them / Sell them," then a Dojo with a
fixed target level, sending its output to the Market.

## Breeding and selling at scale

*Set aside along with the production line. Many players barely use the vanilla
Day Care because they prefer raising Pokémon themselves.*

- Exchange Pokémon for money, starting by hand and gradually automating.
- Place Day Cares in your home town and breed Pokémon in large numbers.
- Story framing: for example, "Professor Oak needs you to repopulate the
  region."
- Technical note: the vanilla Day Care holds 2 Pokémon (`DAYCARE_MON_COUNT`) and
  the PC holds 14 boxes of 30, so large numbers of bred Pokémon would have to be
  stored as counts.

## Buildings that need specific Pokémon to function

*Partly kept: in the current plan, the Pokémon you assign set a building's
efficiency. The dependency chain between buildings was dropped.*

- An **Electrode** powers the generator.
- The generator is needed to open a home **Pokémon Center**.
- A **Chansey** (or another healing Pokémon) runs the Pokémon Center.
- A **Magnemite** helps build metal buildings or decorations.

## Building requirements placed between Gyms

*Replaced by rebuilding in any order, with money gates on Gyms and key paths.
A minimum badge count can still be one of a building's requirements.*

- You progress through the game as normal, but you can't rush: between Gyms you
  also have to meet the requirements for the next town build.

## Buildings considered and dropped

| Building | Idea | Why it was dropped |
|---|---|---|
| Hot Spring | Free healing at home. | Mom already heals your party for free in your house (`PlayersHouse_1F_EventScript_MomHealsParty`). Could return as a cosmetic add-on. |
| Fishing Pond (as a fishing spot) | Fish in town or stock fish that breed. | Less interesting than fishing on existing routes. Kept as a business instead (the Fishery). |
| Day Care+ (as a better Day Care) | More Pokémon, more eggs, upgradable. | Many players don't use the Day Care. Kept as a business instead (the Nursery). |
| Lab Annex + Wild Grass | Kanto and Johto species in a town grass patch, at levels based on badges. | Doesn't fit the business model. Note: all 386 Gen 1–3 species are already in Emerald's data, so this wouldn't break the "no new Pokémon" rule. |
| Workshop (item duplicator) | Put in one item; it slowly produces more. | Easy to exploit (Rare Candy, Master Ball) unless the accepted items are tightly limited. |
| Game Corner | A casino in town. | Mauville already has one. Vanilla's `src/slot_machine.c` and `src/roulette.c` could be reused if wanted. |
| Barn (Miltank) | Like the Miltank barn in Gold/Silver. | Could return as the Café, selling Moomoo Milk (already an item). |
| Café | Moomoo Milk and food (Normal / Ice). | Felt like a theme park rather than part of the push toward the Weather Stabilizer. Replaced by the Delivery Service and the Ice Dome. Note: Miltank isn't catchable in Hoenn. |
| Haunted House | Tickets to a spooky attraction (Ghost / Dark). | Same reason. Replaced by the Spirit Sanctum, which feeds the Weather Stabilizer. |
| Nursery | Eggs, with Ditto as the specialist. | Didn't make the 9. Could return as an upgrade or a post-Elite Four building. |
| Dojo | Training services (Fighting). | Didn't make the 9. Fighting is covered by the Forge. |
| Greenhouse (planting real berries) | Plant collected berries and harvest more. | Kept as a business that sells berries instead. Technical note: vanilla has 128 berry tree save slots and uses 88; vanilla berries grow on the real-time clock, not steps. |

## Other income sources

*Not part of the current plan. Could add variety later.*

- **Training contracts:** An NPC pays a lump sum for training a Pokémon to a
  certain level.
- **Pokémon sales:** An NPC buys specific Pokémon from you.
- **Gym ownership:** You buy the rights to a Gym.
