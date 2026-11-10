#pragma once

#include "Item.h"

class Player;
class Level;

// neoLegacy - Fancy Beds. ONE item id for every bed. The aux value is the variant (see BedTileEntity.h):
// 0-15 = the 16 dye colors in wool order, 16 = straw bed.
class BedItem : public Item
{
public:
	static const int ICON_COUNT = 17;
	static const wchar_t *const VARIANT_NAMES[ICON_COUNT];			// "white" .. "black", "straw"; used for icon names ("bed_white")
	static const unsigned int VARIANT_DESCS[ICON_COUNT];			// string ids for the item names

private:
	Icon **icons;

public:
	BedItem(int id);

	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);

	virtual Icon *getIcon(int auxValue);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);

	// Colored beds stack to 1 like in Java; straw beds stack to 16.
	using Item::getMaxStackSize;
	virtual int getMaxStackSize(const ItemInstance *instance) const;
};
