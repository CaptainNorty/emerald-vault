// Emerald Vault: test-only ROM (make TEST_START=1). Boots straight into a new
// game, skipping the intro, title screen, Birch's speech and the truck. The
// emulator test harness can pick a starting setup (a TEST_PRESET_*) by writing
// gTestStartPreset and gTestStartHarnessReady while the game waits at boot.
#include "global.h"
#include "test_start.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "new_game.h"
#include "overworld.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "constants/vault_town.h"

#if TEST_START

// Frames to wait at boot for the harness before starting with the default setup.
#define HARNESS_WAIT_FRAMES 30

EWRAM_DATA u8 gTestStartPreset = TEST_PRESET_NEW_GAME;
EWRAM_DATA u8 gTestStartHarnessReady = FALSE;
static EWRAM_DATA u8 sHarnessWaitFrames = 0;

static const u8 sTestStartPlayerName[] = _("TEST");

static void StartTestGame(void)
{
    ResetTasks();
    if (gTestStartPreset > TEST_PRESET_OLDALE_NO_BALLS)
        gTestStartPreset = TEST_PRESET_NEW_GAME;
    gSaveBlock2Ptr->playerGender = MALE;
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_FAST; // Predictable dialogue timing
    StringCopy(gSaveBlock2Ptr->playerName, sTestStartPlayerName);
    SetMainCallback2(CB2_NewGame);
}

static void CB2_TestStartWaitForHarness(void)
{
    if (gTestStartHarnessReady || ++sHarnessWaitFrames > HARNESS_WAIT_FRAMES)
        StartTestGame();
}

// Called after the copyright screen, once the save has been loaded.
void TestStart_WaitForHarness(void)
{
    gTestStartPreset = TEST_PRESET_NEW_GAME;
    gTestStartHarnessReady = FALSE;
    sHarnessWaitFrames = 0;
    SetMainCallback2(CB2_TestStartWaitForHarness);
}

void TestStart_SetWarp(void)
{
    if (gTestStartPreset == TEST_PRESET_OLDALE_NO_BALLS)
        SetWarpDestination(MAP_GROUP(MAP_OLDALE_TOWN), MAP_NUM(MAP_OLDALE_TOWN), WARP_ID_NONE, 5, 10);
    else // Outside the player's house
        SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), WARP_ID_NONE, 5, 9);
}

// Called at the end of NewGameInitData.
void TestStart_ApplyPreset(void)
{
    // The moving trucks are only parked outside during the intro.
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);
    if (gTestStartPreset == TEST_PRESET_NEW_GAME)
        return;

    // As if the player has just received the Pokédex and Running Shoes.
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_ADVENTURE_STARTED);
    FlagSet(FLAG_RESCUED_BIRCH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_VISITED_LITTLEROOT_TOWN);
    FlagSet(FLAG_VISITED_OLDALE_TOWN);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);

    ScriptGiveMon(SPECIES_MUDKIP, 5, ITEM_NONE, 0, 0, 0);
    if (gTestStartPreset == TEST_PRESET_HAS_FLYING_TYPE)
        ScriptGiveMon(SPECIES_WINGULL, 5, ITEM_NONE, 0, 0, 0);
    if (gTestStartPreset != TEST_PRESET_OLDALE_NO_BALLS)
        AddBagItem(ITEM_POKE_BALL, 5);
    // Keep wild Pokémon away so test walks through grass are deterministic.
    VarSet(VAR_REPEL_STEP_COUNT, 1000);
}

#endif // TEST_START
