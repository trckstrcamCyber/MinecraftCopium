#pragma once
using namespace std;

#include "Item.h"

class SnowballItem : public Item
{
private:
	bool ice;	// neoLegacy - this item throws Ice Balls
public:
	SnowballItem(int id, bool ice = false);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
};