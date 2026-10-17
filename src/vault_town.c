// Emerald Vault: businesses on the Littleroot Town farm.
//
// Each business earns money as the player walks. Earnings are worked out
// lazily from GAME_STAT_STEPS whenever the player talks to the business, scaled
// by how well suited its assigned worker is (type, level and species).
#include "global.h"
#include "vault_town.h"
#include "event_data.h"
#include "field_effect.h"
#include "fieldmap.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "region_map.h"
#include "string_util.h"
#include "task.h"
#include "constants/characters.h"
#include "constants/game_stat.h"
#include "constants/map_groups.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/species.h"

struct Business
{
    u16 builtFlag;
    u8 preferredType;  // Workers of this type get the best type fit
    u8 secondaryType;  // Workers of this type get a decent type fit
    u16 specialist;    // This species doubles its efficiency
    u16 stepsPerSale;  // Steps per sale for a worker at 1.0x efficiency
    u16 price;         // Money per sale
    u16 stockroomSize; // Uncollected sales stop at this many
};

// Only the Delivery Service is open so far. A zero builtFlag means "not built".
static const struct Business sBusinesses[TOWN_BUILDING_COUNT] =
{
    [TOWN_DELIVERY_SERVICE] =
    {
        .builtFlag = FLAG_DELIVERY_SERVICE_BUILT,
        .preferredType = TYPE_FLYING,
        .secondaryType = TYPE_NORMAL,
        .specialist = SPECIES_SWELLOW,
        .stepsPerSale = 20,
        .price = 12,
        .stockroomSize = 100,
    },
};

// The rebuilt Delivery Service (Brendan's house tiles for now), drawn over its
// ruined plot. Full map grid entries: metatile, collision and elevation.
#define DELIVERY_SERVICE_X 24
#define DELIVERY_SERVICE_Y 4
#define DELIVERY_SERVICE_W 6
#define DELIVERY_SERVICE_H 6
static const u16 sDeliveryServiceTiles[DELIVERY_SERVICE_H][DELIVERY_SERVICE_W] =
{
    {0x3208, 0x3209, 0x3209, 0x3209, 0x320A, 0x3001},
    {0x0610, 0x0611, 0x0611, 0x0611, 0x0612, 0x3001},
    {0x0618, 0x0619, 0x0619, 0x0619, 0x061A, 0x3001},
    {0x0622, 0x0632, 0x0630, 0x0640, 0x0621, 0x3001},
    {0x062A, 0x063A, 0x0638, 0x0648, 0x0629, 0x3001},
    {0x3001, 0x3001, 0x3001, 0x3201, 0x3001, 0x3001},
};

// Where a Delivery Service bird drops the player: in front of the building.
#define DELIVERY_SERVICE_LANDING_X 27
#define DELIVERY_SERVICE_LANDING_Y 10

static struct TownBuilding *GetBuilding(u8 building)
{
    return &gPokemonStoragePtr->town.buildings[building];
}

static bool8 IsValidBuilding(u16 building)
{
    return building < TOWN_BUILDING_COUNT && sBusinesses[building].stepsPerSale != 0;
}

void ResetTownData(void)
{
    memset(&gPokemonStoragePtr->town, 0, sizeof(gPokemonStoragePtr->town));
}

static bool8 SpeciesHasType(u16 species, u8 type)
{
    return gSpeciesInfo[species].types[0] == type || gSpeciesInfo[species].types[1] == type;
}

// Efficiency as a percentage: 100 means 1.0x.
// Type fit (2x preferred, 1.5x secondary, 0.5x other) x level / 10 x 2 for the specialist.
static u32 GetWorkerEfficiency(u8 building)
{
    const struct Business *business = &sBusinesses[building];
    struct TownBuilding *town = GetBuilding(building);
    u16 species;
    u32 typeFit, efficiency;

    if (!town->hasWorker)
        return 0;

    species = GetBoxMonData(&town->worker, MON_DATA_SPECIES);
    if (SpeciesHasType(species, business->preferredType))
        typeFit = 200;
    else if (SpeciesHasType(species, business->secondaryType))
        typeFit = 150;
    else
        typeFit = 50;

    efficiency = typeFit * GetLevelFromBoxMonExp(&town->worker) / 10;
    if (species == business->specialist)
        efficiency *= 2;
    if (efficiency < 5)
        efficiency = 5;
    return efficiency;
}

// Credit the business for the steps walked since it was last settled.
static void SettleBuilding(u8 building)
{
    const struct Business *business = &sBusinesses[building];
    struct TownBuilding *town = GetBuilding(building);
    u32 now = GetGameStat(GAME_STAT_STEPS);
    u32 steps = now - town->lastSteps;
    u32 threshold, sales;

    town->lastSteps = now;
    if (!town->hasWorker || town->pendingSales >= business->stockroomSize)
        return;

    // Cap to avoid overflow; the stockroom fills long before this many steps.
    if (steps > 0xFFFF)
        steps = 0xFFFF;
    threshold = business->stepsPerSale * 100;
    town->progress += steps * GetWorkerEfficiency(building);
    sales = town->progress / threshold;
    town->progress %= threshold;
    if (town->pendingSales + sales >= business->stockroomSize)
    {
        sales = business->stockroomSize - town->pendingSales;
        town->progress = 0;
    }
    town->pendingSales += sales;
    town->pendingMoney += sales * business->price;
}

// Draw every rebuilt business over its ruined plot. Called from Littleroot's
// ON_LOAD map script, and right after a business is rebuilt.
void Town_DrawBuildings(void)
{
    s32 x, y;

    if (FlagGet(FLAG_DELIVERY_SERVICE_BUILT))
    {
        for (y = 0; y < DELIVERY_SERVICE_H; y++)
            for (x = 0; x < DELIVERY_SERVICE_W; x++)
                MapGridSetMetatileEntryAt(DELIVERY_SERVICE_X + x + MAP_OFFSET, DELIVERY_SERVICE_Y + y + MAP_OFFSET,
                                          sDeliveryServiceTiles[y][x]);
    }
}

// VAR_RESULT: whether any non-egg party Pokémon has type VAR_0x8005.
void Town_CheckPartyForType(void)
{
    u8 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && SpeciesHasType(species, gSpecialVar_0x8005))
        {
            gSpecialVar_Result = TRUE;
            return;
        }
    }
}

// VAR_RESULT: whether party slot VAR_0x8004 holds a non-egg Pokémon of type VAR_0x8005.
void Town_IsPartyMonType(void)
{
    struct Pokemon *mon;

    gSpecialVar_Result = FALSE;
    if (gSpecialVar_0x8004 >= PARTY_SIZE)
        return;
    mon = &gPlayerParty[gSpecialVar_0x8004];
    if (!GetMonData(mon, MON_DATA_IS_EGG) && SpeciesHasType(GetMonData(mon, MON_DATA_SPECIES), gSpecialVar_0x8005))
        gSpecialVar_Result = TRUE;
}

// VAR_RESULT: whether business VAR_0x8006 has a worker.
void Town_HasWorker(void)
{
    gSpecialVar_Result = IsValidBuilding(gSpecialVar_0x8006) && GetBuilding(gSpecialVar_0x8006)->hasWorker;
}

// Buffers the worker of business VAR_0x8006: STR_VAR_1 name, STR_VAR_2 level,
// STR_VAR_3 efficiency such as "1.5".
void Town_BufferWorker(void)
{
    struct TownBuilding *town;
    u32 efficiency;
    u8 *end;

    if (!IsValidBuilding(gSpecialVar_0x8006))
        return;
    town = GetBuilding(gSpecialVar_0x8006);
    if (!town->hasWorker)
        return;
    GetBoxMonData(&town->worker, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, GetLevelFromBoxMonExp(&town->worker), STR_CONV_MODE_LEFT_ALIGN, 3);
    efficiency = GetWorkerEfficiency(gSpecialVar_0x8006);
    end = ConvertIntToDecimalStringN(gStringVar3, efficiency / 100, STR_CONV_MODE_LEFT_ALIGN, 3);
    *end++ = CHAR_PERIOD;
    ConvertIntToDecimalStringN(end, (efficiency % 100) / 10, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// Collects the earnings of business VAR_0x8006 into the player's money.
// Buffers STR_VAR_1 sales and STR_VAR_2 money since the last collection, and
// STR_VAR_3 the all-time money earned. VAR_RESULT: whether anything was earned.
void Town_CollectEarnings(void)
{
    struct TownBuilding *town;

    gSpecialVar_Result = FALSE;
    if (!IsValidBuilding(gSpecialVar_0x8006))
        return;
    town = GetBuilding(gSpecialVar_0x8006);
    SettleBuilding(gSpecialVar_0x8006);

    ConvertIntToDecimalStringN(gStringVar1, town->pendingSales, STR_CONV_MODE_LEFT_ALIGN, 5);
    ConvertIntToDecimalStringN(gStringVar2, town->pendingMoney, STR_CONV_MODE_LEFT_ALIGN, 10);
    if (town->pendingMoney != 0)
    {
        AddMoney(&gSaveBlock1Ptr->money, town->pendingMoney);
        town->totalSales += town->pendingSales;
        town->totalMoney += town->pendingMoney;
        town->pendingSales = 0;
        town->pendingMoney = 0;
        gSpecialVar_Result = TRUE;
    }
    ConvertIntToDecimalStringN(gStringVar3, town->totalMoney, STR_CONV_MODE_LEFT_ALIGN, 10);
}

static u8 CountOtherUsablePartyMons(u8 slot)
{
    u8 i, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i != slot && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            count++;
    }
    return count;
}

// Makes party slot VAR_0x8004 the worker of business VAR_0x8006. A previous
// worker swaps into that party slot. VAR_RESULT: a TOWN_ASSIGN_* result.
void Town_AssignWorker(void)
{
    struct TownBuilding *town;
    struct Pokemon *mon;

    if (!IsValidBuilding(gSpecialVar_0x8006) || gSpecialVar_0x8004 >= PARTY_SIZE)
    {
        gSpecialVar_Result = TOWN_ASSIGN_CANCELLED;
        return;
    }
    town = GetBuilding(gSpecialVar_0x8006);
    mon = &gPlayerParty[gSpecialVar_0x8004];
    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = TOWN_ASSIGN_EGG;
        return;
    }
    if (!town->hasWorker && CountOtherUsablePartyMons(gSpecialVar_0x8004) == 0)
    {
        gSpecialVar_Result = TOWN_ASSIGN_LAST_MON;
        return;
    }

    // Credit the outgoing worker for the steps walked so far.
    SettleBuilding(gSpecialVar_0x8006);
    if (town->hasWorker)
    {
        struct BoxPokemon previous = town->worker;
        town->worker = mon->box;
        BoxMonToMon(&previous, mon);
    }
    else
    {
        town->worker = mon->box;
        town->hasWorker = TRUE;
        ZeroMonData(mon);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    gSpecialVar_Result = TOWN_ASSIGN_OK;
}

// Returns the worker of business VAR_0x8006 to the party.
// VAR_RESULT: FALSE if the party is full.
void Town_TakeBackWorker(void)
{
    struct TownBuilding *town;

    gSpecialVar_Result = FALSE;
    if (!IsValidBuilding(gSpecialVar_0x8006))
        return;
    town = GetBuilding(gSpecialVar_0x8006);
    if (!town->hasWorker || CalculatePlayerPartyCount() >= PARTY_SIZE)
        return;

    SettleBuilding(gSpecialVar_0x8006);
    BoxMonToMon(&town->worker, &gPlayerParty[gPlayerPartyCount]);
    ZeroBoxMonData(&town->worker);
    town->hasWorker = FALSE;
    CalculatePlayerPartyCount();
    gSpecialVar_Result = TRUE;
}

void SetDeliveryServiceFlightHome(void)
{
    SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), WARP_ID_NONE,
                       DELIVERY_SERVICE_LANDING_X, DELIVERY_SERVICE_LANDING_Y);
}

static void Task_OpenDeliveryServiceFlyMap(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        StartDeliveryServiceFlyMap();
        SetMainCallback2(CB2_OpenFlyMap);
        DestroyTask(taskId);
    }
}

// Opens the Fly map so a Delivery Service bird can take the player to any
// visited town. Used with waitstate: cancelling returns to the script.
void Town_OpenFlyMap(void)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_OpenDeliveryServiceFlyMap, 10);
}
