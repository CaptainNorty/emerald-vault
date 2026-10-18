#ifndef GUARD_VAULT_TOWN_H
#define GUARD_VAULT_TOWN_H

#include "pokemon.h"
#include "constants/vault_town.h"

// Emerald Vault: save data for one business on the Littleroot farm.
struct TownBuilding
{
    u32 lastSteps;    // GAME_STAT_FARM_STEPS when it was last worked out
    u32 progress;     // Unused (was steps toward the next sale)
    u32 pendingMoney; // Saves from before items only: money not yet collected
    u32 totalSales;
    u32 totalMoney;
    u16 pendingSales;
    bool8 hasWorker;   // Saves from before the FARM box: worker below still to move there
    bool8 everStaffed; // Has had a worker at least once
    struct BoxPokemon worker; // Saves from before the FARM box only
};

// A business's level and stocked items. Kept apart from struct TownBuilding so
// saves from before these existed still line up.
struct TownStock
{
    u32 itemProgress; // Efficiency-weighted steps toward the item being made
    u16 items[TOWN_ITEM_KINDS]; // Uncollected items: which...
    u8 counts[TOWN_ITEM_KINDS]; // ...and how many (0 means the slot is free)
    u8 level;         // 0 or 1 for Lv. 1, up to TOWN_MAX_LEVEL
    u8 nextItem;      // Item table entry + 1 being made from the usual mix; 0: not picked yet
    u8 orderItem;     // Work order: item table entry + 1, made until the stockroom is full; 0 for the usual mix
    u8 unused;
};

// Workers live in the last PC box; see src/vault_town.c.
#define TOWN_FARM_BOX (TOTAL_BOXES_COUNT - 1)

struct TownSaveData
{
    struct TownBuilding buildings[TOWN_BUILDING_COUNT];
    struct TownStock stock[TOWN_BUILDING_COUNT];
};

void ResetTownData(void);
void Town_InitFarmBox(void);
void SetDeliveryServiceFlightHome(void);
void Town_SetWorkerForTest(u8 building, u16 species, u8 level);
void Town_CountStep(void);

#endif // GUARD_VAULT_TOWN_H
