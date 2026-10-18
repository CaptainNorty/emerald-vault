LittlerootTown_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, LittlerootTown_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, LittlerootTown_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, LittlerootTown_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, LittlerootTown_OnWarp
	.byte 0

	@ State descriptions for VAR_LITTLEROOT_TOWN_STATE
	@ 1: Met Rival
	@ 2: Saving Birch
	@ 3: Received Pokedex
	@ 4: Received Running Shoes

	@ State descriptions for VAR_LITTLEROOT_INTRO_STATE
	@ 1: In truck male
	@ 2: In truck female
	@ 3: Entered house
	@ 4: Told to go set clock
	@ 5: Entered 2F to set clock
	@ 6: Set clock
	@ 7: Told to go meet rival

	@ State descriptions for VAR_LITTLEROOT_HOUSES_STATE_MAY
	@                    and VAR_LITTLEROOT_HOUSES_STATE_BRENDAN
	@ 1: On Truck (and is corresponding gender)
	@ 2: Met Rival's Mom (and is corresponding gender)
	@ 3: Entered Hall of Fame
	@ 4: Completed Lati TV / SS Ticket event
	@ 5: Exited house after above event

	@ State descriptions for VAR_LITTLEROOT_RIVAL_STATE
	@ 1: Never occurs
	@ 2: Entered Rival's bedroom
	@ 3: Met Rival
	@ 4: Received Pokedex


@ Emerald Vault: draw the rebuilt businesses over their ruined plots.
LittlerootTown_OnLoad:
	special Town_DrawBuildings
	end

LittlerootTown_OnTransition:
	setflag FLAG_VISITED_LITTLEROOT_TOWN
	call_if_set FLAG_DELIVERY_SERVICE_BUILT, LittlerootTown_EventScript_SetBuilderInYard
	special Town_UpdateTerminals
	call Common_EventScript_SetupRivalGfxId
	call_if_eq VAR_LITTLEROOT_INTRO_STATE, 2, LittlerootTown_EventScript_MoveMomToMaysDoor
	call_if_unset FLAG_RESCUED_BIRCH, LittlerootTown_EventScript_SetTwinPos
	call_if_eq VAR_LITTLEROOT_TOWN_STATE, 3, LittlerootTown_EventScript_SetMomStandingInFrontOfDoorPos
	call_if_eq VAR_LITTLEROOT_HOUSES_STATE_MAY, 4, LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent
	call_if_eq VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4, LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent
	call_if_eq VAR_OLDALE_RIVAL_STATE, 1, LittlerootTown_EventScript_MoveRivalFromOldale
	call_if_eq VAR_LITTLEROOT_RIVAL_STATE, 3, LittlerootTown_EventScript_SetRivalLeftForRoute103
	call_if_eq VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_EventScript_HideMapNamePopup
	call_if_eq VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 2, LittlerootTown_EventScript_LeftLabAfterDexUpgrade
	end

LittlerootTown_EventScript_LeftLabAfterDexUpgrade::
	setvar VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 3
	return

LittlerootTown_EventScript_HideMapNamePopup::
	setflag FLAG_HIDE_MAP_NAME_POPUP
	return

LittlerootTown_EventScript_SetRivalLeftForRoute103::
	setflag FLAG_RIVAL_LEFT_FOR_ROUTE103
	return

LittlerootTown_EventScript_MoveRivalFromOldale::
	setvar VAR_OLDALE_RIVAL_STATE, 2
	setflag FLAG_HIDE_OLDALE_TOWN_RIVAL
	return

LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent::
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 5
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 5
	return

LittlerootTown_EventScript_MoveMomToMaysDoor::
	setobjectxyperm LOCALID_LITTLEROOT_MOM, 14, 8
	return

LittlerootTown_EventScript_SetTwinPos::
	goto_if_eq VAR_LITTLEROOT_TOWN_STATE, 0, LittlerootTown_EventScript_SetTwinGuardingRoutePos
	setobjectxyperm LOCALID_LITTLEROOT_TWIN, 10, 1
	setobjectmovementtype LOCALID_LITTLEROOT_TWIN, MOVEMENT_TYPE_FACE_UP
	return

LittlerootTown_EventScript_SetTwinGuardingRoutePos::
	setobjectxyperm LOCALID_LITTLEROOT_TWIN, 7, 2
	setobjectmovementtype LOCALID_LITTLEROOT_TWIN, MOVEMENT_TYPE_FACE_DOWN
	return

LittlerootTown_EventScript_SetMomStandingInFrontOfDoorPos::
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setobjectmovementtype LOCALID_LITTLEROOT_MOM, MOVEMENT_TYPE_FACE_DOWN
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_SetMomInFrontOfDoorMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_SetMomInFrontOfDoorFemale
	return

LittlerootTown_EventScript_SetMomInFrontOfDoorMale::
	setobjectxyperm LOCALID_LITTLEROOT_MOM, 5, 9
	return

LittlerootTown_EventScript_SetMomInFrontOfDoorFemale::
	setobjectxyperm LOCALID_LITTLEROOT_MOM, 14, 9
	return

LittlerootTown_OnFrame:
	map_script_2 VAR_LITTLEROOT_INTRO_STATE, 1, LittlerootTown_EventScript_StepOffTruckMale
	map_script_2 VAR_LITTLEROOT_INTRO_STATE, 2, LittlerootTown_EventScript_StepOffTruckFemale
	map_script_2 VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_EventScript_BeginDexUpgradeScene
	.2byte 0

LittlerootTown_EventScript_StepOffTruckMale::
	lockall
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 8
	call LittlerootTown_EventScript_GoInsideWithMom
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	warpsilent MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F, 8, 8
	waitstate
	releaseall
	end

LittlerootTown_EventScript_StepOffTruckFemale::
	lockall
	setvar VAR_0x8004, 14
	setvar VAR_0x8005, 8
	call LittlerootTown_EventScript_GoInsideWithMom
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	warpsilent MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F, 2, 8
	waitstate
	releaseall
	end

LittlerootTown_EventScript_GoInsideWithMom::
	delay 15
	playse SE_LEDGE
	applywaitmovement LOCALID_PLAYER, LittlerootTown_Movement_PlayerStepOffTruck
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	addobject LOCALID_LITTLEROOT_MOM
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitHouse
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	delay 10
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerAtTruck
	msgbox LittlerootTown_Text_OurNewHomeLetsGoInside, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachDoor
	applymovement LOCALID_PLAYER, LittlerootTown_Movement_PlayerApproachDoor
	waitmovement 0
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	applymovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomEnterHouse
	applymovement LOCALID_PLAYER, LittlerootTown_Movement_PlayerEnterHouse
	waitmovement 0
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setvar VAR_LITTLEROOT_INTRO_STATE, 3
	hideplayer
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN
	clearflag FLAG_HIDE_MAP_NAME_POPUP
	return

LittlerootTown_Movement_MomExitHouse:
	walk_down
	step_end

LittlerootTown_Movement_MomApproachPlayerAtTruck:
	walk_down
	walk_in_place_faster_left
	step_end

LittlerootTown_Movement_MomApproachDoor:
	delay_16
	delay_8
	walk_up
	step_end

LittlerootTown_Movement_MomEnterHouse:
	walk_up
	set_invisible
	step_end

LittlerootTown_Movement_PlayerApproachDoor:
	delay_16
	delay_8
	walk_right
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_PlayerEnterHouse:
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_PlayerStepOffTruck:
	jump_right
	delay_16
	delay_16
	delay_16
	step_end

LittlerootTown_EventScript_BeginDexUpgradeScene::
	lockall
	playse SE_PIN
	applywaitmovement LOCALID_LITTLEROOT_BIRCH, Common_Movement_ExclamationMark
	delay 80
	msgbox LittlerootTown_Text_BirchSomethingToShowYouAtLab, MSGBOX_DEFAULT
	closemessage
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_RIVAL
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCH
	delay 20
	clearflag FLAG_HIDE_MAP_NAME_POPUP
	warp MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 6, 5
	waitstate
	releaseall
	end

LittlerootTown_OnWarp:
	map_script_2 VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_EventScript_SetRivalBirchPosForDexUpgrade
	.2byte 0

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgrade::
	addobject LOCALID_LITTLEROOT_BIRCH
	addobject LOCALID_LITTLEROOT_RIVAL
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeMale
	goto LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeFemale
	end

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeMale::
	setobjectxy LOCALID_LITTLEROOT_RIVAL, 6, 10
	setobjectxy LOCALID_LITTLEROOT_BIRCH, 5, 10
	end

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeFemale::
	setobjectxy LOCALID_LITTLEROOT_RIVAL, 13, 10
	setobjectxy LOCALID_LITTLEROOT_BIRCH, 14, 10
	end

LittlerootTown_EventScript_FatMan::
	msgbox LittlerootTown_Text_CanUsePCToStoreItems, MSGBOX_NPC
	end

LittlerootTown_EventScript_Boy::
	msgbox LittlerootTown_Text_BirchSpendsDaysInLab, MSGBOX_NPC
	end

LittlerootTown_EventScript_Twin::
	lock
	faceplayer
	goto_if_set FLAG_ADVENTURE_STARTED, LittlerootTown_EventScript_GoodLuck
	goto_if_set FLAG_RESCUED_BIRCH, LittlerootTown_EventScript_YouSavedBirch
	goto_if_ne VAR_LITTLEROOT_TOWN_STATE, 0, LittlerootTown_EventScript_GoSaveBirch
	msgbox LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_GoSaveBirch::
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_CanYouGoSeeWhatsHappening, MSGBOX_DEFAULT
	closemessage
	applywaitmovement LOCALID_LITTLEROOT_TWIN, Common_Movement_FaceOriginalDirection
	setvar VAR_LITTLEROOT_TOWN_STATE, 2
	release
	end

LittlerootTown_EventScript_YouSavedBirch::
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_YouSavedBirch, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_GoodLuck::
	msgbox LittlerootTown_Text_GoodLuckCatchingPokemon, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_NeedPokemonTriggerLeft::
	lockall
	applywaitmovement LOCALID_LITTLEROOT_TWIN, LittlerootTown_Movement_TwinApproachPlayerLeft
	call LittlerootTown_EventScript_DangerousWithoutPokemon
	applywaitmovement LOCALID_LITTLEROOT_TWIN, LittlerootTown_Movement_TwinReturnLeft
	releaseall
	end

LittlerootTown_EventScript_DangerousWithoutPokemon::
	msgbox LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_LITTLEROOT_TWIN, LittlerootTown_Movement_TwinPushPlayerFromRoute
	applymovement LOCALID_PLAYER, LittlerootTown_Movement_PushPlayerBackFromRoute
	waitmovement 0
	msgbox LittlerootTown_Text_DangerousIfYouDontHavePokemon, MSGBOX_DEFAULT
	closemessage
	return

LittlerootTown_Movement_TwinApproachPlayerLeft:
	face_right
	delay_8
	disable_jump_landing_ground_effect
	jump_in_place_right
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_left
	face_down
	step_end

LittlerootTown_Movement_TwinPushPlayerFromRoute:
	walk_down
	step_end

LittlerootTown_Movement_TwinReturnLeft:
	walk_right
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_in_place_faster_down
	step_end

LittlerootTown_Movement_PushPlayerBackFromRoute:
	lock_facing_direction
	walk_down
	unlock_facing_direction
	step_end

LittlerootTown_EventScript_NeedPokemonTriggerRight::
	lockall
	applywaitmovement LOCALID_LITTLEROOT_TWIN, LittlerootTown_Movement_TwinApproachPlayerRight
	call LittlerootTown_EventScript_DangerousWithoutPokemon
	applywaitmovement LOCALID_LITTLEROOT_TWIN, LittlerootTown_Movement_TwinReturnRight
	releaseall
	end

LittlerootTown_Movement_TwinApproachPlayerRight:
	face_right
	delay_8
	disable_jump_landing_ground_effect
	jump_in_place_right
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_right
	face_down
	step_end

LittlerootTown_Movement_TwinReturnRight:
	walk_left
	walk_down
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_down
	step_end

LittlerootTown_EventScript_GoSaveBirchTrigger::
	lockall
	applywaitmovement LOCALID_LITTLEROOT_TWIN, Common_Movement_WalkInPlaceFasterRight
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_CanYouGoSeeWhatsHappening, MSGBOX_DEFAULT
	closemessage
	applywaitmovement LOCALID_LITTLEROOT_TWIN, Common_Movement_FaceOriginalDirection
	setvar VAR_LITTLEROOT_TOWN_STATE, 2
	releaseall
	end

LittlerootTown_EventScript_TownSign::
	msgbox LittlerootTown_Text_TownSign, MSGBOX_SIGN
	end

@ A Pokémon Storage PC out on the farm, so workers can be swapped without a trip
@ to Oldale. It's an object, so it skips the PC screen flicker on the map.
LittlerootTown_EventScript_FarmPC::
	lockall
	setvar VAR_0x8004, PC_LOCATION_OTHER
	playse SE_PC_ON
	msgbox Text_BootUpPC, MSGBOX_DEFAULT
	goto EventScript_PCMainMenu
	end

LittlerootTown_EventScript_RuinsSign_DeliveryService::
	goto_if_set FLAG_DELIVERY_SERVICE_BUILT, LittlerootTown_EventScript_DeliveryServiceSign
	msgbox LittlerootTown_Text_RuinsSign_DeliveryService, MSGBOX_SIGN
	end

LittlerootTown_EventScript_DeliveryServiceSign::
	msgbox LittlerootTown_Text_DeliveryServiceSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Mart::
	goto_if_set FLAG_LITTLEROOT_MART_BUILT, LittlerootTown_EventScript_MartSign
	msgbox LittlerootTown_Text_RuinsSign_Mart, MSGBOX_SIGN
	end

LittlerootTown_EventScript_MartSign::
	msgbox LittlerootTown_Text_MartSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Forge::
	goto_if_set FLAG_FORGE_BUILT, LittlerootTown_EventScript_ForgeSign
	msgbox LittlerootTown_Text_RuinsSign_Forge, MSGBOX_SIGN
	end

LittlerootTown_EventScript_ForgeSign::
	msgbox LittlerootTown_Text_ForgeSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Greenhouse::
	goto_if_set FLAG_GREENHOUSE_BUILT, LittlerootTown_EventScript_GreenhouseSign
	msgbox LittlerootTown_Text_RuinsSign_Greenhouse, MSGBOX_SIGN
	end

LittlerootTown_EventScript_GreenhouseSign::
	msgbox LittlerootTown_Text_GreenhouseSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_IceDome::
	goto_if_set FLAG_ICE_DOME_BUILT, LittlerootTown_EventScript_IceDomeSign
	msgbox LittlerootTown_Text_RuinsSign_IceDome, MSGBOX_SIGN
	end

LittlerootTown_EventScript_IceDomeSign::
	msgbox LittlerootTown_Text_IceDomeSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_WeatherStabilizer::
	msgbox LittlerootTown_Text_RuinsSign_WeatherStabilizer, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Generator::
	goto_if_set FLAG_GENERATOR_BUILT, LittlerootTown_EventScript_GeneratorSign
	msgbox LittlerootTown_Text_RuinsSign_Generator, MSGBOX_SIGN
	end

LittlerootTown_EventScript_GeneratorSign::
	msgbox LittlerootTown_Text_GeneratorSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Fishery::
	goto_if_set FLAG_FISHERY_BUILT, LittlerootTown_EventScript_FisherySign
	msgbox LittlerootTown_Text_RuinsSign_Fishery, MSGBOX_SIGN
	end

LittlerootTown_EventScript_FisherySign::
	msgbox LittlerootTown_Text_FisherySign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_SpiritSanctum::
	goto_if_set FLAG_SPIRIT_SANCTUM_BUILT, LittlerootTown_EventScript_SpiritSanctumSign
	msgbox LittlerootTown_Text_RuinsSign_SpiritSanctum, MSGBOX_SIGN
	end

LittlerootTown_EventScript_SpiritSanctumSign::
	msgbox LittlerootTown_Text_SpiritSanctumSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_RuinsSign_Quarry::
	goto_if_set FLAG_QUARRY_BUILT, LittlerootTown_EventScript_QuarrySign
	msgbox LittlerootTown_Text_RuinsSign_Quarry, MSGBOX_SIGN
	end

LittlerootTown_EventScript_QuarrySign::
	msgbox LittlerootTown_Text_QuarrySign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_BirchsLabSign::
	msgbox LittlerootTown_Text_ProfBirchsLab, MSGBOX_SIGN
	end

LittlerootTown_EventScript_BrendansHouseSign::
	lockall
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_PlayersHouseSignMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_BirchsHouseSignFemale
	releaseall
	end

LittlerootTown_EventScript_PlayersHouseSignMale::
	msgbox LittlerootTown_Text_PlayersHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_BirchsHouseSignFemale::
	msgbox LittlerootTown_Text_ProfBirchsHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_MaysHouseSign::
	lockall
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_BirchsHouseSignMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_PlayersHouseSignFemale
	releaseall
	end

LittlerootTown_EventScript_BirchsHouseSignMale::
	msgbox LittlerootTown_Text_ProfBirchsHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_PlayersHouseSignFemale::
	msgbox LittlerootTown_Text_PlayersHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_GiveRunningShoesTrigger0::
	lockall
	setvar VAR_0x8008, 0
	setobjectxy LOCALID_LITTLEROOT_MOM, 10, 9
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger1::
	lockall
	setvar VAR_0x8008, 1
	setobjectxy LOCALID_LITTLEROOT_MOM, 11, 9
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger2::
	lockall
	setvar VAR_0x8008, 2
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger3::
	lockall
	setvar VAR_0x8008, 3
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger4::
	lockall
	setvar VAR_0x8008, 4
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger5::
	lockall
	setvar VAR_0x8008, 5
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_MomNoticePlayerMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_MomNoticePlayerFemale
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_SetHomeDoorCoordsMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_SetHomeDoorCoordsFemale
	msgbox LittlerootTown_Text_WaitPlayer, MSGBOX_DEFAULT
	closemessage
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_MomApproachPlayerMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_MomApproachPlayerFemale
	call LittlerootTown_EventScript_GiveRunningShoes
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_MomReturnHomeMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_MomReturnHomeFemale
	goto LittlerootTown_EventScript_SetReceivedRunningShoes
	end

LittlerootTown_EventScript_SetHomeDoorCoordsMale::
	setvar VAR_0x8009, 5
	setvar VAR_0x800A, 8
	return

LittlerootTown_EventScript_SetHomeDoorCoordsFemale::
	setvar VAR_0x8009, 14
	setvar VAR_0x800A, 8
	return

LittlerootTown_EventScript_MomNoticePlayerMale::
	applywaitmovement LOCALID_LITTLEROOT_MOM, Common_Movement_WalkInPlaceFasterRight
	return

LittlerootTown_EventScript_MomNoticePlayerFemale::
	applywaitmovement LOCALID_LITTLEROOT_MOM, Common_Movement_WalkInPlaceFasterLeft
	return

LittlerootTown_EventScript_MomApproachPlayerMale::
	call_if_eq VAR_0x8008, 0, LittlerootTown_EventScript_MomApproachPlayer0
	call_if_eq VAR_0x8008, 1, LittlerootTown_EventScript_MomApproachPlayer1
	call_if_eq VAR_0x8008, 2, LittlerootTown_EventScript_MomApproachPlayerMale2
	call_if_eq VAR_0x8008, 3, LittlerootTown_EventScript_MomApproachPlayerMale3
	call_if_eq VAR_0x8008, 4, LittlerootTown_EventScript_MomApproachPlayerMale4
	call_if_eq VAR_0x8008, 5, LittlerootTown_EventScript_MomApproachPlayerMale5
	return

LittlerootTown_EventScript_MomApproachPlayerFemale::
	call_if_eq VAR_0x8008, 0, LittlerootTown_EventScript_MomApproachPlayer0
	call_if_eq VAR_0x8008, 1, LittlerootTown_EventScript_MomApproachPlayer1
	call_if_eq VAR_0x8008, 2, LittlerootTown_EventScript_MomApproachPlayerFemale2
	call_if_eq VAR_0x8008, 3, LittlerootTown_EventScript_MomApproachPlayerFemale3
	call_if_eq VAR_0x8008, 4, LittlerootTown_EventScript_MomApproachPlayerFemale4
	call_if_eq VAR_0x8008, 5, LittlerootTown_EventScript_MomApproachPlayerFemale5
	return

LittlerootTown_EventScript_MomApproachPlayer0::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayer0
	return

LittlerootTown_EventScript_MomApproachPlayer1::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayer1
	return

LittlerootTown_EventScript_MomApproachPlayerMale2::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerMale2
	return

LittlerootTown_EventScript_MomApproachPlayerMale3::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerMale3
	return

LittlerootTown_EventScript_MomApproachPlayerMale4::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerMale4
	return

LittlerootTown_EventScript_MomApproachPlayerMale5::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerMale5
	return

LittlerootTown_EventScript_MomApproachPlayerFemale2::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerFemale2
	return

LittlerootTown_EventScript_MomApproachPlayerFemale3::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerFemale3
	return

LittlerootTown_EventScript_MomApproachPlayerFemale4::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerFemale4
	return

LittlerootTown_EventScript_MomApproachPlayerFemale5::
	applywaitmovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomApproachPlayerFemale5
	return

LittlerootTown_EventScript_MomReturnHomeMale::
	call_if_eq VAR_0x8008, 0, LittlerootTown_EventScript_MomReturnHome0
	call_if_eq VAR_0x8008, 1, LittlerootTown_EventScript_MomReturnHome1
	call_if_eq VAR_0x8008, 2, LittlerootTown_EventScript_MomReturnHomeMale2
	call_if_eq VAR_0x8008, 3, LittlerootTown_EventScript_MomReturnHomeMale3
	call_if_eq VAR_0x8008, 4, LittlerootTown_EventScript_MomReturnHomeMale4
	call_if_eq VAR_0x8008, 5, LittlerootTown_EventScript_MomReturnHomeMale5
	return

LittlerootTown_EventScript_MomReturnHomeFemale::
	call_if_eq VAR_0x8008, 0, LittlerootTown_EventScript_MomReturnHome0
	call_if_eq VAR_0x8008, 1, LittlerootTown_EventScript_MomReturnHome1
	call_if_eq VAR_0x8008, 2, LittlerootTown_EventScript_MomReturnHomeFemale2
	call_if_eq VAR_0x8008, 3, LittlerootTown_EventScript_MomReturnHomeFemale3
	call_if_eq VAR_0x8008, 4, LittlerootTown_EventScript_MomReturnHomeFemale4
	call_if_eq VAR_0x8008, 5, LittlerootTown_EventScript_MomReturnHomeFemale5
	return

LittlerootTown_EventScript_MomReturnHome0::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHome0
	return

LittlerootTown_EventScript_MomReturnHome1::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHome1
	return

LittlerootTown_EventScript_MomReturnHomeMale2::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeMale2
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale3::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeMale3
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale4::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeMale4
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale5::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeMale5
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale2::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeFemale2
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale3::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeFemale3
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale4::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeFemale4
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale5::
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomReturnHomeFemale5
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_Movement_MomApproachPlayer0:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_MomApproachPlayer1:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_MomApproachPlayerMale2:
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale3:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale4:
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale5:
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale2:
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale3:
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale4:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale5:
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomReturnHome0:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

LittlerootTown_Movement_MomReturnHome1:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

LittlerootTown_Movement_MomReturnHomeMale2:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale3:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale4:
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale5:
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale2:
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale3:
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale4:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale5:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

LittlerootTown_Movement_MomExitThroughDoor:
	walk_up
	step_end

LittlerootTown_EventScript_Mom::
	lock
	faceplayer
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LittlerootTown_EventScript_SetHomeDoorCoordsMale
	call_if_eq VAR_RESULT, FEMALE, LittlerootTown_EventScript_SetHomeDoorCoordsFemale
	call LittlerootTown_EventScript_GiveRunningShoes
	applywaitmovement LOCALID_LITTLEROOT_MOM, Common_Movement_WalkInPlaceFasterUp
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applywaitmovement LOCALID_LITTLEROOT_MOM, LittlerootTown_Movement_MomExitThroughDoor
	hideobjectat LOCALID_LITTLEROOT_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	goto LittlerootTown_EventScript_SetReceivedRunningShoes
	end

LittlerootTown_EventScript_SetReceivedRunningShoes::
	removeobject LOCALID_LITTLEROOT_MOM
	setflag FLAG_SYS_B_DASH
	setvar VAR_LITTLEROOT_TOWN_STATE, 4
	release
	end

LittlerootTown_EventScript_GiveRunningShoes::
	msgbox LittlerootTown_Text_WearTheseRunningShoes, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message LittlerootTown_Text_SwitchShoesWithRunningShoes
	waitfanfare
	setflag FLAG_RECEIVED_RUNNING_SHOES
	msgbox LittlerootTown_Text_ExplainRunningShoes, MSGBOX_DEFAULT
	msgbox LittlerootTown_Text_ComeHomeIfAnythingHappens, MSGBOX_DEFAULT
	closemessage
	delay 30
	return

LittlerootTown_Text_OurNewHomeLetsGoInside:
	.string "MOM: {PLAYER}, we're here, honey!\p"
	.string "It must be tiring riding with our things\n"
	.string "in the moving truck.\p"
	.string "Well, this is LITTLEROOT TOWN.\p"
	.string "How do you like it?\n"
	.string "This is our new home!\p"
	.string "It has a quaint feel, but it seems to be\n"
	.string "an easy place to live, don't you think?\p"
	.string "And, you get your own room, {PLAYER}!\n"
	.string "Let's go inside.$"

LittlerootTown_Text_WaitPlayer:
	.string "MOM: Wait, {PLAYER}!$"

LittlerootTown_Text_WearTheseRunningShoes:
	.string "MOM: {PLAYER}! {PLAYER}! Did you\n"
	.string "introduce yourself to PROF. BIRCH?\p"
	.string "Oh! What an adorable POKéMON!\n"
	.string "You got it from PROF. BIRCH. How nice!\p"
	.string "You're your father's child, all right.\n"
	.string "You look good together with POKéMON!\p"
	.string "Here, honey! If you're going out on an\n"
	.string "adventure, wear these RUNNING SHOES.\p"
	.string "They'll put a zip in your step!$"

LittlerootTown_Text_SwitchShoesWithRunningShoes:
	.string "{PLAYER} switched shoes with the\n"
	.string "RUNNING SHOES.$"

LittlerootTown_Text_ExplainRunningShoes:
	.string "MOM: {PLAYER}, those shoes came with\n"
	.string "instructions.\p"
	.string "“Press the B Button while wearing these\n"
	.string "RUNNING SHOES to run extra-fast!\p"
	.string "“Slip on these RUNNING SHOES and race\n"
	.string "in the great outdoors!”$"

LittlerootTown_Text_ComeHomeIfAnythingHappens:
	.string "… … … … … … … …\n"
	.string "… … … … … … … …\p"
	.string "To think that you have your very own\n"
	.string "POKéMON now…\l"
	.string "Your father will be overjoyed.\p"
	.string "…But please be careful.\n"
	.string "If anything happens, you can come home.\p"
	.string "Go on, go get them, honey!$"

LittlerootTown_Text_CanUsePCToStoreItems:
	.string "If you use a PC, you can store items\n"
	.string "and POKéMON.\p"
	.string "The power of science is staggering!$"

LittlerootTown_Text_BirchSpendsDaysInLab:
	.string "PROF. BIRCH spends days in his LAB\n"
	.string "studying, then he'll suddenly go out in\l"
	.string "the wild to do more research…\p"
	.string "When does PROF. BIRCH spend time\n"
	.string "at home?$"

LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut:
	.string "Um, um, um!\p"
	.string "If you go outside and go in the grass,\n"
	.string "wild POKéMON will jump out!$"

LittlerootTown_Text_DangerousIfYouDontHavePokemon:
	.string "It's dangerous if you don't have\n"
	.string "your own POKéMON.$"

LittlerootTown_Text_CanYouGoSeeWhatsHappening:
	.string "Um, hi!\p"
	.string "There are scary POKéMON outside!\n"
	.string "I can hear their cries!\p"
	.string "I want to go see what's going on,\n"
	.string "but I don't have any POKéMON…\p"
	.string "Can you go see what's happening\n"
	.string "for me?$"

LittlerootTown_Text_YouSavedBirch:
	.string "You saved PROF. BIRCH!\n"
	.string "I'm so glad!$"

LittlerootTown_Text_GoodLuckCatchingPokemon:
	.string "Are you going to catch POKéMON?\n"
	.string "Good luck!$"

LittlerootTown_Text_TownSign:
	.string "LITTLEROOT TOWN\n"
	.string "“A town that can't be shaded any hue.”$"

LittlerootTown_Text_ProfBirchsLab:
	.string "PROF. BIRCH'S POKéMON LAB$"

LittlerootTown_Text_PlayersHouse:
	.string "{PLAYER}'s HOUSE$"

LittlerootTown_Text_ProfBirchsHouse:
	.string "PROF. BIRCH'S HOUSE$"

LittlerootTown_Text_BirchSomethingToShowYouAtLab:
	.string "PROF. BIRCH: Well, well, {PLAYER}{KUN}!\n"
	.string "That was good work out there!\p"
	.string "I knew there was something special\n"
	.string "about you when I first saw you,\l"
	.string "but I never expected this.\p"
	.string "Oh, yes. Do you still have the POKéDEX\n"
	.string "I gave you?\p"
	.string "I have something to show you.\n"
	.string "Let's go to my LAB.$"

LittlerootTown_Text_RuinsSign_DeliveryService:
	.string "DELIVERY SERVICE (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_Mart:
	.string "POKé MART (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_MartSign:
	.string "POKé MART\n"
	.string "Now buying farm goods!$"

LittlerootTown_Text_RuinsSign_Forge:
	.string "FORGE (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_Greenhouse:
	.string "GREENHOUSE (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_IceDome:
	.string "ICE DOME (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_WeatherStabilizer:
	.string "WEATHER STABILIZER (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_Generator:
	.string "GENERATOR (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_Fishery:
	.string "FISHERY (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_SpiritSanctum:
	.string "SPIRIT SANCTUM (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_RuinsSign_Quarry:
	.string "QUARRY (RUINS)\n"
	.string "Maybe it can be rebuilt someday.$"

LittlerootTown_Text_ForgeSign:
	.string "FORGE\n"
	.string "Fine POKé BALLS, forged by hand!$"

LittlerootTown_Text_GreenhouseSign:
	.string "GREENHOUSE\n"
	.string "Fresh BERRIES all year round!$"

LittlerootTown_Text_IceDomeSign:
	.string "ICE DOME\n"
	.string "Keeping HOENN cool!$"

LittlerootTown_Text_GeneratorSign:
	.string "GENERATOR\n"
	.string "Power for the whole town!$"

LittlerootTown_Text_FisherySign:
	.string "FISHERY\n"
	.string "Today's catch, fresh from the pond!$"

LittlerootTown_Text_SpiritSanctumSign:
	.string "SPIRIT SANCTUM\n"
	.string "Charms to soothe restless spirits.$"

LittlerootTown_Text_QuarrySign:
	.string "QUARRY\n"
	.string "EVOLUTION STONES, dug fresh daily!$"

@ ---------------------------------------------------------------------------
@ Emerald Vault: the Builder and the businesses' terminals (src/vault_town.c)
@ ---------------------------------------------------------------------------

@ After rebuilding the Delivery Service, the Builder stands in the yard
@ between the old town and the farm.
LittlerootTown_EventScript_SetBuilderInYard::
	setobjectxyperm LOCALID_LITTLEROOT_BUILDER, 21, 12
	setobjectmovementtype LOCALID_LITTLEROOT_BUILDER, MOVEMENT_TYPE_FACE_RIGHT
	return

LittlerootTown_EventScript_Builder::
	lock
	faceplayer
	goto_if_unset FLAG_DELIVERY_SERVICE_BUILT, LittlerootTown_EventScript_BuilderFirstBuilding
	special Town_HasBuilderOffers
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_BuilderNothingToBuild
	message LittlerootTown_Text_BuilderWhatToRebuild
	waitmessage
	special Town_ShowBuilderMenu
	goto_if_eq VAR_RESULT, TOWN_MENU_EMPTY, LittlerootTown_EventScript_BuilderNothingToBuild
	goto_if_eq VAR_RESULT, TOWN_MENU_CANCELLED, LittlerootTown_EventScript_BuilderGoodbye
	copyvar VAR_0x8006, VAR_RESULT
	goto_if_eq VAR_0x8006, TOWN_MART, LittlerootTown_EventScript_BuilderMart
	special Town_BufferBuildingInfo
	special Town_CanAffordBuilding
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_BuilderCantAfford
	goto_if_ge VAR_0x8005, 2, LittlerootTown_EventScript_BuilderUpgrade
	special Town_CheckPartyForWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_BuilderNeedWorker
	showmoneybox 0, 0
	@ Drawing money boxes reuses STR_VAR_1, so buffer the building again.
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_BuilderConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_EventScript_BuilderDeclined
	special Town_BuyBuilding
	updatemoneybox
	playse SE_SHOP
	waitse
	hidemoneybox
	msgbox LittlerootTown_Text_BuilderStandBack, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	special Town_DrawBuildings
	special DrawWholeMapView
	addobject VAR_0x8007
	waitse
	delay 30
	fadescreen FADE_FROM_BLACK
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_BuilderBuilt, MSGBOX_DEFAULT
	release
	end

@ Upgrades add a worker slot and a better item tier; the building looks the same.
LittlerootTown_EventScript_BuilderUpgrade::
	showmoneybox 0, 0
	@ Drawing money boxes reuses STR_VAR_1, so buffer the building again.
	special Town_BufferBuildingInfo
	buffernumberstring STR_VAR_3, VAR_0x8005
	msgbox LittlerootTown_Text_BuilderConfirmUpgrade, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_EventScript_BuilderDeclined
	special Town_BuyBuilding
	updatemoneybox
	playse SE_SHOP
	waitse
	hidemoneybox
	msgbox LittlerootTown_Text_BuilderStandBack, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	waitse
	delay 30
	fadescreen FADE_FROM_BLACK
	special Town_BufferBuildingInfo
	buffernumberstring STR_VAR_3, VAR_0x8005
	msgbox LittlerootTown_Text_BuilderUpgraded, MSGBOX_DEFAULT
	release
	end

@ The Poké Mart has no workers or terminal: just pay and build.
LittlerootTown_EventScript_BuilderMart::
	special Town_BufferBuildingInfo
	special Town_CanAffordBuilding
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_BuilderCantAfford
	showmoneybox 0, 0
	@ Drawing money boxes reuses STR_VAR_1, so buffer the building again.
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_BuilderConfirmMart, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_EventScript_BuilderDeclined
	special Town_BuyBuilding
	updatemoneybox
	playse SE_SHOP
	waitse
	hidemoneybox
	msgbox LittlerootTown_Text_BuilderStandBack, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	special Town_DrawBuildings
	special DrawWholeMapView
	waitse
	delay 30
	fadescreen FADE_FROM_BLACK
	msgbox LittlerootTown_Text_BuilderBuiltMart, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_BuilderDeclined::
	hidemoneybox
LittlerootTown_EventScript_BuilderGoodbye::
	msgbox LittlerootTown_Text_BuilderGoodbye, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_BuilderCantAfford::
	msgbox LittlerootTown_Text_BuilderCantAfford, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_BuilderNeedWorker::
	msgbox LittlerootTown_Text_BuilderNeedWorker, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_BuilderNothingToBuild::
	msgbox LittlerootTown_Text_BuilderNothingToBuild, MSGBOX_DEFAULT
	release
	end

@ The Delivery Service is free: the Builder just needs to see a Flying-type.
LittlerootTown_EventScript_BuilderFirstBuilding::
	goto_if_set FLAG_MET_BUILDER, LittlerootTown_EventScript_BuilderCheckForFlyingType
	msgbox LittlerootTown_Text_BuilderIntro, MSGBOX_DEFAULT
	setflag FLAG_MET_BUILDER
LittlerootTown_EventScript_BuilderCheckForFlyingType::
	setvar VAR_0x8006, TOWN_DELIVERY_SERVICE
	special Town_CheckPartyForWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_BuilderNeedFlyingType
	msgbox LittlerootTown_Text_BuilderSeesFlyingType, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	special Town_BuyBuilding
	special Town_DrawBuildings
	special DrawWholeMapView
	addobject VAR_0x8007
	waitse
	delay 30
	fadescreen FADE_FROM_BLACK
	msgbox LittlerootTown_Text_BuilderDone, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	call LittlerootTown_EventScript_SetBuilderInYard
	setobjectxy LOCALID_LITTLEROOT_BUILDER, 21, 12
	turnobject LOCALID_LITTLEROOT_BUILDER, DIR_EAST
	fadescreen FADE_FROM_BLACK
	release
	end

LittlerootTown_EventScript_BuilderNeedFlyingType::
	msgbox LittlerootTown_Text_BuilderNeedFlyingType, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_DeliveryTerminal::
	lock
	setvar VAR_0x8006, TOWN_DELIVERY_SERVICE
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_GreenhouseTerminal::
	lock
	setvar VAR_0x8006, TOWN_GREENHOUSE
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_QuarryTerminal::
	lock
	setvar VAR_0x8006, TOWN_QUARRY
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_FisheryTerminal::
	lock
	setvar VAR_0x8006, TOWN_FISHERY
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_GeneratorTerminal::
	lock
	setvar VAR_0x8006, TOWN_GENERATOR
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_ForgeTerminal::
	lock
	setvar VAR_0x8006, TOWN_FORGE
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_SpiritSanctumTerminal::
	lock
	setvar VAR_0x8006, TOWN_SPIRIT_SANCTUM
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_IceDomeTerminal::
	lock
	setvar VAR_0x8006, TOWN_ICE_DOME
	goto LittlerootTown_EventScript_Terminal

LittlerootTown_EventScript_Terminal::
	playse SE_PC_ON
	special Town_WasEverStaffed
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalFirstWorker
	special Town_HasWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalNoWorker
	special Town_BufferStock
	showmonpic VAR_0x8008, 1, 1
	special Town_BufferStatus
	goto_if_eq VAR_RESULT, TRUE, LittlerootTown_EventScript_TerminalTeamStatus
	msgbox LittlerootTown_Text_TerminalWorkerStatus, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalTeamStatus::
	msgbox LittlerootTown_Text_TerminalTeamStatus, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalNoWorker::
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_TerminalNoWorker, MSGBOX_DEFAULT
LittlerootTown_EventScript_TerminalMenu::
	special Town_BufferStock
	call_if_ne VAR_0x8008, SPECIES_NONE, LittlerootTown_EventScript_TerminalShowWorkerPic
	message LittlerootTown_Text_TerminalStock
	waitmessage
	goto_if_eq VAR_0x8006, TOWN_DELIVERY_SERVICE, LittlerootTown_EventScript_DeliveryTerminalMenu
	multichoice 17, 2, MULTI_TOWN_TERMINAL, FALSE
	switch VAR_RESULT
	case TOWN_TERMINAL_COLLECT, LittlerootTown_EventScript_TerminalCollect
	case TOWN_TERMINAL_ORDER, LittlerootTown_EventScript_TerminalOrder
	case TOWN_TERMINAL_CHANGE_WORKER, LittlerootTown_EventScript_TerminalChooseWorker
	case TOWN_TERMINAL_TAKE_BACK, LittlerootTown_EventScript_TerminalTakeBackWorker
	goto LittlerootTown_EventScript_TerminalLogOff

@ The Delivery Service's terminal also books flights to visited towns.
LittlerootTown_EventScript_DeliveryTerminalMenu::
	multichoice 17, 0, MULTI_DELIVERY_TERMINAL, FALSE
	switch VAR_RESULT
	case DELIVERY_TERMINAL_COLLECT, LittlerootTown_EventScript_TerminalCollect
	case DELIVERY_TERMINAL_ORDER, LittlerootTown_EventScript_TerminalOrder
	case DELIVERY_TERMINAL_FLY, LittlerootTown_EventScript_DeliveryFly
	case DELIVERY_TERMINAL_CHANGE_WORKER, LittlerootTown_EventScript_TerminalChooseWorker
	case DELIVERY_TERMINAL_TAKE_BACK, LittlerootTown_EventScript_TerminalTakeBackWorker
	goto LittlerootTown_EventScript_TerminalLogOff

@ Does nothing if the picture is already up.
LittlerootTown_EventScript_TerminalShowWorkerPic::
	showmonpic VAR_0x8008, 1, 1
	return

LittlerootTown_EventScript_TerminalLogOff::
	hidemonpic
	closemessage
	playse SE_PC_OFF
	release
	end

@ Moves each kind of item into the bag. Saves from before items only may also
@ have money left over, which is paid out first.
LittlerootTown_EventScript_TerminalCollect::
	special Town_BufferStock
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalNothingToCollect
	special Town_CollectEarnings
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalCollectItems
	playse SE_SHOP
	msgbox LittlerootTown_Text_TerminalEarnings, MSGBOX_DEFAULT
LittlerootTown_EventScript_TerminalCollectItems::
	setvar VAR_0x8009, 0
LittlerootTown_EventScript_TerminalCollectNextItem::
	special Town_CollectNextItem
	goto_if_eq VAR_RESULT, TOWN_ITEMS_DONE, LittlerootTown_EventScript_TerminalMenu
	goto_if_eq VAR_RESULT, TOWN_ITEM_NO_ROOM, LittlerootTown_EventScript_TerminalNoRoomForItem
	playse SE_SELECT
	msgbox LittlerootTown_Text_TerminalCollectedItem, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalCollectNextItem

LittlerootTown_EventScript_TerminalNoRoomForItem::
	msgbox LittlerootTown_Text_TerminalNoRoomForItem, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalCollectNextItem

LittlerootTown_EventScript_TerminalNothingToCollect::
	msgbox LittlerootTown_Text_TerminalNothingToCollect, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

@ A work order: the building makes only the chosen item (or its usual mix)
@ until the stockroom is full, and keeps on until it's changed.
LittlerootTown_EventScript_TerminalOrder::
	special Town_HasWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalOrderNoWorker
	hidemonpic
	message LittlerootTown_Text_TerminalWhatToMake
	waitmessage
	special Town_ChooseOrderItem
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalMenu
	special Town_SetOrder
	playse SE_SELECT
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalOrderUsualMix
	msgbox LittlerootTown_Text_TerminalOrderSet, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalOrderUsualMix::
	msgbox LittlerootTown_Text_TerminalOrderUsualMix, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalOrderNoWorker::
	msgbox LittlerootTown_Text_TerminalOrderNoWorker, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_DeliveryFly::
	hidemonpic
	msgbox LittlerootTown_Text_DeliveryWhereTo, MSGBOX_DEFAULT
	closemessage
	special Town_OpenFlyMap
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalChooseWorker::
	hidemonpic
	setvar VAR_0x8005, TOWN_SLOTS_TO_ASSIGN
	special Town_ChooseWorkerSlot
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalMenu
	msgbox LittlerootTown_Text_TerminalWhoShouldWork, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, LittlerootTown_EventScript_TerminalLogOff
	special Town_AssignWorker
	goto_if_eq VAR_RESULT, TOWN_ASSIGN_EGG, LittlerootTown_EventScript_TerminalCantUseEgg
	goto_if_eq VAR_RESULT, TOWN_ASSIGN_LAST_MON, LittlerootTown_EventScript_TerminalCantUseLastMon
	special Town_BufferWorker
	msgbox LittlerootTown_Text_TerminalNewWorker, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalTakeBackWorker::
	special Town_HasWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalNoWorkerToTakeBack
	hidemonpic
	setvar VAR_0x8005, TOWN_SLOTS_TO_TAKE_BACK
	special Town_ChooseWorkerSlot
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalMenu
	special Town_BufferWorker
	special Town_TakeBackWorker
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalPartyFull
	hidemonpic
	msgbox LittlerootTown_Text_TerminalWorkerReturned, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalNoWorkerToTakeBack::
	msgbox LittlerootTown_Text_TerminalNoWorkerToTakeBack, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalPartyFull::
	msgbox LittlerootTown_Text_TerminalPartyFull, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalMenu

LittlerootTown_EventScript_TerminalCantUseEgg::
	msgbox LittlerootTown_Text_TerminalCantUseEgg, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalCantUseLastMon::
	msgbox LittlerootTown_Text_TerminalCantUseLastMon, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

@ A business's first worker has to be one of its types. After that, anyone
@ can work there, though those types work best.
LittlerootTown_EventScript_TerminalFirstWorker::
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_TerminalNeedFirstWorker, MSGBOX_DEFAULT
	closemessage
	special ChoosePartyMon
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, LittlerootTown_EventScript_TerminalFirstWorkerLater
	special Town_IsPartyMonSuited
	goto_if_eq VAR_RESULT, FALSE, LittlerootTown_EventScript_TerminalFirstWorkerUnsuited
	setvar VAR_0x800A, 0
	special Town_AssignWorker
	goto_if_eq VAR_RESULT, TOWN_ASSIGN_LAST_MON, LittlerootTown_EventScript_TerminalCantUseLastMon
	special Town_BufferWorker
	msgbox LittlerootTown_Text_TerminalFirstWorkerReady, MSGBOX_DEFAULT
	goto_if_ne VAR_0x8006, TOWN_DELIVERY_SERVICE, LittlerootTown_EventScript_TerminalLogOff
	goto_if_set FLAG_RECEIVED_BIRD_WHISTLE, LittlerootTown_EventScript_TerminalLogOff
	msgbox LittlerootTown_Text_DeliveryTakeWhistle, MSGBOX_DEFAULT
	giveitem ITEM_BIRD_WHISTLE
	setflag FLAG_RECEIVED_BIRD_WHISTLE
	msgbox LittlerootTown_Text_DeliveryExplainFlights, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalFirstWorkerLater::
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_TerminalFirstWorkerLater, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_EventScript_TerminalFirstWorkerUnsuited::
	special Town_BufferBuildingInfo
	msgbox LittlerootTown_Text_TerminalFirstWorkerUnsuited, MSGBOX_DEFAULT
	goto LittlerootTown_EventScript_TerminalLogOff

LittlerootTown_Text_BuilderIntro:
	.string "Hey there! I'm the BUILDER.\p"
	.string "All these ruins used to be\n"
	.string "LITTLEROOT's businesses.\p"
	.string "I can rebuild every one of them,\n"
	.string "and the first one's on me!\p"
	.string "Let's start with the DELIVERY\n"
	.string "SERVICE. It needs a bird to run it.$"

LittlerootTown_Text_BuilderNeedFlyingType:
	.string "Bring me a FLYING-type POKéMON and\n"
	.string "I'll rebuild the DELIVERY SERVICE.\p"
	.string "I hear WINGULL nest in the grass\n"
	.string "on ROUTE 103, north of OLDALE TOWN.$"

LittlerootTown_Text_BuilderSeesFlyingType:
	.string "Is that a FLYING-type POKéMON?\n"
	.string "Perfect! Stand back…$"

LittlerootTown_Text_BuilderDone:
	.string "Ta-da! The DELIVERY SERVICE is\n"
	.string "back in business!\p"
	.string "Use its terminal to put your\n"
	.string "FLYING-type to work.\p"
	.string "I'll set up in the yard by the old\n"
	.string "town. Come see me when you want\l"
	.string "to rebuild more!$"

LittlerootTown_Text_BuilderWhatToRebuild:
	.string "What should I rebuild next, boss?$"

LittlerootTown_Text_BuilderNothingToBuild:
	.string "Nothing else is ready to rebuild\n"
	.string "just yet.\p"
	.string "Keep earning BADGES and exploring,\n"
	.string "and come back to see me!$"

LittlerootTown_Text_BuilderGoodbye:
	.string "Come back anytime, boss!$"

LittlerootTown_Text_BuilderCantAfford:
	.string "That'll cost ¥{STR_VAR_2}.\n"
	.string "Come back when you've saved up!$"

LittlerootTown_Text_BuilderConfirmUpgrade:
	.string "Upgrade the {STR_VAR_1} to\n"
	.string "Lv. {STR_VAR_3} for ¥{STR_VAR_2}?$"

LittlerootTown_Text_BuilderUpgraded:
	.string "Ta-da! The {STR_VAR_1} is\n"
	.string "now Lv. {STR_VAR_3}!\p"
	.string "It has room for another worker,\n"
	.string "and it'll make better goods.\p"
	.string "Add a worker at its terminal!$"

LittlerootTown_Text_BuilderNeedWorker:
	.string "The {STR_VAR_1} needs a\n"
	.string "{STR_VAR_3}-type POKéMON to\l"
	.string "work there. Bring one along first!$"

LittlerootTown_Text_BuilderConfirm:
	.string "Rebuild the {STR_VAR_1}\n"
	.string "for ¥{STR_VAR_2}?$"

LittlerootTown_Text_BuilderStandBack:
	.string "You got it, boss! Stand back…$"

LittlerootTown_Text_BuilderBuilt:
	.string "Ta-da! The {STR_VAR_1} is open\n"
	.string "for business!\p"
	.string "Use its terminal to put a\n"
	.string "{STR_VAR_3}-type POKéMON to work.$"

LittlerootTown_Text_BuilderConfirmMart:
	.string "MR. STONE's buyer needs a shop?\n"
	.string "Build the POKé MART for ¥{STR_VAR_2}?$"

LittlerootTown_Text_BuilderBuiltMart:
	.string "Ta-da! The POKé MART is open!\p"
	.string "DEVON's buyer will take any goods\n"
	.string "you want to sell, right here in\l"
	.string "LITTLEROOT.$"

LittlerootTown_Text_DeliveryServiceSign:
	.string "DELIVERY SERVICE\n"
	.string "Wings for hire, all across HOENN!$"

LittlerootTown_Text_TerminalNeedFirstWorker:
	.string "{STR_VAR_1} TERMINAL\n"
	.string "No worker registered.\p"
	.string "A {STR_VAR_3}-type POKéMON is\n"
	.string "needed to start. Choose a worker.$"

LittlerootTown_Text_TerminalFirstWorkerLater:
	.string "Register a {STR_VAR_3}-type\n"
	.string "POKéMON to start the business.$"

LittlerootTown_Text_TerminalFirstWorkerUnsuited:
	.string "Error: the first worker must be\n"
	.string "a {STR_VAR_3}-type POKéMON.$"

LittlerootTown_Text_TerminalFirstWorkerReady:
	.string "{STR_VAR_1} is registered and\n"
	.string "ready to work!\p"
	.string "The better suited and stronger the\n"
	.string "worker, the more the business makes.\p"
	.string "Money and goods are made as you\n"
	.string "walk. Workers gain EXP, too.\p"
	.string "Come back to this terminal to\n"
	.string "collect them!$"

LittlerootTown_Text_DeliveryTakeWhistle:
	.string "A package is waiting for you!$"

LittlerootTown_Text_DeliveryExplainFlights:
	.string "Blow the BIRD WHISTLE anywhere\n"
	.string "outdoors, and one of our birds will\l"
	.string "fly you right back here.\p"
	.string "This terminal can also book a flight\n"
	.string "to any town you've visited.$"

LittlerootTown_Text_TerminalWorkerStatus:
	.string "{STR_VAR_1} (Lv. {STR_VAR_2}) is working\n"
	.string "at {STR_VAR_3}x speed.$"

LittlerootTown_Text_TerminalTeamStatus:
	.string "{STR_VAR_1} workers on the job at\n"
	.string "this Lv. {STR_VAR_2} business.\p"
	.string "Total speed: {STR_VAR_3}x.$"

LittlerootTown_Text_TerminalNoWorker:
	.string "The {STR_VAR_1} has no worker!\n"
	.string "Choose CHANGE WORKER to add one.$"

LittlerootTown_Text_TerminalStock:
	.string "In stock: {STR_VAR_2} items.\n"
	.string "{STR_VAR_3}$"

LittlerootTown_Text_TerminalWhatToMake:
	.string "What should we make? A single item\n"
	.string "takes a bit longer than the usual mix.$"

LittlerootTown_Text_TerminalOrderSet:
	.string "Got it! Making {STR_VAR_1} from now\n"
	.string "on, until the stockroom is full.$"

LittlerootTown_Text_TerminalOrderUsualMix:
	.string "Got it! Back to the usual mix.$"

LittlerootTown_Text_TerminalOrderNoWorker:
	.string "Nobody's here to fill an order. Put\n"
	.string "a worker on the job first!$"

LittlerootTown_Text_TerminalEarnings:
	.string "Found ¥{STR_VAR_2} in old earnings!\n"
	.string "It was added to your money.$"

LittlerootTown_Text_TerminalCollectedItem:
	.string "Collected {STR_VAR_2} {STR_VAR_1}!$"

LittlerootTown_Text_TerminalNoRoomForItem:
	.string "No room in the BAG for\n"
	.string "{STR_VAR_2} {STR_VAR_1}.\l"
	.string "They'll stay in stock.$"

LittlerootTown_Text_TerminalNothingToCollect:
	.string "Nothing to collect yet. Workers get\n"
	.string "things done while you're out on\l"
	.string "your adventure!$"

LittlerootTown_Text_DeliveryWhereTo:
	.string "Where to? Our birds can fly you to\n"
	.string "any town you've visited.$"

LittlerootTown_Text_TerminalWhoShouldWork:
	.string "Who should work here?$"

LittlerootTown_Text_TerminalNewWorker:
	.string "{STR_VAR_1} is on the job now!\n"
	.string "It works at {STR_VAR_3}x speed.$"

LittlerootTown_Text_TerminalWorkerReturned:
	.string "{STR_VAR_1} returned to your party.$"

LittlerootTown_Text_TerminalNoWorkerToTakeBack:
	.string "There's no worker to take back.$"

LittlerootTown_Text_TerminalPartyFull:
	.string "Your party is full.\n"
	.string "Make some room first!$"

LittlerootTown_Text_TerminalCantUseEgg:
	.string "An EGG can't work here!$"

LittlerootTown_Text_TerminalCantUseLastMon:
	.string "That's your last POKéMON!\n"
	.string "You'll need it on your journey.$"
