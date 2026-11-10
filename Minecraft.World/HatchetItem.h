#pragma once

#include "DiggerItem.h"

#define HATCHET_DIGGABLES 8
class HatchetItem : public DiggerItem
{
private:
	static TileArray *diggables;

public:
	static void staticCtor();
	HatchetItem(int id, const Tier *tier);
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);	// 4J - brought forward from 1.2.3
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);	// neoLegacy - strips logs and Block of Bamboo
};
