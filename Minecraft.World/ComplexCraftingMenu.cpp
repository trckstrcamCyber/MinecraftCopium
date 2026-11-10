#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "CraftingContainer.h"
#include "ResultContainer.h"
#include "ComplexCraftingMenu.h"

ComplexCraftingMenu::ComplexCraftingMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt)
	: CraftingMenu(inventory, level, xt, yt, zt)
{
	complexLevel = level;

	// CraftingMenu's own constructor already called slotsChanged() once, but
	// virtual dispatch during base construction always calls the BASE
	// version (the vtable isn't finalized to this derived class yet), so
	// that first call used the normal recipes. Re-run it now that
	// construction is complete so the override below takes over.
	slotsChanged();
}

void ComplexCraftingMenu::slotsChanged()
{
	resultSlots->setItem(0, Recipes::getInstance()->getComplexItemFor(craftSlots, complexLevel));
}

bool ComplexCraftingMenu::stillValid(shared_ptr<Player> player)
{
	if (complexLevel->getTile(getX(), getY(), getZ()) != Tile::complex_table_Id) return false;
	if (player->distanceToSqr(getX() + 0.5, getY() + 0.5, getZ() + 0.5) > 8 * 8) return false;
	return true;
}
