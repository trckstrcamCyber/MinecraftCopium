#pragma once

#include "FoodItem.h"

class ChorusFruitItem : public FoodItem
{
public:
	ChorusFruitItem(int id, int nutrition, float saturationMod);

	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);

private:
	static bool randomTeleport(Level *level, shared_ptr<Player> player, double x, double y, double z);
};
