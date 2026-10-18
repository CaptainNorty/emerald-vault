@ Emerald Vault: DEVON's buyer, set up by Mr. Stone once the Devon Goods are
@ returned. Nothing for sale; buys the player's goods.
LittlerootTown_Mart_MapScripts::
	.byte 0

LittlerootTown_Mart_EventScript_Clerk::
	lock
	faceplayer
	message LittlerootTown_Mart_Text_Welcome
	waitmessage
	special CreateSellOnlyMartMenu
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

LittlerootTown_Mart_Text_Welcome:
	.string "Welcome! DEVON sent me to buy goods\n"
	.string "from the farm. What can I take off\l"
	.string "your hands?$"
