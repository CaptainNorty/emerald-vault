#ifndef GUARD_CONSTANTS_TEST_START_H
#define GUARD_CONSTANTS_TEST_START_H

// Emerald Vault: layout of gTestStartConfig, the u16 array the emulator test
// harness (tools/emulator/test_rom.py) fills in while the test-only ROM waits
// at boot. Keep the two in sync.
#define TEST_CONFIG_READY          0
#define TEST_CONFIG_HAS_LOCATION   1
#define TEST_CONFIG_MAP_GROUP      2
#define TEST_CONFIG_MAP_NUM        3
#define TEST_CONFIG_X              4
#define TEST_CONFIG_Y              5
#define TEST_CONFIG_MONEY_LO       6
#define TEST_CONFIG_MONEY_HI       7
#define TEST_CONFIG_SET_MONEY      8
#define TEST_CONFIG_FLAG_COUNT     9
#define TEST_CONFIG_VAR_COUNT      10
#define TEST_CONFIG_PARTY_COUNT    11
#define TEST_CONFIG_ITEM_COUNT     12
#define TEST_CONFIG_WORKER_COUNT   13

#define TEST_CONFIG_FLAGS          32  // Up to 64 flag IDs; TEST_CONFIG_CLEAR_FLAG set means clear it
#define TEST_CONFIG_VARS           96  // Up to 16 (var, value) pairs
#define TEST_CONFIG_PARTY          128 // Up to 6 (species, level) pairs
#define TEST_CONFIG_ITEMS          144 // Up to 16 (item, count) pairs
#define TEST_CONFIG_WORKERS        176 // Up to 9 (building, species, level) triples
#define TEST_CONFIG_SIZE           208

#define TEST_CONFIG_MAX_FLAGS      64
#define TEST_CONFIG_MAX_VARS       16
#define TEST_CONFIG_MAX_ITEMS      16
#define TEST_CONFIG_CLEAR_FLAG     0x8000

#endif // GUARD_CONSTANTS_TEST_START_H
