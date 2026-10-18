// Emerald Vault: businesses on the Littleroot Town farm.
//
// The Builder rebuilds ruined plots into businesses and upgrades them (Lv. 1 to
// 3; each level adds a worker slot and a better item tier). Each business earns
// money and stocks items as the player walks, and its workers gain EXP.
// Everything is worked out lazily from GAME_STAT_STEPS whenever the player uses
// its terminal, scaled by how well suited its workers are (type, level and
// species). Workers live in the last PC box, the FARM box, which the PC shows
// but won't let the player change.
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
#define WORKER_STEPS_PER_EXP 2 // Half the Day Care's rate

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
    const struct TownItem *items[TOWN_MAX_LEVEL]; // By level; up to TOWN_ITEM_KINDS, ending with ITEM_NONE
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

// Items each business stocks, by level: tier 1 at Lv. 1, and better tiers at
// Lv. 2 and 3.
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

static const struct TownItem sDeliveryServiceItems2[] =
{
    {ITEM_SUPER_POTION, 30},
    {ITEM_GREAT_BALL, 25},
    {ITEM_PARALYZE_HEAL, 15},
    {ITEM_SUPER_REPEL, 15},
    {ITEM_AWAKENING, 10},
    {ITEM_ESCAPE_ROPE, 5},
    {ITEM_NONE},
};

static const struct TownItem sDeliveryServiceItems3[] =
{
    {ITEM_HYPER_POTION, 30},
    {ITEM_ULTRA_BALL, 25},
    {ITEM_FULL_HEAL, 15},
    {ITEM_MAX_REPEL, 15},
    {ITEM_REVIVE, 15},
    {ITEM_NONE},
};

static const struct TownItem sGreenhouseItems2[] =
{
    {ITEM_SITRUS_BERRY, 25},
    {ITEM_ORAN_BERRY, 15},
    {ITEM_LEPPA_BERRY, 15},
    {ITEM_LUM_BERRY, 10},
    {ITEM_PECHA_BERRY, 10},
    {ITEM_CHERI_BERRY, 10},
    {ITEM_PERSIM_BERRY, 10},
    {ITEM_ASPEAR_BERRY, 5},
    {ITEM_NONE},
};

static const struct TownItem sGreenhouseItems3[] =
{
    {ITEM_SITRUS_BERRY, 25},
    {ITEM_LUM_BERRY, 20},
    {ITEM_LEPPA_BERRY, 15},
    {ITEM_LIECHI_BERRY, 8},
    {ITEM_GANLON_BERRY, 8},
    {ITEM_SALAC_BERRY, 8},
    {ITEM_PETAYA_BERRY, 8},
    {ITEM_APICOT_BERRY, 8},
    {ITEM_NONE},
};

static const struct TownItem sQuarryItems2[] =
{
    {ITEM_STARDUST, 20},
    {ITEM_HARD_STONE, 12},
    {ITEM_FIRE_STONE, 12},
    {ITEM_WATER_STONE, 12},
    {ITEM_THUNDER_STONE, 12},
    {ITEM_LEAF_STONE, 12},
    {ITEM_MOON_STONE, 10},
    {ITEM_SUN_STONE, 10},
    {ITEM_NONE},
};

static const struct TownItem sQuarryItems3[] =
{
    {ITEM_STAR_PIECE, 15},
    {ITEM_NUGGET, 13},
    {ITEM_FIRE_STONE, 12},
    {ITEM_WATER_STONE, 12},
    {ITEM_THUNDER_STONE, 12},
    {ITEM_LEAF_STONE, 12},
    {ITEM_MOON_STONE, 12},
    {ITEM_SUN_STONE, 12},
    {ITEM_NONE},
};

static const struct TownItem sFisheryItems2[] =
{
    {ITEM_PEARL, 20},
    {ITEM_BIG_PEARL, 15},
    {ITEM_HEART_SCALE, 15},
    {ITEM_NET_BALL, 15},
    {ITEM_DIVE_BALL, 15},
    {ITEM_SHOAL_SALT, 10},
    {ITEM_SHOAL_SHELL, 5},
    {ITEM_MYSTIC_WATER, 5},
    {ITEM_NONE},
};

static const struct TownItem sFisheryItems3[] =
{
    {ITEM_BIG_PEARL, 25},
    {ITEM_HEART_SCALE, 15},
    {ITEM_NET_BALL, 15},
    {ITEM_DIVE_BALL, 15},
    {ITEM_MYSTIC_WATER, 10},
    {ITEM_DRAGON_SCALE, 10},
    {ITEM_DEEP_SEA_TOOTH, 5},
    {ITEM_DEEP_SEA_SCALE, 5},
    {ITEM_NONE},
};

static const struct TownItem sGeneratorItems2[] =
{
    {ITEM_SUPER_POTION, 20},
    {ITEM_HYPER_POTION, 15},
    {ITEM_SUPER_REPEL, 15},
    {ITEM_PARALYZE_HEAL, 15},
    {ITEM_MAX_REPEL, 10},
    {ITEM_FULL_HEAL, 10},
    {ITEM_MAGNET, 10},
    {ITEM_THUNDER_STONE, 5},
    {ITEM_NONE},
};

static const struct TownItem sGeneratorItems3[] =
{
    {ITEM_HYPER_POTION, 25},
    {ITEM_MAX_REPEL, 20},
    {ITEM_FULL_HEAL, 15},
    {ITEM_MAX_POTION, 10},
    {ITEM_MAGNET, 10},
    {ITEM_THUNDER_STONE, 10},
    {ITEM_METAL_COAT, 5},
    {ITEM_PP_UP, 5},
    {ITEM_NONE},
};

static const struct TownItem sForgeItems2[] =
{
    {ITEM_GREAT_BALL, 30},
    {ITEM_ULTRA_BALL, 20},
    {ITEM_TIMER_BALL, 12},
    {ITEM_NEST_BALL, 12},
    {ITEM_REPEAT_BALL, 12},
    {ITEM_BURN_HEAL, 5},
    {ITEM_CHARCOAL, 5},
    {ITEM_BLACK_BELT, 4},
    {ITEM_NONE},
};

static const struct TownItem sForgeItems3[] =
{
    {ITEM_ULTRA_BALL, 40},
    {ITEM_TIMER_BALL, 12},
    {ITEM_NEST_BALL, 12},
    {ITEM_REPEAT_BALL, 12},
    {ITEM_LUXURY_BALL, 10},
    {ITEM_CHARCOAL, 5},
    {ITEM_BLACK_BELT, 5},
    {ITEM_PROTEIN, 4},
    {ITEM_NONE},
};

static const struct TownItem sSpiritSanctumItems2[] =
{
    {ITEM_REVIVE, 30},
    {ITEM_FULL_HEAL, 20},
    {ITEM_HYPER_POTION, 20},
    {ITEM_MAX_REVIVE, 8},
    {ITEM_TWISTED_SPOON, 8},
    {ITEM_CLEANSE_TAG, 7},
    {ITEM_SPELL_TAG, 7},
    {ITEM_NONE},
};

static const struct TownItem sSpiritSanctumItems3[] =
{
    {ITEM_MAX_REVIVE, 20},
    {ITEM_REVIVE, 20},
    {ITEM_FULL_RESTORE, 15},
    {ITEM_ELIXIR, 15},
    {ITEM_RARE_CANDY, 10},
    {ITEM_SPELL_TAG, 10},
    {ITEM_TWISTED_SPOON, 10},
    {ITEM_NONE},
};

static const struct TownItem sIceDomeItems2[] =
{
    {ITEM_MAX_POTION, 25},
    {ITEM_FULL_RESTORE, 20},
    {ITEM_MAX_REPEL, 15},
    {ITEM_ULTRA_BALL, 15},
    {ITEM_NEVER_MELT_ICE, 10},
    {ITEM_RARE_CANDY, 10},
    {ITEM_ICE_HEAL, 5},
    {ITEM_NONE},
};

static const struct TownItem sIceDomeItems3[] =
{
    {ITEM_FULL_RESTORE, 30},
    {ITEM_MAX_POTION, 15},
    {ITEM_MAX_REVIVE, 15},
    {ITEM_RARE_CANDY, 15},
    {ITEM_PP_UP, 10},
    {ITEM_NEVER_MELT_ICE, 10},
    {ITEM_PP_MAX, 5},
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
        .items = {sDeliveryServiceItems, sDeliveryServiceItems2, sDeliveryServiceItems3},
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
        .items = {sGreenhouseItems, sGreenhouseItems2, sGreenhouseItems3},
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
        .items = {sQuarryItems, sQuarryItems2, sQuarryItems3},
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
        .items = {sFisheryItems, sFisheryItems2, sFisheryItems3},
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
        .items = {sGeneratorItems, sGeneratorItems2, sGeneratorItems3},
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
        .items = {sForgeItems, sForgeItems2, sForgeItems3},
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
        .items = {sSpiritSanctumItems, sSpiritSanctumItems2, sSpiritSanctumItems3},
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
        .items = {sIceDomeItems, sIceDomeItems2, sIceDomeItems3},
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

// Upgrade prices: Lv. 2 costs 2x the building's price and Lv. 3 costs 6x. The
// free Delivery Service is priced as if it cost this much.
#define FREE_BUILDING_UPGRADE_BASE 2500

static const u8 sText_FarmBoxName[] = _("FARM");
static const u8 sText_Of[] = _(" of ");

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

// A business's level, 1 to TOWN_MAX_LEVEL. It has that many worker slots.
static u8 GetLevel(u8 building)
{
    u8 level = GetStock(building)->level;

    return level == 0 ? 1 : level;
}

// What it costs to build a business (level 1) or upgrade it to a level.
static u32 GetCost(u8 building, u8 level)
{
    u32 base = sBusinesses[building].cost;

    if (level <= 1)
        return base;
    if (base == 0)
        base = FREE_BUILDING_UPGRADE_BASE;
    return level == 2 ? base * 2 : base * 6;
}

// ---------------------------------------------------------------------------
// Workers live in the FARM box: building b's slot s is box position
// b * TOWN_MAX_WORKERS + s.
// ---------------------------------------------------------------------------

static struct BoxPokemon *GetWorker(u8 building, u8 slot)
{
    return GetBoxedMonPtr(TOWN_FARM_BOX, building * TOWN_MAX_WORKERS + slot);
}

static bool8 HasWorkerAt(u8 building, u8 slot)
{
    return GetBoxMonData(GetWorker(building, slot), MON_DATA_SPECIES) != SPECIES_NONE;
}

static u8 CountWorkers(u8 building)
{
    u8 slot, count = 0;

    for (slot = 0; slot < TOWN_MAX_WORKERS; slot++)
    {
        if (HasWorkerAt(building, slot))
            count++;
    }
    return count;
}

static s8 GetFirstWorkerSlot(u8 building)
{
    u8 slot;

    for (slot = 0; slot < TOWN_MAX_WORKERS; slot++)
    {
        if (HasWorkerAt(building, slot))
            return slot;
    }
    return -1;
}

static bool8 MoveToFreeBoxSlot(struct BoxPokemon *mon)
{
    u8 box, pos;

    for (box = 0; box < TOWN_FARM_BOX; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            struct BoxPokemon *dest = GetBoxedMonPtr(box, pos);
            if (GetBoxMonData(dest, MON_DATA_SPECIES) == SPECIES_NONE)
            {
                *dest = *mon;
                ZeroBoxMonData(mon);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Sets up the FARM box: the first time, moves anything the player kept in the
// last box to other boxes and names it. Also moves workers from saves made
// before the FARM box existed (one per building, kept in struct TownBuilding).
void Town_InitFarmBox(void)
{
    u8 building, pos;

    if (!FlagGet(FLAG_FARM_BOX_READY))
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            struct BoxPokemon *mon = GetBoxedMonPtr(TOWN_FARM_BOX, pos);
            if (GetBoxMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE)
                MoveToFreeBoxSlot(mon);
        }
        StringCopy(gPokemonStoragePtr->boxNames[TOWN_FARM_BOX], sText_FarmBoxName);
        FlagSet(FLAG_FARM_BOX_READY);
    }

    for (building = 0; building < TOWN_BUILDING_COUNT; building++)
    {
        struct TownBuilding *town = GetBuilding(building);
        if (!town->hasWorker)
            continue;
        if (!HasWorkerAt(building, 0))
            *GetWorker(building, 0) = town->worker;
        ZeroBoxMonData(&town->worker);
        town->hasWorker = FALSE;
    }
}

void ResetTownData(void)
{
    memset(&gPokemonStoragePtr->town, 0, sizeof(gPokemonStoragePtr->town));
    Town_InitFarmBox();
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
static u32 GetWorkerEfficiency(u8 building, struct BoxPokemon *worker)
{
    const struct Business *business = &sBusinesses[building];
    u16 species = GetBoxMonData(worker, MON_DATA_SPECIES);
    u32 typeFit, efficiency;

    if (species == SPECIES_NONE)
        return 0;
    if (IsSuitedWorker(building, species))
        typeFit = 200;
    else if (SpeciesHasType(species, business->bonusType))
        typeFit = 150;
    else
        typeFit = 50;

    efficiency = typeFit * GetLevelFromBoxMonExp(worker) / 10;
    if (species == business->specialists[0] || species == business->specialists[1])
        efficiency *= 2;
    if (efficiency < 5)
        efficiency = 5;
    return efficiency;
}

// All the business's workers together.
static u32 GetTotalEfficiency(u8 building)
{
    u8 slot;
    u32 total = 0;

    for (slot = 0; slot < TOWN_MAX_WORKERS; slot++)
        total += GetWorkerEfficiency(building, GetWorker(building, slot));
    return total;
}

static u32 CountStockedItems(struct TownStock *stock)
{
    u32 i, count = 0;

    for (i = 0; i < TOWN_ITEM_KINDS; i++)
        count += stock->counts[i];
    return count;
}

// Picks an item from the business's table for its level, weighted.
static u16 RollItem(u8 building)
{
    const struct TownItem *items = sBusinesses[building].items[GetLevel(building) - 1];
    u32 i, total = 0, roll;

    for (i = 0; i < TOWN_ITEM_KINDS && items[i].item != ITEM_NONE; i++)
        total += items[i].weight;
    roll = Random() % total;
    for (i = 0; roll >= items[i].weight; i++)
        roll -= items[i].weight;
    return items[i].item;
}

// Adds one item to the stock. FALSE if there's no room for another kind.
static bool8 StockItem(struct TownStock *stock, u16 item)
{
    u32 i;

    for (i = 0; i < TOWN_ITEM_KINDS; i++)
    {
        if (stock->counts[i] != 0 && stock->items[i] == item)
        {
            stock->counts[i]++;
            return TRUE;
        }
    }
    for (i = 0; i < TOWN_ITEM_KINDS; i++)
    {
        if (stock->counts[i] == 0)
        {
            stock->items[i] = item;
            stock->counts[i] = 1;
            return TRUE;
        }
    }
    return FALSE;
}

static void SettleItems(u8 building, u32 steps, u32 efficiency)
{
    struct TownStock *stock = GetStock(building);
    u32 threshold = sBusinesses[building].itemSteps * 100;
    u32 stocked = CountStockedItems(stock);

    stock->itemProgress += steps * efficiency;
    while (stock->itemProgress >= threshold && stocked < ITEM_STOCKROOM_SIZE)
    {
        stock->itemProgress -= threshold;
        // After an upgrade, old and new kinds can fill every stock slot; a roll
        // with nowhere to go is lost.
        if (StockItem(stock, RollItem(building)))
            stocked++;
    }
    if (stocked >= ITEM_STOCKROOM_SIZE)
        stock->itemProgress = 0;
}

// Working earns each worker EXP at half the Day Care's rate (and, like the Day
// Care, no new moves). Odd leftover steps are dropped at each settle.
static void GiveWorkerExp(struct BoxPokemon *worker, u32 steps)
{
    u16 species = GetBoxMonData(worker, MON_DATA_SPECIES);
    u32 exp, maxExp;

    if (species == SPECIES_NONE)
        return;
    exp = GetBoxMonData(worker, MON_DATA_EXP) + steps / WORKER_STEPS_PER_EXP;
    maxExp = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
    if (exp > maxExp)
        exp = maxExp;
    SetBoxMonData(worker, MON_DATA_EXP, &exp);
}

// Credit the business for the steps walked since it was last settled: sales,
// items, and EXP for the workers.
static void SettleBuilding(u8 building)
{
    struct TownBuilding *town = GetBuilding(building);
    u32 now = GetGameStat(GAME_STAT_STEPS);
    u32 steps = now - town->lastSteps;
    u32 threshold = STEPS_PER_SALE * 100;
    u32 sales, efficiency;
    u8 slot;

    town->lastSteps = now;
    efficiency = GetTotalEfficiency(building);
    if (efficiency == 0)
        return;

    // Cap to avoid overflow; the stockrooms fill long before this many steps.
    if (steps > 0xFFFF)
        steps = 0xFFFF;
    for (slot = 0; slot < TOWN_MAX_WORKERS; slot++)
        GiveWorkerExp(GetWorker(building, slot), steps);
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

// Writes an efficiency such as "1.5" to dest.
static void ConvertEfficiencyToString(u8 *dest, u32 efficiency)
{
    dest = ConvertIntToDecimalStringN(dest, efficiency / 100, STR_CONV_MODE_LEFT_ALIGN, 3);
    *dest++ = CHAR_PERIOD;
    ConvertIntToDecimalStringN(dest, (efficiency % 100) / 10, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// Draw every rebuilt business over its ruined plot. Called from Littleroot's
// ON_LOAD map script, and right after a business is rebuilt.
void Town_DrawBuildings(void)
{
    u8 building;
    s32 x, y;

    Town_InitFarmBox();
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

// Show the terminals of rebuilt businesses. Called from Littleroot's
// ON_TRANSITION map script, before objects are spawned.
void Town_UpdateTerminals(void)
{
    u8 building;

    Town_InitFarmBox();
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

// Buffers business VAR_0x8006 at level VAR_0x8005 (0 or 1 to build it):
// STR_VAR_1 name, STR_VAR_2 cost, STR_VAR_3 worker types.
void Town_BufferBuildingInfo(void)
{
    const struct Business *business;

    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    business = &sBusinesses[gSpecialVar_0x8006];
    StringCopy(gStringVar1, business->name);
    ConvertIntToDecimalStringN(gStringVar2, GetCost(gSpecialVar_0x8006, gSpecialVar_0x8005), STR_CONV_MODE_LEFT_ALIGN, 7);
    StringCopy(gStringVar3, business->workerTypes);
}

// VAR_RESULT: whether the player can pay for business VAR_0x8006 at level VAR_0x8005.
void Town_CanAffordBuilding(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006)
                      && IsEnoughMoney(&gSaveBlock1Ptr->money, GetCost(gSpecialVar_0x8006, gSpecialVar_0x8005));
}

// VAR_RESULT: whether the party has a Pokémon suited to work at business VAR_0x8006.
void Town_CheckPartyForWorker(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && PartyHasSuitedWorker(gSpecialVar_0x8006);
}

// Pays for business VAR_0x8006 at level VAR_0x8005: rebuilds it (level 0 or 1)
// or upgrades it. VAR_0x8007: its terminal's local ID.
void Town_BuyBuilding(void)
{
    const struct Business *business;

    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    business = &sBusinesses[gSpecialVar_0x8006];
    RemoveMoney(&gSaveBlock1Ptr->money, GetCost(gSpecialVar_0x8006, gSpecialVar_0x8005));
    if (gSpecialVar_0x8005 >= 2)
    {
        // Settle at the old level first, so earlier steps use the old items.
        SettleBuilding(gSpecialVar_0x8006);
        GetStock(gSpecialVar_0x8006)->level = gSpecialVar_0x8005;
    }
    else
    {
        FlagSet(business->builtFlag);
        FlagClear(business->terminalHideFlag);
    }
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

// VAR_RESULT: whether business VAR_0x8006 has any workers.
void Town_HasWorker(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && CountWorkers(gSpecialVar_0x8006) != 0;
}

// VAR_RESULT: whether business VAR_0x8006 has ever had a worker.
void Town_WasEverStaffed(void)
{
    gSpecialVar_Result = IsBusiness(gSpecialVar_0x8006) && GetBuilding(gSpecialVar_0x8006)->everStaffed;
}

// Buffers the worker in slot VAR_0x800A of business VAR_0x8006: STR_VAR_1
// name, STR_VAR_2 level, STR_VAR_3 efficiency such as "1.5".
void Town_BufferWorker(void)
{
    struct BoxPokemon *worker;

    if (!IsBusiness(gSpecialVar_0x8006) || gSpecialVar_0x800A >= TOWN_MAX_WORKERS)
        return;
    worker = GetWorker(gSpecialVar_0x8006, gSpecialVar_0x800A);
    if (GetBoxMonData(worker, MON_DATA_SPECIES) == SPECIES_NONE)
        return;
    GetBoxMonData(worker, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, GetLevelFromBoxMonExp(worker), STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertEfficiencyToString(gStringVar3, GetWorkerEfficiency(gSpecialVar_0x8006, worker));
}

// For the terminal's first line. With one slot (Lv. 1), buffers the worker like
// Town_BufferWorker and VAR_RESULT is FALSE. With more, buffers STR_VAR_1 "2 of
// 3" workers, STR_VAR_2 the building's level and STR_VAR_3 the total speed,
// and VAR_RESULT is TRUE.
void Town_BufferStatus(void)
{
    u8 *end;

    gSpecialVar_Result = FALSE;
    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    if (GetLevel(gSpecialVar_0x8006) == 1 && GetFirstWorkerSlot(gSpecialVar_0x8006) >= 0)
    {
        gSpecialVar_0x800A = GetFirstWorkerSlot(gSpecialVar_0x8006);
        Town_BufferWorker();
        return;
    }
    end = ConvertIntToDecimalStringN(gStringVar1, CountWorkers(gSpecialVar_0x8006), STR_CONV_MODE_LEFT_ALIGN, 1);
    end = StringCopy(end, sText_Of);
    ConvertIntToDecimalStringN(end, GetLevel(gSpecialVar_0x8006), STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar2, GetLevel(gSpecialVar_0x8006), STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertEfficiencyToString(gStringVar3, GetTotalEfficiency(gSpecialVar_0x8006));
    gSpecialVar_Result = TRUE;
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
// items. VAR_0x8008: the first worker's species, for showmonpic. VAR_RESULT:
// whether there's anything to collect.
void Town_BufferStock(void)
{
    struct TownBuilding *town;
    s8 slot;
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
    slot = GetFirstWorkerSlot(gSpecialVar_0x8006);
    if (slot >= 0)
        gSpecialVar_0x8008 = GetBoxMonData(GetWorker(gSpecialVar_0x8006, slot), MON_DATA_SPECIES);
    gSpecialVar_Result = (town->pendingMoney != 0 || items != 0);
}

// Moves the next kind of stocked item from business VAR_0x8006 into the bag,
// starting from stock slot VAR_0x8009 (set it to 0 first). Buffers STR_VAR_1
// the item name and STR_VAR_2 how many. VAR_RESULT: a TOWN_ITEM_* result;
// items that don't fit in the bag stay in stock.
void Town_CollectNextItem(void)
{
    struct TownStock *stock;
    u16 i;

    gSpecialVar_Result = TOWN_ITEMS_DONE;
    if (!IsBusiness(gSpecialVar_0x8006))
        return;
    stock = GetStock(gSpecialVar_0x8006);
    for (i = gSpecialVar_0x8009; i < TOWN_ITEM_KINDS; i++)
    {
        u8 count = stock->counts[i];
        if (count == 0)
            continue;

        gSpecialVar_0x8009 = i + 1;
        CopyItemNameHandlePlural(stock->items[i], gStringVar1, count);
        ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (AddBagItem(stock->items[i], count))
        {
            stock->counts[i] = 0;
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

// Makes party slot VAR_0x8004 the worker in slot VAR_0x800A of business
// VAR_0x8006. A worker already there swaps into that party slot.
// VAR_RESULT: a TOWN_ASSIGN_* result.
void Town_AssignWorker(void)
{
    struct BoxPokemon *worker;
    struct Pokemon *mon;

    if (!IsBusiness(gSpecialVar_0x8006) || gSpecialVar_0x8004 >= PARTY_SIZE
     || gSpecialVar_0x800A >= GetLevel(gSpecialVar_0x8006))
    {
        gSpecialVar_Result = TOWN_ASSIGN_CANCELLED;
        return;
    }
    worker = GetWorker(gSpecialVar_0x8006, gSpecialVar_0x800A);
    mon = &gPlayerParty[gSpecialVar_0x8004];
    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = TOWN_ASSIGN_EGG;
        return;
    }
    if (GetBoxMonData(worker, MON_DATA_SPECIES) == SPECIES_NONE && CountOtherUsablePartyMons(gSpecialVar_0x8004) == 0)
    {
        gSpecialVar_Result = TOWN_ASSIGN_LAST_MON;
        return;
    }

    // Credit the outgoing workers for the steps walked so far.
    SettleBuilding(gSpecialVar_0x8006);
    if (GetBoxMonData(worker, MON_DATA_SPECIES) != SPECIES_NONE)
    {
        struct BoxPokemon previous = *worker;
        *worker = mon->box;
        BoxMonToMon(&previous, mon);
    }
    else
    {
        *worker = mon->box;
        ZeroMonData(mon);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    GetBuilding(gSpecialVar_0x8006)->everStaffed = TRUE;
    gSpecialVar_Result = TOWN_ASSIGN_OK;
}

// Returns the worker in slot VAR_0x800A of business VAR_0x8006 to the party.
// VAR_RESULT: FALSE if the party is full.
void Town_TakeBackWorker(void)
{
    struct BoxPokemon *worker;

    gSpecialVar_Result = FALSE;
    if (!IsBusiness(gSpecialVar_0x8006) || gSpecialVar_0x800A >= TOWN_MAX_WORKERS)
        return;
    worker = GetWorker(gSpecialVar_0x8006, gSpecialVar_0x800A);
    if (GetBoxMonData(worker, MON_DATA_SPECIES) == SPECIES_NONE || CalculatePlayerPartyCount() >= PARTY_SIZE)
        return;

    SettleBuilding(gSpecialVar_0x8006);
    BoxMonToMon(worker, &gPlayerParty[gPlayerPartyCount]);
    ZeroBoxMonData(worker);
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
    *GetWorker(building, 0) = mon.box;
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
// List menus: the Builder's offers and a business's worker slots.
// ---------------------------------------------------------------------------

#define LIST_MENU_EXIT 0xFF
#define BUILDER_MENU_WIDTH 24
#define WORKER_MENU_WIDTH  20
#define LIST_MENU_ROWS     4
#define LIST_LABEL_LENGTH  24

// A Builder offer's list ID: building + TOWN_BUILDING_COUNT * (level - 1).
#define OFFER_ID(building, level) ((building) + TOWN_BUILDING_COUNT * ((level) - 1))

#define tListTaskId data[0]
#define tWindowId   data[1]
#define tMenuKind   data[2]

enum {
    MENU_KIND_BUILDER,
    MENU_KIND_WORKERS,
};

struct TownListMenu
{
    struct ListMenuItem items[TOWN_BUILDING_COUNT * TOWN_MAX_LEVEL + 1];
    u8 labels[TOWN_BUILDING_COUNT * TOWN_MAX_LEVEL + 1][LIST_LABEL_LENGTH];
    u8 width;
};

static EWRAM_DATA struct TownListMenu *sTownListMenu = NULL;

static const u8 sListMenuTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sText_LevelPrefix[] = _(" Lv.");
static const u8 sText_EmptySlot[] = _("- empty -");
static const u8 sText_SpeedSuffix[] = _("x");

// The level a building could be bought or upgraded to next, or 0 if none.
static u8 GetBuilderOfferLevel(u8 building)
{
    const struct Business *business = &sBusinesses[building];

    if (!IsBusiness(building))
        return 0;
    if (IsBuilt(building))
        return GetLevel(building) < TOWN_MAX_LEVEL ? GetLevel(building) + 1 : 0;
    if (business->cost == 0)
        return 0; // The free Delivery Service is built with the Builder's intro
    if (business->unlockFlag != 0 && !FlagGet(business->unlockFlag))
        return 0;
    if (business->hiddenWithoutWorker && !PartyHasSuitedWorker(building))
        return 0;
    return 1;
}

// VAR_RESULT: whether the Builder can build or upgrade anything right now.
void Town_HasBuilderOffers(void)
{
    u8 building;

    gSpecialVar_Result = FALSE;
    for (building = 0; building < TOWN_BUILDING_COUNT; building++)
    {
        if (GetBuilderOfferLevel(building) != 0)
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

// Prints a Builder offer's cost, or a worker's speed, at the right edge.
static void TownListMenu_PrintRight(u8 windowId, u32 itemId, u8 y)
{
    u8 *label;

    if (itemId == LIST_MENU_EXIT)
        return;
    label = gStringVar4;
    if (sTownListMenu->width == BUILDER_MENU_WIDTH)
    {
        ConvertIntToDecimalStringN(gStringVar1, GetCost(itemId % TOWN_BUILDING_COUNT, itemId / TOWN_BUILDING_COUNT + 1),
                                   STR_CONV_MODE_LEFT_ALIGN, 7);
        StringExpandPlaceholders(label, gText_PokedollarVar1);
    }
    else
    {
        struct BoxPokemon *worker = GetWorker(gSpecialVar_0x8006, itemId);
        if (GetBoxMonData(worker, MON_DATA_SPECIES) == SPECIES_NONE)
            return;
        ConvertEfficiencyToString(label, GetWorkerEfficiency(gSpecialVar_0x8006, worker));
        StringAppend(label, sText_SpeedSuffix);
    }
    AddTextPrinterParameterized4(windowId, FONT_NORMAL,
                                 GetStringRightAlignXOffset(FONT_NORMAL, label, sTownListMenu->width * 8 - 2),
                                 y, 0, 0, sListMenuTextColors, TEXT_SKIP_DRAW, label);
}

static void FreeTownListMenu(void)
{
    Free(sTownListMenu);
    sTownListMenu = NULL;
}

static void Task_TownListMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 input = ListMenu_ProcessInput(task->tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;

    PlaySE(SE_SELECT);
    if (task->tMenuKind == MENU_KIND_BUILDER)
    {
        if (input == LIST_CANCEL || input == LIST_MENU_EXIT)
        {
            gSpecialVar_Result = TOWN_MENU_CANCELLED;
        }
        else
        {
            gSpecialVar_Result = input % TOWN_BUILDING_COUNT;
            gSpecialVar_0x8005 = input / TOWN_BUILDING_COUNT + 1;
        }
    }
    else
    {
        if (input == LIST_CANCEL || input == LIST_MENU_EXIT)
        {
            gSpecialVar_Result = FALSE;
        }
        else
        {
            gSpecialVar_0x800A = input;
            gSpecialVar_Result = TRUE;
        }
    }

    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    FreeTownListMenu();
    ClearStdWindowAndFrameToTransparent(task->tWindowId, TRUE);
    RemoveWindow(task->tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void ShowTownListMenu(u8 kind, u8 count)
{
    struct ListMenuTemplate list;
    struct WindowTemplate window;
    u8 taskId, windowId, rows;

    sTownListMenu->items[count].name = gText_Exit;
    sTownListMenu->items[count].id = LIST_MENU_EXIT;
    count++;

    rows = min(count, LIST_MENU_ROWS);
    window = CreateWindowTemplate(0, 1, 1, sTownListMenu->width, rows * 2, 15, 0x64);
    windowId = AddWindow(&window);
    SetStandardWindowBorderStyle(windowId, FALSE);

    list.items = sTownListMenu->items;
    list.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    list.itemPrintFunc = TownListMenu_PrintRight;
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

    taskId = CreateTask(Task_TownListMenu, 80);
    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tMenuKind = kind;
    gTasks[taskId].tListTaskId = ListMenuInit(&list, 0, 0);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static u32 GetOfferSortKey(u32 id)
{
    u8 level = id / TOWN_BUILDING_COUNT + 1;
    u32 cost = GetCost(id % TOWN_BUILDING_COUNT, level);

    // Costs stay under 2^24 (MAX_MONEY), so the top bit marks upgrades.
    return level > 1 ? cost | (1u << 31) : cost;
}

// Lists what the Builder can build or upgrade now, with costs: new buildings
// first, then upgrades, each cheapest first.
// Used with waitstate; check Town_HasBuilderOffers first. VAR_RESULT: the
// chosen business (VAR_0x8005: the level, 1 to build it), TOWN_MENU_CANCELLED,
// or TOWN_MENU_EMPTY if there's nothing to offer.
void Town_ShowBuilderMenu(void)
{
    u8 building, level, count = 0, i, j;

    sTownListMenu = AllocZeroed(sizeof(*sTownListMenu));
    sTownListMenu->width = BUILDER_MENU_WIDTH;
    for (building = 0; building < TOWN_BUILDING_COUNT; building++)
    {
        u8 *label = sTownListMenu->labels[count];
        level = GetBuilderOfferLevel(building);
        if (level == 0)
            continue;
        label = StringCopy(label, sBusinesses[building].name);
        if (level > 1)
        {
            label = StringCopy(label, sText_LevelPrefix);
            ConvertIntToDecimalStringN(label, level, STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        sTownListMenu->items[count].name = sTownListMenu->labels[count];
        sTownListMenu->items[count].id = OFFER_ID(building, level);
        count++;
    }
    if (count == 0)
    {
        FreeTownListMenu();
        gSpecialVar_Result = TOWN_MENU_EMPTY;
        // Resume on the next frame, once the script is waiting.
        CreateTask(Task_ResumeScript, 80);
        return;
    }

    // New buildings first, then upgrades, each cheapest first. The labels stay
    // put; only the items move.
    for (i = 1; i < count; i++)
    {
        for (j = i; j > 0; j--)
        {
            struct ListMenuItem *a = &sTownListMenu->items[j - 1], *b = &sTownListMenu->items[j];
            struct ListMenuItem swap;
            if (GetOfferSortKey(a->id) <= GetOfferSortKey(b->id))
                break;
            swap = *a;
            *a = *b;
            *b = swap;
        }
    }
    ShowTownListMenu(MENU_KIND_BUILDER, count);
}

// Lets the player pick a worker slot of business VAR_0x8006. VAR_0x8005 is
// TOWN_SLOTS_TO_ASSIGN (every slot the building has) or TOWN_SLOTS_TO_TAKE_BACK
// (occupied slots). With only one choice, it's picked without a menu. Used
// with waitstate. VAR_RESULT: whether a slot was picked; VAR_0x800A: the slot.
void Town_ChooseWorkerSlot(void)
{
    u8 slot, count = 0;

    gSpecialVar_Result = FALSE;
    sTownListMenu = AllocZeroed(sizeof(*sTownListMenu));
    sTownListMenu->width = WORKER_MENU_WIDTH;
    if (IsBusiness(gSpecialVar_0x8006))
    {
        for (slot = 0; slot < GetLevel(gSpecialVar_0x8006); slot++)
        {
            u8 *label = sTownListMenu->labels[count];
            struct BoxPokemon *worker = GetWorker(gSpecialVar_0x8006, slot);
            if (GetBoxMonData(worker, MON_DATA_SPECIES) == SPECIES_NONE)
            {
                if (gSpecialVar_0x8005 == TOWN_SLOTS_TO_TAKE_BACK)
                    continue;
                StringCopy(label, sText_EmptySlot);
            }
            else
            {
                GetBoxMonData(worker, MON_DATA_NICKNAME, label);
                StringGet_Nickname(label);
                label = StringAppend(label, sText_LevelPrefix);
                ConvertIntToDecimalStringN(label, GetLevelFromBoxMonExp(worker), STR_CONV_MODE_LEFT_ALIGN, 3);
            }
            sTownListMenu->items[count].name = sTownListMenu->labels[count];
            sTownListMenu->items[count].id = slot;
            count++;
        }
    }
    if (count <= 1)
    {
        if (count == 1)
        {
            gSpecialVar_0x800A = sTownListMenu->items[0].id;
            gSpecialVar_Result = TRUE;
        }
        FreeTownListMenu();
        CreateTask(Task_ResumeScript, 80);
        return;
    }
    ShowTownListMenu(MENU_KIND_WORKERS, count);
}

#undef tListTaskId
#undef tWindowId
#undef tMenuKind
