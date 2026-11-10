#pragma once

#include "SimpleContainer.h"

class GrindstoneMenu;

// Holds the two grindstone input slots. Mirrors RepairContainer but with no
// naming concept (grindstones don't rename items) and no per-slot item-type
// restriction -- GrindstoneMenu::createResult() figures out whether it's
// looking at a repair-combine, a strip-enchantments, or a book-absorb based
// on whatever the two items actually are.
class GrindstoneContainer : public SimpleContainer, public enable_shared_from_this<GrindstoneContainer>
{
private:
	GrindstoneMenu *m_menu;

public:
	GrindstoneContainer(GrindstoneMenu *menu, int name, bool customName, int size);
	void setChanged();
	bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};
