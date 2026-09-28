# Emerald Vault: Vision & Goals

## The pitch

Emerald Vault makes **earning money** a core focus of Pokémon Emerald. After you
beat a trainer, you can hire them. You give your hired trainers Pokémon to
battle with, and they earn money for you over time. Key steps in the game's
progression cost money, so building up an income is how you move forward.

## Core mechanic: hiring trainers

1. **Hire:** After beating a trainer in battle, you can hire them.
2. **Staff:** A hired trainer needs Pokémon to battle with, and you provide them.
3. **Earn:** Once a hired trainer has Pokémon, they build up money over time.
4. **Collect:** You go back to the trainer to collect what they've earned.
5. **Reclaim:** You can take back Pokémon you've given to a hired trainer at any
   time.

### Collecting earnings
- **Early game:** You have to visit each hired trainer in person to collect.
- **Later:** You can pay to unlock a building where you collect earnings from,
  and manage, all your hires in one place. This is one of the investments that
  pays off by saving you time.

### Time is measured in steps
Hired trainers earn money based on **steps you take**. The game already counts
steps (`GAME_STAT_STEPS`, incremented in `src/field_control_avatar.c`), so each
hire can store the step count from their last collection, and their earnings are
worked out from how many steps have passed since then.

### Earning rates
- Early-game trainers earn a little over time, and late-game trainers earn more.
- The game decides **whether a hired trainer wins, and how often**, based on the
  quality of the Pokémon you gave them. Better Pokémon mean more wins and more
  income.

## Progression gated by money

Core progression events are blocked until you pay for them. That gives you the
reason to hire trainers and build income.

### HMs for sale (idea)
All the HMs you need could be for sale in the first city, with costs that rise
exponentially. This teaches you what the game is about right away. Draft prices:

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

## Other income sources (candidates)

Hiring trainers is the core, but there could be 5–10 other ways to earn money.
Current ideas:

- **Training contracts:** An NPC pays a lump sum for training a Pokémon to a
  certain level.
- **Pokémon sales:** An NPC buys specific Pokémon from you.
- **Building repair:** You repair buildings so they start generating income.
- **Gym ownership:** You buy the rights to a Gym.

## Non-goals

The economy systems are a big enough job on their own, so to keep the project
manageable:

- No new Pokémon
- No changes to music or sprites
- Few or no map changes

## Milestones

### First playable
- Hire trainers after beating them.
- Give hired trainers Pokémon.
- Come back to collect their earnings.
- Progression doesn't have to be gated by money yet.

### First release
- Everything in the first playable version.
- Progression gated by money (for example, the HM shop).

## Open questions & technical notes

- **Money cap:** Vanilla Emerald caps money at **$999,999** (`MAX_MONEY` in
  `src/money.c`). Money is already saved as a 32-bit number, so raising the cap
  is a small, save-compatible change, mostly to the displays that assume 6
  digits (the money box and the trainer card). Standard Poké Mart prices are
  16-bit (max $65,535), so expensive purchases like the HM shop should use
  scripted purchases, which already handle 32-bit amounts.
- **HMs still need badges:** Vanilla checks badges before letting you use HM
  field moves (`src/party_menu.c`, `src/field_control_avatar.c`). If HMs are
  bought instead, decide whether those badge checks go away.
- **Waterfall** isn't in the HM price list yet.
- **Step farming:** Earning per step invites players to ride their bike in
  circles. Consider capping how much a hire can hold before you collect
  (like a full container in Stardew Valley).
- **How a hired trainer's win rate is calculated:** What makes a Pokémon
  "better"? Level, stats, type matchups against the trainer's opponents, or a
  mix?
- **Storing lent Pokémon:** Lent Pokémon have to be kept in the save file,
  which has limited space. How many hires, and how many Pokémon each, can the
  save hold? Do lent Pokémon gain experience while they're hired out?
