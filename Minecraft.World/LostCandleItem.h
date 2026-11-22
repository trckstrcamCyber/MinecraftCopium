#pragma once
#include "Item.h"

// neoLegacy - Lost Candle, from the Illager Invasion mod: use it and it tells you whether there is ore within 8 blocks (a chime and the ore's name if there is, a puff
// of smoke if not). Not used up. In the mod it only turns up in labyrinth map chests; here an Archivist now and then carries one.
class LostCandleItem : public Item
{
public:
	static const int RANGE = 8;

	LostCandleItem(int id);

	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly = false);
};
