#pragma once

#include "Recipy.h"

// neoLegacy - adds a pattern to a banner: a banner and dyes in a pattern in the 3x3 crafting grid (see BannerPatterns). Like the firework recipe
// it is not in the normal recipe list (the console crafting menu cannot show free-form recipes); Recipes::getItemFor asks it directly, so it
// works wherever there is a free 3x3 grid (the crafting table and the Classic Crafting menu).
class BannerPatternRecipe : public Recipy
{
private:
	shared_ptr<ItemInstance> sample;

public:
	BannerPatternRecipe();

	bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);
	shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);
	int size() { return 10; }
	const ItemInstance *getResultItem() { return sample.get(); }
	virtual const int getGroup() { return 0; }

	virtual bool reqs(int iRecipe) { return false; }
	virtual void reqs(INGREDIENTS_REQUIRED *pIngReq) {}

	virtual void writeToStream(DataOutputStream *dos);
};
