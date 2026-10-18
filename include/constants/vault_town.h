#ifndef GUARD_CONSTANTS_VAULT_TOWN_H
#define GUARD_CONSTANTS_VAULT_TOWN_H

// Emerald Vault: the businesses on Littleroot Town's farm, by plot.
#define TOWN_DELIVERY_SERVICE    0
#define TOWN_FORGE               1
#define TOWN_GREENHOUSE          2
#define TOWN_ICE_DOME            3
#define TOWN_WEATHER_STABILIZER  4
#define TOWN_GENERATOR           5
#define TOWN_FISHERY             6
#define TOWN_SPIRIT_SANCTUM      7
#define TOWN_QUARRY              8
#define TOWN_BUILDING_COUNT      9

// Town specials take the building in VAR_0x8006, a party slot in VAR_0x8004
// (as set by ChoosePartyMon) and a Pokémon type in VAR_0x8005.

// VAR_RESULT from Town_AssignWorker
#define TOWN_ASSIGN_OK          0
#define TOWN_ASSIGN_CANCELLED   1
#define TOWN_ASSIGN_EGG         2
#define TOWN_ASSIGN_LAST_MON    3

// Most kinds of item one business can stock
#define TOWN_ITEM_KINDS 8

// VAR_RESULT from Town_CollectNextItem
#define TOWN_ITEMS_DONE         0
#define TOWN_ITEM_COLLECTED     1
#define TOWN_ITEM_NO_ROOM       2

// VAR_RESULT from Town_ShowBuilderMenu, besides a building
#define TOWN_MENU_CANCELLED     0xFF
#define TOWN_MENU_EMPTY         0xFE

#endif // GUARD_CONSTANTS_VAULT_TOWN_H
