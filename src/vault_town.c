// Emerald Vault: businesses on the Littleroot Town farm.
//
// The Builder rebuilds ruined plots into businesses. Each business earns money
// and stocks items as the player walks, and its worker gains EXP. Everything is
// worked out lazily from GAME_STAT_STEPS whenever the player uses its terminal,
// scaled by how well suited its assigned worker is (type, level and species).
#include "global.h"
#include "vault_town.h"
#include "event_data.h"
#include "field_effect.h"
#include "fieldmap.h"
#include "item.h"
#include "random.h"
#include "international_string_util.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "region_map.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/game_stat.h"
#include "constants/items.h"
#include "constants/map_event_ids.h"
#include "constants/map_groups.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"

#define STEPS_PER_SALE 20  // Steps per sale for a worker at 1.0x efficiency
#define STOCKROOM_SIZE 100 // Uncollected sales stop at this many
#define ITEM_STOCKROOM_SIZE 30 // Uncollected items stop at this many
#define WORKER_EXP_PER_STEP 1  // Like the Day Care

// One kind of item a business stocks, and how often it comes up.
struct TownItem
{
    u16 item;
    u8 weight;
};

struct Business
{
    const u8 *name;
    const u8 *workerTypes; // Shown to the player, e.g. "GRASS or BUG"
    const u8 *saleVerb;    // "sold" in "sold 12 BERRIES"
    const u8 *product;
    u32 cost;
    u16 price;             // Money per sale
    u16 itemSteps;         // Steps per item for a worker at 1.0x efficiency
    const struct TownItem *items; // Up to TOWN_ITEM_KINDS, ending with ITEM_NONE
    u16 builtFlag;
    u16 unlockFlag;        // Needed besides money and a worker; 0 for none
    u16 terminalHideFlag;
    u16 terminalLocalId;
    u8 types[3];           // Worker types: 2x efficiency; the first worker must have one
    u8 bonusType;          // 1.5x efficiency
    u16 specialists[2];    // Species that double their efficiency
    bool8 hiddenWithoutWorker; // Only offered once the party has a suitable worker
};

struct BuildingTiles
{
    const u16 *tiles;
    u8 x, y, width, height;
};

#include "data/vault_town_buildings.h"

static const u8 sText_DeliveryService[] = _("DELIVERY SERVICE");
static const u8 sText_Greenhouse[] = _("GREENHOUSE");
static const u8 sText_Quarry[] = _("QUARRY");
static const u8 sText_Fishery[] = _("FISHERY");
static const u8 sText_Generator[] = _("GENERATOR");
static const u8 sText_Forge[] = _("FORGE");
static const u8 sText_SpiritSanctum[] = _("SPIRIT SANCTUM");
static const u8 sText_IceDome[] = _("ICE DOME");

static const u8 sText_TypesFlying[] = _("FLYING");
static const u8 sText_TypesGrassBug[] = _("GRASS or BUG");
static const u8 sText_TypesRockGround[] = _("ROCK or GROUND");
static const u8 sText_TypesWaterDark[] = _("WATER or DARK");
static const u8 sText_TypesElectricSteel[] = _("ELECTRIC or STEEL");
static const u8 sText_TypesFireFighting[] = _("FIRE or FIGHTING");
static const u8 sText_TypesGhostPsychicPoison[] = _("GHOST, PSYCHIC or POISON");
static const u8 sText_TypesIce[] = _("ICE");

static const u8 sText_Made[] = _("made");
static const u8 sText_Sold[] = _("sold");
static const u8 sText_CaughtAndSold[] = _("caught and sold");

static const u8 sText_Deliveries[] = _("deliveries");
static const u8 sText_Berries[] = _("BERRIES");
static const u8 sText_EvolutionStones[] = _("EVOLUTION STONES");
static const u8 sText_Pokemon[] = _("POKéMON");
static const u8 sText_PowerCells[] = _("POWER CELLS");
static const u8 sText_PokeBalls[] = _("POKé BALLS");
static const u8 sText_Charms[] = _("CHARMS");
static const u8 sText_IceBlocks[] = _("ICE BLOCKS");

// Items each business stocks. Tier 1 for now; upgrades will add better ones.
static const struct TownItem sDeliveryServiceItems[] =
{
    {ITEM_POTION, 35},
    {ITEM_POKE_BALL, 30},
    {ITEM_ANTIDOTE, 15},
    {ITEM_REPEL, 12},
    {ITEM_ESCAPE_ROPE, 8},
    {ITEM_NONE},
};

static const struct TownItem sGreenhouseItems[] =
{
    {ITEM_ORAN_BERRY, 30},
    {ITEM_PECHA_BERRY, 15},
    {ITEM_CHERI_BERRY, 15},
    {ITEM_RAWST_BERRY, 10},
    {ITEM_CHESTO_BERRY, 10},
    {ITEM_ASPEAR_BERRY, 10},
    {ITEM_PERSIM_BERRY, 5},
    {ITEM_SITRUS_BERRY, 5},
    {ITEM_NONE},
};

static const struct TownItem sQuarryItems[] =
{
    {ITEM_STARDUST, 30},
    {ITEM_HARD_STONE, 15},
    {ITEM_EVERSTONE, 15},
    {ITEM_SOFT_SAND, 15},
    {ITEM_FIRE_STONE, 5},
    {ITEM_WATER_STONE, 5},
    {ITEM_THUNDER_STONE, 5},
    {ITEM_LEAF_STONE, 5},
    {ITEM_NONE},
};

static const struct TownItem sFisheryItems[] =
{
    {ITEM_PEARL, 30},
    {ITEM_SHOAL_SALT, 20},
    {ITEM_SHOAL_SHELL, 15},
    {ITEM_HEART_SCALE, 15},
    {ITEM_NET_BALL, 10},
    {ITEM_DIVE_BALL, 5},
    {ITEM_BIG_PEARL, 5},
    {ITEM_NONE},
};

static const struct TownItem sGeneratorItems[] =
{
    {ITEM_SUPER_POTION, 30},
    {ITEM_SUPER_REPEL, 25},
    {ITEM_PARALYZE_HEAL, 20},
    {ITEM_AWAKENING, 15},
    {ITEM_MAGNET, 5},
    {ITEM_THUNDER_STONE, 5},
    {ITEM_NONE},
};

static const struct TownItem sForgeItems[] =
{
    {ITEM_GREAT_BALL, 40},
    {ITEM_ULTRA_BALL, 15},
    {ITEM_TIMER_BALL, 10},
    {ITEM_NEST_BALL, 10},
    {ITEM_REPEAT_BALL, 10},
    {ITEM_BURN_HEAL, 10},
    {ITEM_CHARCOAL, 5},
    {ITEM_NONE},
};

static const struct TownItem sSpiritSanctumItems[] =
{
    {ITEM_REVIVE, 30},
    {ITEM_FULL_HEAL, 25},
    {ITEM_HYPER_POTION, 20},
    {ITEM_CLEANSE_TAG, 10},
    {ITEM_SPELL_TAG, 10},
    {ITEM_MAX_REVIVE, 5},
    {ITEM_NONE},
};

static const struct TownItem sIceDomeItems[] =
{
    {ITEM_MAX_POTION, 25},
    {ITEM_MAX_REPEL, 20},
    {ITEM_ICE_HEAL, 20},
    {ITEM_FULL_RESTORE, 15},
    {ITEM_ULTRA_BALL, 15},
    {ITEM_NEVER_MELT_ICE, 5},
    {ITEM_NONE},
};

// Money per sale scales with each building's cost; the Greenhouse earns twice
// the Delivery Service's rate. The Weather Stabilizer isn't for sale yet.
static const struct Business sBusinesses[TOWN_BUILDING_COUNT] =
{
    [TOWN_DELIVERY_SERVICE] =
    {
        .name = sText_DeliveryService,
        .workerTypes = sText_TypesFlying,
        .saleVerb = sText_Made,
        .product = sText_Deliveries,
        .cost = 0, // The Builder's free first building
        .price = 12,
        .itemSteps = 50,
        .items = sDeliveryServiceItems,
        .builtFlag = FLAG_DELIVERY_SERVICE_BUILT,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_DELIVERY_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_DELIVERY_TERMINAL,
        .types = {TYPE_FLYING, TYPE_NONE, TYPE_NONE},
        .bonusType = TYPE_NORMAL,
        .specialists = {SPECIES_SWELLOW, SPECIES_NONE},
    },
    [TOWN_GREENHOUSE] =
    {
        .name = sText_Greenhouse,
        .workerTypes = sText_TypesGrassBug,
        .saleVerb = sText_Sold,
        .product = sText_Berries,
        .cost = 5000,
        .price = 24,
        .itemSteps = 50,
        .items = sGreenhouseItems,
        .builtFlag = FLAG_GREENHOUSE_BUILT,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_GREENHOUSE_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_GREENHOUSE_TERMINAL,
        .types = {TYPE_GRASS, TYPE_BUG, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_LUDICOLO, SPECIES_NONE},
    },
    [TOWN_QUARRY] =
    {
        .name = sText_Quarry,
        .workerTypes = sText_TypesRockGround,
        .saleVerb = sText_Sold,
        .product = sText_EvolutionStones,
        .cost = 15000,
        .price = 72,
        .itemSteps = 50,
        .items = sQuarryItems,
        .builtFlag = FLAG_QUARRY_BUILT,
        .unlockFlag = FLAG_BADGE01_GET,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_QUARRY_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_QUARRY_TERMINAL,
        .types = {TYPE_ROCK, TYPE_GROUND, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_GOLEM, SPECIES_NONE},
    },
    [TOWN_FISHERY] =
    {
        .name = sText_Fishery,
        .workerTypes = sText_TypesWaterDark,
        .saleVerb = sText_CaughtAndSold,
        .product = sText_Pokemon,
        .cost = 45000,
        .price = 216,
        .itemSteps = 50,
        .items = sFisheryItems,
        .builtFlag = FLAG_FISHERY_BUILT,
        .unlockFlag = FLAG_RECEIVED_OLD_ROD,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_FISHERY_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_FISHERY_TERMINAL,
        .types = {TYPE_WATER, TYPE_DARK, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_SHARPEDO, SPECIES_NONE},
    },
    [TOWN_GENERATOR] =
    {
        .name = sText_Generator,
        .workerTypes = sText_TypesElectricSteel,
        .saleVerb = sText_Sold,
        .product = sText_PowerCells,
        .cost = 80000,
        .price = 384,
        .itemSteps = 50,
        .items = sGeneratorItems,
        .builtFlag = FLAG_GENERATOR_BUILT,
        .unlockFlag = FLAG_BADGE03_GET,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_GENERATOR_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_GENERATOR_TERMINAL,
        .types = {TYPE_ELECTRIC, TYPE_STEEL, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_MAGNETON, SPECIES_NONE},
    },
    [TOWN_FORGE] =
    {
        .name = sText_Forge,
        .workerTypes = sText_TypesFireFighting,
        .saleVerb = sText_Sold,
        .product = sText_PokeBalls,
        .cost = 250000,
        .price = 1200,
        .itemSteps = 50,
        .items = sForgeItems,
        .builtFlag = FLAG_FORGE_BUILT,
        .unlockFlag = FLAG_BADGE04_GET,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_FORGE_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_FORGE_TERMINAL,
        .types = {TYPE_FIRE, TYPE_FIGHTING, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_BLAZIKEN, SPECIES_NONE},
    },
    [TOWN_SPIRIT_SANCTUM] =
    {
        .name = sText_SpiritSanctum,
        .workerTypes = sText_TypesGhostPsychicPoison,
        .saleVerb = sText_Sold,
        .product = sText_Charms,
        .cost = 750000,
        .price = 3600,
        .itemSteps = 50,
        .items = sSpiritSanctumItems,
        .builtFlag = FLAG_SPIRIT_SANCTUM_BUILT,
        .unlockFlag = FLAG_VISITED_MT_PYRE,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_SPIRIT_SANCTUM_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_SPIRIT_SANCTUM_TERMINAL,
        .types = {TYPE_GHOST, TYPE_PSYCHIC, TYPE_POISON},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_DUSCLOPS, SPECIES_CLAYDOL},
    },
    [TOWN_ICE_DOME] =
    {
        .name = sText_IceDome,
        .workerTypes = sText_TypesIce,
        .saleVerb = sText_Sold,
        .product = sText_IceBlocks,
        .cost = 1500000,
        .price = 7200,
        .itemSteps = 50,
        .items = sIceDomeItems,
        .builtFlag = FLAG_ICE_DOME_BUILT,
        .terminalHideFlag = FLAG_HIDE_LITTLEROOT_ICE_DOME_TERMINAL,
        .terminalLocalId = LOCALID_LITTLEROOT_ICE_DOME_TERMINAL,
        .types = {TYPE_ICE, TYPE_NONE, TYPE_NONE},
        .bonusType = TYPE_NONE,
        .specialists = {SPECIES_GLALIE, SPECIES_NONE},
        .hiddenWithoutWorker = TRUE, // Hoenn's Ice-types live in Shoal Cave
    },
};

// Where a Delivery Service bird drops the player: in front of the building.
#define DELIVERY_SERVICE_LANDING_X 27
#define DELIVERY_SERVICE_LANDING_Y 10

static struct TownBuilding *GetBuilding(u8 building)
{
    return &gPokemonStoragePtr->town.buildings[building];
}

static struct TownStock *GetStock(u8 building)
{
    return &gPokemonStoragePtr->town.stock[building];
}

static bool8 IsBusiness(u16 building)
{
    return building < TOWN_BUILDING_COUNT && sBusinesses[building].name != NULL;
}

static bool8 IsBuilt(u8 building)
{
    return IsBusiness(building) && FlagGet(sBusinesses[building].builtFlag);
}

void ResetTownData(void)
{
    memset(&gPokemonStoragePtr->town, 0, sizeof(gPokemonStoragePtr->town));
}

static bool8 SpeciesHasType(u16 species, u8 type)
{
    return type != TYPE_NONE && (gSpeciesInfo[species].types[0] == type || gSpeciesInfo[species].types[1] == type);
}

static bool8 IsSuitedWorker(u8 building, u16 species)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sBusinesses[building].types); i++)
    {
        if (SpeciesHasType(species, sBusinesses[building].types[i]))
            return TRUE;
    }
    return FALSE;
}

static bool8 PartyHasSuitedWorker(u8 building)
{
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && IsSuitedWorker(building, species))
            return TRUE;
    }
    return FALSE;
}

// Efficiency as a percentage: 100 means 1.0x. Type fit (2x for the business's
// types, 1.5x for its bonus type, 0.5x otherwise) x level / 10, doubled for a
// specialist species.
static u32 GetWorkerEfficiency(u8 building)
{
    const struct Business *business = &sBusinesses[building];
    struct TownBuilding *town = GetBuilding(building);
    u16 species;
    u32 typeFit, efficiency;

    if (!town->hasWorker)
        return 0;

    species = GetBoxMonData(&town->worker, MON_DATA_SPECIES);
    if (IsSuitedWorker(building, species))
        typeFit = 200;
    else if (SpeciesHasType(species, business->bonusType))
        typeFit = 150;
    else
        typeFit = 50;

    efficiency = typeFit * GetLevelFromBoxMonExp(&town->worker) / 10;
    if (species == business->specialists[0] || species == business->specialists[1])
        efficiency *= 2;
    if (efficiency < 5)
        efficiency = 5;
    return efficiency;
}

static u32 CountStockedItems(struct TownStock *stock)
{
    u32 i, count = 0;

    for (i = 0; i < TOWN_ITEM_KINDS; i++)
        count += stock->itemCounts[i];
    return count;
}

// Picks an entry of the business's item table, weighted.
static u8 RollItem(u8 building)
{
    const struct TownItem *items = sBusinesses[building].items;
    u32 i, total = 0, roll;

    for (i = 0; i < TOWN_ITEM_KINDS && items[i].item != ITEM_NONE; i++)
        total += items[i].weight;
    roll = Random() % total;
    for (i = 0; roll >= items[i].weight; i++)
        roll -= items[i].weight;
    return i;
}

static void SettleItems(u8 building, u32 steps, u32 efficiency)
{
    struct TownStock *stock = GetStock(building);
    u32 threshold = sBusinesses[building].itemSteps * 100;
    u32 stocked = CountStockedItems(stock);

    if (sBusinesses[building].items == NULL)
        return;
    stock->itemProgress += steps * efficiency;
    while (stock->itemProgress >= threshold && stocked < ITEM_STOCKROOM_SIZE)
    {
        stock->itemProgress -= threshold;
        stock->itemCounts[RollItem(building)]++;
        stocked++;
    }
    if (stocked >= ITEM_STOCKROOM_SIZE)
        stock->itemProgress = 0;
}

// Working earns the worker EXP, like the Day Care (but no new moves).
static void GiveWorkerExp(struct BoxPokemon *worker, u32 steps)
{
    u16 species = GetBoxMonData(worker, MON_DATA_SPECIES);
    u32 exp = GetBoxMonData(worker, MON_DATA_EXP) + steps * WORKER_EXP_PER_STEP;
    u32 maxExp = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];

    if (exp > maxExp)
        exp = maxExp;
    SetBoxMonData(worker, MON_DATA_EXP, &exp);
}

// Credit the business for the steps walked since it was last settled: sales,
// items, and EXP for the worker.
static void SettleBuilding(u8 building)
{
    struct TownBuilding *town = GetBuilding(building);
    u32 now = GetGameStat(GAME_STAT_STEPS);
    u32 steps = now - town->lastSteps;
    u32 threshold = STEPS_PER_SALE * 100;
    u32 sales, efficiency;

    town->lastSteps = now;
    if (!town->hasWorker)
        return;

    // Cap to avoid overflow; the stockrooms fill long before this many steps.
    if (steps > 0xFFFF)
        steps = 0xFFFF;
    efficiency = GetWorkerEfficiency(building);
    GiveWorkerExp(&town->worker, steps);
    SettleItems(building, steps, efficiency);
    if (town->pendingSales >= STOCKROOM_SIZE)
        return;

    town->progress += steps * efficiency;
    sales = town->progress / threshold;
    town->progress %= threshold;
    if (town->pendingSales + sales >= STOCKROOM_SIZE)
    {
        sales = STOCKROOM_SIZE - town->pendingSales;
        town->progress = 0;
    }
    town->pendingSales += sales;
    town->pendingMoney += sales * sBusinesses[building].price;
}

// Draw every rebuilt business over its ruined plot. Called from Littleroot's
// ON_LOAD map script, and right after a business is rebuilt.
void Town_DrawBuildings(void)
{
    u8 building;
    s32 x, y;

    for (building = 0; building < TOWN_BUILDING_COUNT; building++)
    {
        const struct BuildingTiles *art = &sBuildingTiles[building];
        if (!IsBuilt(building) || art->tiles == NULL)
            continue;
        for (y = 0; y < art->height; y++)
            for (x = 0; x < art->width; x++)
                MapGridSetMetatileEntryAt(art->x + x + MAP_OFFSET, art->y + y + MAP_OFFSET,
                                          art->tiles[y * art->width + x]);
    }
}

// Show the managers of rebuilt businesses. Called from Littleroot's
// ON_TRANSITION map script, before objects are spawned.
void Town_UpdateTerminals(void)
{
    u8 building;

    for (building = 0; building < TOWN_BUILDING_COUNT; building++)
    {
        if (!IsBusiness(building))
            continue;
        if (IsBuilt(building))
            FlagClear(sBusinesses[building].terminalHideFlag);
        else
            FlagSet(sBusinesses[building].terminalHideFlag);
    }
}

// Buffers business VAR_0x8006: STR_VAR_1 name, STR_VAR_2 cost, STR_VAR_3 worker types.
void Town_BufferBuildingInfo(void)
{
    const struct Business *business;

    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    business = &sBusinesses[gSpecialVar_0x8006];
    StringCopy(gStringVar1, business->name);
    ConvertIntToDecimalStringN(gStringVar2, business->cost, STR_CONV_MODE_LEFT_ALIGN, 7);
    StringCopy(gStringVar3, business->workerTypes);
}

// VAR_RESULT: whether the player can pay for business VAR_0x8006.
void Town_CanAffordBuilding(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006)
                      && IsEnoughMoney(&gSaveBlock1Ptr->money, sBusinesses[gSpecialVar_0x8006].cost);
}

// VAR_RESULT: whether the party has a Pokémon suited to work at business VAR_0x8006.
void Town_CheckPartyForWorker(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && PartyHasSuitedWorker(gSpecialVar_0x8006);
}

// Pays for and rebuilds business VAR_0x8006. VAR_0x8007: its manager's local ID.
void Town_BuyBuilding(void)
{
    const struct Business *business;

    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    business = &sBusinesses[gSpecialVar_0x8006];
    RemoveMoney(&gSaveBlock1Ptr->money, business->cost);
    FlagSet(business->builtFlag);
    FlagClear(business->terminalHideFlag);
    gSpecialVar_0x8007 = business->terminalLocalId;
}

// VAR_RESULT: whether party slot VAR_0x8004 holds a non-egg Pokémon suited to
// work at business VAR_0x8006.
void Town_IsPartyMonSuited(void)
{
    struct Pokemon *mon;

    gSpecialVar_Result = FALSE;
    if (!IsBusiness(gSpecialVar_0x8006) || gSpecialVar_0x8004 >= PARTY_SIZE)
        return;
    mon = &gPlayerParty[gSpecialVar_0x8004];
    if (!GetMonData(mon, MON_DATA_IS_EGG) && IsSuitedWorker(gSpecialVar_0x8006, GetMonData(mon, MON_DATA_SPECIES)))
        gSpecialVar_Result = TRUE;
}

// VAR_RESULT: whether business VAR_0x8006 has a worker.
void Town_HasWorker(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && GetBuilding(gSpecialVar_0x8006)->hasWorker;
}

// VAR_RESULT: whether business VAR_0x8006 has ever had a worker.
void Town_WasEverStaffed(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && GetBuilding(gSpecialVar_0x8006)->everStaffed;
}

// Buffers the worker of business VAR_0x8006: STR_VAR_1 name, STR_VAR_2 level,
// STR_VAR_3 efficiency such as "1.5".
void Town_BufferWorker(void)
{
    struct TownBuilding *town;
    u32 efficiency;
    u8 *end;

    if (!IsBusiness(gSpecialVar_0x8006))
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
// Buffers STR_VAR_3 what was sold (e.g. "sold 12 BERRIES") and STR_VAR_2 the
// money since the last collection. VAR_RESULT: whether anything was earned.
void Town_CollectEarnings(void)
{
    const struct Business *business;
    struct TownBuilding *town;
    u8 *end;

    gSpecialVar_Result = FALSE;
    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    business = &sBusinesses[gSpecialVar_0x8006];
    town = GetBuilding(gSpecialVar_0x8006);
    SettleBuilding(gSpecialVar_0x8006);

    end = StringCopy(gStringVar3, business->saleVerb);
    *end++ = CHAR_SPACE;
    end = ConvertIntToDecimalStringN(end, town->pendingSales, STR_CONV_MODE_LEFT_ALIGN, 5);
    *end++ = CHAR_SPACE;
    StringCopy(end, business->product);
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
}

// For the terminal's status line: settles business VAR_0x8006, then buffers
// STR_VAR_1 the uncollected money and STR_VAR_2 the number of uncollected
// items. VAR_0x8008: the worker's species, for showmonpic. VAR_RESULT: whether
// there's anything to collect.
void Town_BufferStock(void)
{
    struct TownBuilding *town;
    u32 items;

    gSpecialVar_Result = FALSE;
    gSpecialVar_0x8008 = SPECIES_NONE;
    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    town = GetBuilding(gSpecialVar_0x8006);
    SettleBuilding(gSpecialVar_0x8006);
    items = CountStockedItems(GetStock(gSpecialVar_0x8006));
    ConvertIntToDecimalStringN(gStringVar1, town->pendingMoney, STR_CONV_MODE_LEFT_ALIGN, 10);
    ConvertIntToDecimalStringN(gStringVar2, items, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (town->hasWorker)
        gSpecialVar_0x8008 = GetBoxMonData(&town->worker, MON_DATA_SPECIES);
    gSpecialVar_Result = (town->pendingMoney != 0 || items != 0);
}

// Moves the next kind of stocked item from business VAR_0x8006 into the bag,
// starting from item table entry VAR_0x8009 (set it to 0 first). Buffers
// STR_VAR_1 the item name and STR_VAR_2 how many. VAR_RESULT: a TOWN_ITEM_*
// result; items that don't fit in the bag stay in stock.
void Town_CollectNextItem(void)
{
    const struct TownItem *items;
    struct TownStock *stock;
    u16 i;

    gSpecialVar_Result = TOWN_ITEMS_DONE;
    if (!IsBusiness(gSpecialVar_0x8006) || sBusinesses[gSpecialVar_0x8006].items == NULL)
        return;
    items = sBusinesses[gSpecialVar_0x8006].items;
    stock = GetStock(gSpecialVar_0x8006);
    for (i = gSpecialVar_0x8009; i < TOWN_ITEM_KINDS && items[i].item != ITEM_NONE; i++)
    {
        u8 count = stock->itemCounts[i];
        if (count == 0)
            continue;

        gSpecialVar_0x8009 = i + 1;
        CopyItemNameHandlePlural(items[i].item, gStringVar1, count);
        ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (AddBagItem(items[i].item, count))
        {
            stock->itemCounts[i] = 0;
            gSpecialVar_Result = TOWN_ITEM_COLLECTED;
        }
        else
        {
            gSpecialVar_Result = TOWN_ITEM_NO_ROOM;
        }
        return;
    }
}

// Buffers STR_VAR_1: all the money business VAR_0x8006 has earned.
void Town_BufferTotalEarnings(void)
{
    if (IsBusiness(gSpecialVar_0x8006))
        ConvertIntToDecimalStringN(gStringVar1, GetBuilding(gSpecialVar_0x8006)->totalMoney, STR_CONV_MODE_LEFT_ALIGN, 10);
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

    if (!IsBusiness(gSpecialVar_0x8006) || gSpecialVar_0x8004 >= PARTY_SIZE)
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
    town->everStaffed = TRUE;
    gSpecialVar_Result = TOWN_ASSIGN_OK;
}

// Returns the worker of business VAR_0x8006 to the party.
// VAR_RESULT: FALSE if the party is full.
void Town_TakeBackWorker(void)
{
    struct TownBuilding *town;

    gSpecialVar_Result = FALSE;
    if (!IsBusiness(gSpecialVar_0x8006))
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

#if TEST_START
// Test-only ROM: start with a business already staffed.
void Town_SetWorkerForTest(u8 building, u16 species, u8 level)
{
    struct TownBuilding *town;
    struct Pokemon mon;

    if (!IsBusiness(building))
        return;
    town = GetBuilding(building);
    CreateMon(&mon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    town->worker = mon.box;
    town->hasWorker = TRUE;
    town->everStaffed = TRUE;
    town->lastSteps = GetGameStat(GAME_STAT_STEPS);
}
#endif

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

// ---------------------------------------------------------------------------
// The Builder's menu: businesses that can be rebuilt right now, with costs.
// ---------------------------------------------------------------------------

#define BUILDER_MENU_WIDTH 19
#define BUILDER_MENU_ROWS  4
#define BUILDER_MENU_EXIT  TOWN_BUILDING_COUNT

#define tListTaskId data[0]
#define tWindowId   data[1]

static EWRAM_DATA struct ListMenuItem *sBuilderMenuItems = NULL;

static const u8 sBuilderMenuTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

// The Builder lists businesses cheapest first.
static const u8 sBuilderMenuOrder[] =
{
    TOWN_GREENHOUSE,
    TOWN_QUARRY,
    TOWN_FISHERY,
    TOWN_GENERATOR,
    TOWN_FORGE,
    TOWN_SPIRIT_SANCTUM,
    TOWN_ICE_DOME,
};

static bool8 IsOfferedByBuilder(u8 building)
{
    const struct Business *business = &sBusinesses[building];

    if (!IsBusiness(building) || business->cost == 0 || IsBuilt(building))
        return FALSE;
    if (business->unlockFlag != 0 && !FlagGet(business->unlockFlag))
        return FALSE;
    if (business->hiddenWithoutWorker && !PartyHasSuitedWorker(building))
        return FALSE;
    return TRUE;
}

// VAR_RESULT: whether the Builder can rebuild anything right now.
void Town_HasBuilderOffers(void)
{
    u8 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < ARRAY_COUNT(sBuilderMenuOrder); i++)
    {
        if (IsOfferedByBuilder(sBuilderMenuOrder[i]))
        {
            gSpecialVar_Result = TRUE;
            return;
        }
    }
}

static void Task_ResumeScript(u8 taskId)
{
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Prints each business's cost at the right edge of the menu.
static void BuilderMenu_PrintCost(u8 windowId, u32 itemId, u8 y)
{
    if (itemId >= TOWN_BUILDING_COUNT)
        return;
    ConvertIntToDecimalStringN(gStringVar1, sBusinesses[itemId].cost, STR_CONV_MODE_LEFT_ALIGN, 7);
    StringExpandPlaceholders(gStringVar4, gText_PokedollarVar1);
    AddTextPrinterParameterized4(windowId, FONT_NORMAL,
                                 GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, BUILDER_MENU_WIDTH * 8 - 2),
                                 y, 0, 0, sBuilderMenuTextColors, TEXT_SKIP_DRAW, gStringVar4);
}

static void Task_BuilderMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 input = ListMenu_ProcessInput(task->tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL || input == BUILDER_MENU_EXIT)
        gSpecialVar_Result = TOWN_MENU_CANCELLED;
    else
        gSpecialVar_Result = input;

    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    Free(sBuilderMenuItems);
    sBuilderMenuItems = NULL;
    ClearStdWindowAndFrameToTransparent(task->tWindowId, TRUE);
    RemoveWindow(task->tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Lists the businesses the Builder can rebuild now. Used with waitstate; check
// Town_HasBuilderOffers first. VAR_RESULT: the chosen business,
// TOWN_MENU_CANCELLED, or TOWN_MENU_EMPTY if nothing can be rebuilt.
void Town_ShowBuilderMenu(void)
{
    struct ListMenuTemplate list;
    struct WindowTemplate window;
    u8 i, building, count = 0, taskId, windowId, rows;

    sBuilderMenuItems = AllocZeroed((ARRAY_COUNT(sBuilderMenuOrder) + 1) * sizeof(*sBuilderMenuItems));
    for (i = 0; i < ARRAY_COUNT(sBuilderMenuOrder); i++)
    {
        building = sBuilderMenuOrder[i];
        if (IsOfferedByBuilder(building))
        {
            sBuilderMenuItems[count].name = sBusinesses[building].name;
            sBuilderMenuItems[count].id = building;
            count++;
        }
    }
    if (count == 0)
    {
        Free(sBuilderMenuItems);
        sBuilderMenuItems = NULL;
        gSpecialVar_Result = TOWN_MENU_EMPTY;
        // Resume on the next frame, once the script is waiting.
        CreateTask(Task_ResumeScript, 80);
        return;
    }
    sBuilderMenuItems[count].name = gText_Exit;
    sBuilderMenuItems[count].id = BUILDER_MENU_EXIT;
    count++;

    rows = min(count, BUILDER_MENU_ROWS);
    window = CreateWindowTemplate(0, 1, 1, BUILDER_MENU_WIDTH, rows * 2, 15, 0x64);
    windowId = AddWindow(&window);
    SetStandardWindowBorderStyle(windowId, FALSE);

    list.items = sBuilderMenuItems;
    list.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    list.itemPrintFunc = BuilderMenu_PrintCost;
    list.totalItems = count;
    list.maxShowed = rows;
    list.windowId = windowId;
    list.header_X = 0;
    list.item_X = 8;
    list.cursor_X = 0;
    list.upText_Y = 1;
    list.cursorPal = 2;
    list.fillValue = 1;
    list.cursorShadowPal = 3;
    list.lettersSpacing = 0;
    list.itemVerticalPadding = 0;
    list.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    list.fontId = FONT_NORMAL;
    list.cursorKind = CURSOR_BLACK_ARROW;

    taskId = CreateTask(Task_BuilderMenu, 80);
    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tListTaskId = ListMenuInit(&list, 0, 0);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

#undef tListTaskId
#undef tWindowId
