#pragma once

// neoLegacy - Complex Table
//
// A CraftingMenu for the Complex Table. It differs from the normal menu in two ways:
//   * slotsChanged() matches the crafting grid against ONLY the Complex-Table-only
//     recipes (Recipes::getComplexItemFor) instead of everything else.
//   * stillValid() checks for the Complex Table block instead of the crafting table.
//
// Its "am I a Complex Table?" identity is also what the rest of the game keys off:
// the recipe-book UI (IUIScene_CraftingMenu::CheckRecipesAvailable) and the server's
// craft handler (PlayerConnection::handleCraftItem) use dynamic_cast<ComplexCraftingMenu*>
// / m_bComplexTable to decide which recipes are allowed. The recipes themselves live in
// the shared Recipes list (see Recipes::isComplexOnly) because the recipe book and
// CraftItemPacket address recipes by index into that list.
//
// CraftingMenu::level/x/y/z are private (not protected), so this class keeps its own
// `complexLevel` copy rather than touching the base class.

#include "CraftingMenu.h"

class ComplexCraftingMenu : public CraftingMenu
{
private:
	Level *complexLevel;

public:
	ComplexCraftingMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt);

	virtual void slotsChanged();
	virtual bool stillValid(shared_ptr<Player> player);
};
