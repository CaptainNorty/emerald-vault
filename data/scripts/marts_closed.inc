@ Emerald Vault: every shop's shelves are empty. Goods come from the farm in
@ Littleroot instead. The pokemart script command jumps here (src/scrcmd.c).
EventScript_MartClosed::
	msgbox Text_MartClosed, MSGBOX_DEFAULT
	release
	end

Text_MartClosed:
	.string "…Oh, I'm sorry. Our shelves are\n"
	.string "empty.\p"
	.string "No supplies have come in for ages.\n"
	.string "Nobody makes anything anymore…\p"
	.string "Well, I did hear there's a farm\n"
	.string "down in LITTLEROOT TOWN that's\l"
	.string "started making goods again.$"
