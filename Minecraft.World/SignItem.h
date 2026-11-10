#pragma once
using namespace std;

#include "Item.h"

class SignItem : public Item 
{
public:
	// neoLegacy - woodType -1 (default) is THE sign item: the wood is the item's aux value (0 oak, 1 spruce, 2 birch, 3 jungle,
	// 4 acacia, 5 dark oak, 6 bamboo). 0..5 are the old per-wood sign items, kept so old inventories still work.
	SignItem(int id, int woodType = -1);
private:
	int woodType;
	Icon *woodIcons[7];
public:
	virtual Icon *getIcon(int auxValue);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};