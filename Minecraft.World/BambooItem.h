#pragma once
#include "Item.h"

// neoLegacy - the Bamboo item. On soil it plants a Bamboo shoot; on top of a bamboo plant (or shoot) it extends the stalk.
class BambooItem : public Item
{
public:
	BambooItem(int id);
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};
