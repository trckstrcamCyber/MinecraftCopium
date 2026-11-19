#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "DataOutputStream.h"
#include "ItemInstance.h"
#include "BannerItem.h"
#include "BannerPatterns.h"
#include "BannerPatternRecipe.h"

BannerPatternRecipe::BannerPatternRecipe()
{
	sample = std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15);
}

bool BannerPatternRecipe::matches(shared_ptr<CraftingContainer> craftSlots, Level *level)
{
	return BannerPatterns::match(craftSlots, nullptr) >= 0;
}

shared_ptr<ItemInstance> BannerPatternRecipe::assemble(shared_ptr<CraftingContainer> craftSlots)
{
	int color = 0;
	int pattern = BannerPatterns::match(craftSlots, &color);
	if (pattern < 0) return nullptr;

	for (unsigned int i = 0; i < craftSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> s = craftSlots->getItem(i);
		if (s != nullptr && dynamic_cast<BannerItem *>(s->getItem()) != nullptr)
		{
			shared_ptr<ItemInstance> result = s->copy();
			result->count = 1;
			BannerItem::addPattern(result, BannerPatterns::get(pattern)->code, color);
			return result;
		}
	}
	return nullptr;
}

void BannerPatternRecipe::writeToStream(DataOutputStream *dos)
{
	dos->writeByte(99);
}
