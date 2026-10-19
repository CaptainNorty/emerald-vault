@ Emerald Vault: story deliveries. Someone in the story needs a pile of the
@ farm's goods, and takes them in as many trips as it takes.
@ Before calling: VAR_0x8004 and VAR_0x8005 = the items they take (VAR_0x8005
@ can be ITEM_NONE), VAR_0x8006 = how many they need, VAR_0x8007 = how many
@ were delivered so far.
@ Afterwards: VAR_0x8007 = the new total (copy it back), VAR_RESULT = TRUE if
@ anything was handed over, STR_VAR_2 = how many are still needed.
FarmDelivery_EventScript_Offer::
	special Town_CountDeliveryItems
	goto_if_eq VAR_RESULT, 0, FarmDelivery_EventScript_NothingHandedOver
	msgbox FarmDelivery_Text_HandOver, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FarmDelivery_EventScript_NothingHandedOver
	special Town_TakeDeliveryItems
	playse SE_SHOP
	msgbox FarmDelivery_Text_HandedOver, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

FarmDelivery_EventScript_NothingHandedOver::
	setvar VAR_RESULT, FALSE
	return

FarmDelivery_Text_HandOver:
	.string "Hand over {STR_VAR_1} {STR_VAR_3}?$"

FarmDelivery_Text_HandedOver:
	.string "{PLAYER} handed over\n"
	.string "{STR_VAR_1} {STR_VAR_3}.$"
