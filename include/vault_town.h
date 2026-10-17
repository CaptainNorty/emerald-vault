#ifndef GUARD_VAULT_TOWN_H
#define GUARD_VAULT_TOWN_H

#include "pokemon.h"
#include "constants/vault_town.h"

// Emerald Vault: save data for one business on the Littleroot farm.
struct TownBuilding
{
    u32 lastSteps;    // GAME_STAT_STEPS when earnings were last worked out
    u32 progress;     // Efficiency-weighted steps toward the next sale
    u32 pendingMoney; // Earned but not yet collected
    u32 totalSales;
    u32 totalMoney;
    u16 pendingSales;
    bool8 hasWorker;
    bool8 everStaffed; // Has had a worker at least once
    struct BoxPokemon worker;
};

struct TownSaveData
{
    struct TownBuilding buildings[TOWN_BUILDING_COUNT];
};

void ResetTownData(void);
void SetDeliveryServiceFlightHome(void);
void Town_SetWorkerForTest(u8 building, u16 species, u8 level);

#endif // GUARD_VAULT_TOWN_H
