PetalburgCity_WallysHouse_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PetalburgCity_WallysHouse_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PetalburgCity_WallysHouse_OnWarp
	.byte 0

PetalburgCity_WallysHouse_OnWarp:
	map_script_2 VAR_PETALBURG_CITY_STATE, 4, PetalburgCity_WallysHouse_EventScript_PlayerWallysDadFaceEachOther
	.2byte 0

PetalburgCity_WallysHouse_EventScript_PlayerWallysDadFaceEachOther::
	turnobject LOCALID_PLAYER, DIR_EAST
	turnobject LOCALID_WALLYS_HOUSE_WALLYS_DAD, DIR_WEST
	end

PetalburgCity_WallysHouse_OnFrame:
	map_script_2 VAR_PETALBURG_CITY_STATE, 4, PetalburgCity_WallysHouse_EventScript_GiveHMSurf
	.2byte 0

PetalburgCity_WallysHouse_EventScript_GiveHMSurf::
	lockall
	msgbox PetalburgCity_WallysHouse_Text_PleaseExcuseUs, MSGBOX_DEFAULT
	@ Emerald Vault: Surf is his thanks for berries to keep Wally healthy.
	msgbox PetalburgCity_WallysHouse_Text_BerriesForWally, MSGBOX_DEFAULT
	setvar VAR_PETALBURG_CITY_STATE, 5
	call PetalburgCity_WallysHouse_EventScript_CollectBerries
	releaseall
	end

@ Takes Sitrus and Lum Berries for Wally, and gives HM Surf once he has them all.
PetalburgCity_WallysHouse_EventScript_CollectBerries::
	copyvar VAR_0x8007, VAR_WALLY_BERRIES_DELIVERED
	setvar VAR_0x8004, ITEM_SITRUS_BERRY
	setvar VAR_0x8005, ITEM_LUM_BERRY
	setvar VAR_0x8006, WALLY_BERRIES_NEEDED
	call FarmDelivery_EventScript_Offer
	copyvar VAR_WALLY_BERRIES_DELIVERED, VAR_0x8007
	goto_if_ge VAR_WALLY_BERRIES_DELIVERED, WALLY_BERRIES_NEEDED, PetalburgCity_WallysHouse_EventScript_BerriesComplete
	goto_if_eq VAR_RESULT, TRUE, PetalburgCity_WallysHouse_EventScript_BerriesPartway
	msgbox PetalburgCity_WallysHouse_Text_BringBerries, MSGBOX_DEFAULT
	return

PetalburgCity_WallysHouse_EventScript_BerriesPartway::
	msgbox PetalburgCity_WallysHouse_Text_BerriesPartway, MSGBOX_DEFAULT
	return

PetalburgCity_WallysHouse_EventScript_BerriesComplete::
	msgbox PetalburgCity_WallysHouse_Text_BerriesComplete, MSGBOX_DEFAULT
	giveitem ITEM_HM_SURF
	setflag FLAG_RECEIVED_HM_SURF
	msgbox PetalburgCity_WallysHouse_Text_SurfGoAllSortsOfPlaces, MSGBOX_DEFAULT
	return

PetalburgCity_WallysHouse_EventScript_AskForBerries::
	call PetalburgCity_WallysHouse_EventScript_CollectBerries
	release
	end

PetalburgCity_WallysHouse_EventScript_WallysDad::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_WALLY_VICTORY_ROAD, PetalburgCity_WallysHouse_EventScript_DefeatedWallyInVictoryRoad
	goto_if_set FLAG_RECEIVED_HM_SURF, PetalburgCity_WallysHouse_EventScript_ReceievedHMSurf
	goto_if_ge VAR_PETALBURG_CITY_STATE, 5, PetalburgCity_WallysHouse_EventScript_AskForBerries
	goto_if_set FLAG_THANKED_FOR_PLAYING_WITH_WALLY, PetalburgCity_WallysHouse_EventScript_PlayedWithWally
	msgbox PetalburgCity_WallysHouse_Text_ThanksForPlayingWithWally, MSGBOX_DEFAULT
	setflag FLAG_THANKED_FOR_PLAYING_WITH_WALLY
	release
	end

PetalburgCity_WallysHouse_EventScript_ReceievedHMSurf::
	msgbox PetalburgCity_WallysHouse_Text_WallyIsComingHomeSoon, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_DefeatedWallyInVictoryRoad::
	msgbox PetalburgCity_WallysHouse_Text_YouMetWallyInEverGrandeCity, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_PlayedWithWally::
	msgbox PetalburgCity_WallysHouse_Text_WonderHowWallyIsDoing, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_WallysMom::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_HM_SURF, PetalburgCity_WallysHouse_EventScript_ReceivedHMSurf
	goto_if_ge VAR_PETALBURG_CITY_STATE, 5, PetalburgCity_WallysHouse_EventScript_MomBerries
	msgbox PetalburgCity_WallysHouse_Text_WallyWasReallyHappy, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_MomBerries::
	msgbox PetalburgCity_WallysHouse_Text_MomBerries, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_EventScript_ReceivedHMSurf::
	msgbox PetalburgCity_WallysHouse_Text_WallyLeftWithoutTelling, MSGBOX_DEFAULT
	release
	end

PetalburgCity_WallysHouse_Text_ThanksForPlayingWithWally:
	.string "You're…\n"
	.string "Ah, you must be {PLAYER}{KUN}, right?\p"
	.string "Thank you for playing with WALLY a\n"
	.string "little while ago.\p"
	.string "He's been frail and sickly ever\n"
	.string "since he was a baby.\p"
	.string "We've sent him to stay with my relatives\n"
	.string "in VERDANTURF TOWN for a while.\p"
	.string "The air is a lot cleaner there\n"
	.string "than it is here.\p"
	.string "What's that? Where's WALLY?\n"
	.string "He's already left, our WALLY.\p"
	.string "I wonder where he could have\n"
	.string "gotten by now?$"

PetalburgCity_WallysHouse_Text_WonderHowWallyIsDoing:
	.string "I wonder how our WALLY is doing?$"

PetalburgCity_WallysHouse_Text_PleaseExcuseUs:
	.string "{PLAYER}{KUN}! Please excuse us for\n"
	.string "dragging you here this way.\p"
	.string "But our WALLY's become very healthy\n"
	.string "since he went to VERDANTURF TOWN.\p"
	.string "We owe it all to you!\p"
	.string "When WALLY left town, you helped\n"
	.string "him catch a POKéMON, right?\p"
	.string "I think that made WALLY really\n"
	.string "happy.\p"
	.string "Actually, not just WALLY.\n"
	.string "It made me, his father, happy too.\p"
	.string "Happy that he's gained such a great\n"
	.string "friend as you.$"

PetalburgCity_WallysHouse_Text_BerriesForWally:
	.string "But I have one more favor to ask.\p"
	.string "WALLY's doctor says SITRUS and LUM\n"
	.string "BERRIES keep him strong.\p"
	.string "Nobody grows them anymore, and his\n"
	.string "supply in VERDANTURF is running out.\p"
	.string "I hear the GREENHOUSE on your farm\n"
	.string "grows them, once it's been upgraded.\p"
	.string "Could you bring me 30 of them?\n"
	.string "SITRUS or LUM, any mix will do.\p"
	.string "I'll send them on to WALLY. And I have\n"
	.string "something to thank you with, too.$"

PetalburgCity_WallysHouse_Text_BringBerries:
	.string "WALLY still needs {STR_VAR_2} more\n"
	.string "SITRUS or LUM BERRIES.\p"
	.string "The GREENHOUSE in LITTLEROOT grows\n"
	.string "them once it's upgraded to LV. 2.$"

PetalburgCity_WallysHouse_Text_BerriesPartway:
	.string "Thank you! Just {STR_VAR_2} more,\n"
	.string "and WALLY's set for the season.$"

PetalburgCity_WallysHouse_Text_BerriesComplete:
	.string "That's all 30! I'll send them to\n"
	.string "VERDANTURF right away.\p"
	.string "This isn't a bribe or anything, but\n"
	.string "I'd really like you to have this.$"

PetalburgCity_WallysHouse_Text_MomBerries:
	.string "Those berries will do WALLY a world\n"
	.string "of good. Thank you for your help.$"

PetalburgCity_WallysHouse_Text_SurfGoAllSortsOfPlaces:
	.string "If your POKéMON can SURF, you'll be\n"
	.string "able to go to all sorts of places.$"

PetalburgCity_WallysHouse_Text_WallyIsComingHomeSoon:
	.string "WALLY's coming home soon.\n"
	.string "I'm looking forward to that.$"

PetalburgCity_WallysHouse_Text_YouMetWallyInEverGrandeCity:
	.string "Oh? You met WALLY in\n"
	.string "EVER GRANDE CITY?\p"
	.string "Oh, {PLAYER}{KUN}, don't be silly.\p"
	.string "He may have gotten healthy, but he\n"
	.string "can't go somewhere far away like\l"
	.string "that all by himself.$"

PetalburgCity_WallysHouse_Text_WallyWasReallyHappy:
	.string "WALLY was really happy when he told\n"
	.string "us that he caught a POKéMON.\p"
	.string "It's been ages since I've seen him\n"
	.string "smile like that.$"

PetalburgCity_WallysHouse_Text_WallyLeftWithoutTelling:
	.string "I want you to keep this a secret\n"
	.string "from my husband…\p"
	.string "But our WALLY left VERDANTURF TOWN\n"
	.string "without telling anyone.\p"
	.string "You know, WALLY is frail, but\n"
	.string "he's surprisingly strong-willed.\p"
	.string "I'm sure that he'll come back safe\n"
	.string "and sound one day!$"

