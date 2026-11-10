#pragma once
#include "Item.h"

// neoLegacy - Crossbow. Hold right-click for 1.25 s to draw it (uses one arrow), release: it is now LOADED. Right-click again to fire
// (speed 3.15, always critical). The loaded state is a second item id (Item::crossbow_loaded, shown with the arrow icon and kept out of
// the creative menu) that shares the first one's damage, so no per-stack icon logic is needed. Not implemented yet: Quick Charge,
// Multishot, Piercing, firework rockets as ammunition, the intermediate "pulling" icons.
class CrossbowItem : public Item
{
private:
	bool loaded;
	Icon *pullingIcons[3];
public:
	static const int CHARGE_TICKS = 25;

	CrossbowItem(int id, bool loaded);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual void registerIcons(IconRegister *iconRegister);
	Icon *getDrawnIcon(int amount);	// 0..2: how far the string is drawn back (used by Player::getItemInHandIcon, like the bow)
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
};
