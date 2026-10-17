// Emerald Vault: test-only ROM (make TEST_START=1). Boots straight into a new
// game, skipping the intro, title screen, Birch's speech and the truck. While
// the game waits at boot, the emulator test harness can fill in
// gTestStartConfig (see include/constants/test_start.h) to choose the starting
// location, flags, variables, money, party, items and business workers.
#include "global.h"
#include "test_start.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "new_game.h"
#include "overworld.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "task.h"
#include "vault_town.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/test_start.h"

#if TEST_START

// Frames to wait at boot for the harness before starting a plain new game.
#define HARNESS_WAIT_FRAMES 30

EWRAM_DATA u16 gTestStartConfig[TEST_CONFIG_SIZE] = {0};
static EWRAM_DATA u8 sHarnessWaitFrames = 0;

static const u8 sTestStartPlayerName[] = _("TEST");

static void StartTestGame(void)
{
    ResetTasks();
    gSaveBlock2Ptr->playerGender = MALE;
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_FAST; // Predictable dialogue timing
    StringCopy(gSaveBlock2Ptr->playerName, sTestStartPlayerName);
    SetMainCallback2(CB2_NewGame);
}

static void CB2_TestStartWaitForHarness(void)
{
    if (gTestStartConfig[TEST_CONFIG_READY] || ++sHarnessWaitFrames > HARNESS_WAIT_FRAMES)
        StartTestGame();
}

// Called after the copyright screen, once the save has been loaded.
void TestStart_WaitForHarness(void)
{
    memset(gTestStartConfig, 0, sizeof(gTestStartConfig));
    sHarnessWaitFrames = 0;
    SetMainCallback2(CB2_TestStartWaitForHarness);
}

void TestStart_SetWarp(void)
{
    if (gTestStartConfig[TEST_CONFIG_HAS_LOCATION])
        SetWarpDestination(gTestStartConfig[TEST_CONFIG_MAP_GROUP], gTestStartConfig[TEST_CONFIG_MAP_NUM], WARP_ID_NONE,
                           gTestStartConfig[TEST_CONFIG_X], gTestStartConfig[TEST_CONFIG_Y]);
    else // Outside the player's house
        SetWarpDestination(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), WARP_ID_NONE, 5, 9);
}

// Called at the end of NewGameInitData.
void TestStart_ApplyPreset(void)
{
    const u16 *config = gTestStartConfig;
    u16 i;

    // The moving trucks are only parked outside during the intro.
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);

    for (i = 0; i < config[TEST_CONFIG_FLAG_COUNT] && i < TEST_CONFIG_MAX_FLAGS; i++)
    {
        u16 flag = config[TEST_CONFIG_FLAGS + i];
        if (flag & TEST_CONFIG_CLEAR_FLAG)
            FlagClear(flag & ~TEST_CONFIG_CLEAR_FLAG);
        else
            FlagSet(flag);
    }
    for (i = 0; i < config[TEST_CONFIG_VAR_COUNT] && i < TEST_CONFIG_MAX_VARS; i++)
        VarSet(config[TEST_CONFIG_VARS + i * 2], config[TEST_CONFIG_VARS + i * 2 + 1]);
    if (config[TEST_CONFIG_SET_MONEY])
        SetMoney(&gSaveBlock1Ptr->money, config[TEST_CONFIG_MONEY_LO] | (config[TEST_CONFIG_MONEY_HI] << 16));
    for (i = 0; i < config[TEST_CONFIG_PARTY_COUNT] && i < PARTY_SIZE; i++)
        ScriptGiveMon(config[TEST_CONFIG_PARTY + i * 2], config[TEST_CONFIG_PARTY + i * 2 + 1], ITEM_NONE, 0, 0, 0);
    for (i = 0; i < config[TEST_CONFIG_ITEM_COUNT] && i < TEST_CONFIG_MAX_ITEMS; i++)
        AddBagItem(config[TEST_CONFIG_ITEMS + i * 2], config[TEST_CONFIG_ITEMS + i * 2 + 1]);
    for (i = 0; i < config[TEST_CONFIG_WORKER_COUNT] && i < TOWN_BUILDING_COUNT; i++)
        Town_SetWorkerForTest(config[TEST_CONFIG_WORKERS + i * 3], config[TEST_CONFIG_WORKERS + i * 3 + 1],
                              config[TEST_CONFIG_WORKERS + i * 3 + 2]);
}

#endif // TEST_START
