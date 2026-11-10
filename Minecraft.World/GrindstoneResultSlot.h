#pragma once

#include "Slot.h"

class GrindstoneMenu;

class GrindstoneResultSlot : public Slot
{
private:
	GrindstoneMenu *m_menu;
	int xt, yt, zt;

public:
	GrindstoneResultSlot(GrindstoneMenu *menu, int xt, int yt, int zt, shared_ptr<Container> container, int slot, int x, int y);

	bool mayPlace(shared_ptr<ItemInstance> item);
	bool mayPickup(shared_ptr<Player> player);
	void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(shared_ptr<ItemInstance> item);
};
