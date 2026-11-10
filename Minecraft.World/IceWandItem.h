#pragma once
#include "Item.h"

// neoLegacy - Ice Wand (from the FrozenFiend mod): right-click a creature to conjure an Ice Cube above it. 64 uses, 20 second cooldown.
class IceWandItem : public Item
{
public:
	IceWandItem(int id);

	virtual bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob);
	virtual bool isHandEquipped() { return true; }

private:
	static const int COOLDOWN_TICKS = 400;
};
