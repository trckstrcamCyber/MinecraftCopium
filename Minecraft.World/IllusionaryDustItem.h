#pragma once
#include "Item.h"

// neoLegacy - Illusionary Dust, from the Illager Invasion mod: sprinkle it on yourself to turn invisible for a minute (and quick for ten seconds). Used up when used.
// Illusioners drop it; it is also in Illusioner tower chests.
class IllusionaryDustItem : public Item
{
public:
	static const int INVISIBILITY_TICKS = 1200;
	static const int SPEED_TICKS = 200;

	IllusionaryDustItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
