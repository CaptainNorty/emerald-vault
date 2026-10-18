@ Emerald Vault: every shop's shelves are empty, since goods come from the farm
@ in Littleroot, but the clerks still buy. The pokemart script command jumps
@ here (src/scrcmd.c) after the clerk's greeting.
EventScript_MartClosed::
	msgbox Text_MartClosed, MSGBOX_DEFAULT
	message Text_MartAnythingToSell
	waitmessage
	special CreateSellOnlyMartMenu
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Text_MartClosed:
	.string "…Oh, I'm sorry. Our shelves are\n"
	.string "empty.\p"
	.string "No supplies have come in for ages.\n"
	.string "Nobody makes anything anymore…\p"
	.string "I did hear there's a farm down in\n"
	.string "LITTLEROOT TOWN that's started\l"
	.string "making goods again.\p"
	.string "And we're still buying! If you have\n"
	.string "goods to spare, we'll pay for them.$"

Text_MartAnythingToSell:
	.string "Do you have anything to sell?$"
