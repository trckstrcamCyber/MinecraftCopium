#pragma once
#include "Item.h"

// neoLegacy - Goat Horn (item 619): blow it to sound one of the eight horn calls
// (item.goat_horn.call -> call1..8.ogg, picked at random by the sound engine).
// Seven seconds between blasts (kept on the horn itself, like the Horn of Sight);
// the horn is not used up. The calls are the Java goat-horn calls (call0-7,
// renumbered 1-8 for the engine's numbered-variant scheme). Max stack 1, like Java.
class GoatHornItem : public Item
{
public:
	static const int COOLDOWN_TICKS = 140;

	GoatHornItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
