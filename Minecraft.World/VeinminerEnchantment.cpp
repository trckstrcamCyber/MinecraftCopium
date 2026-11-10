#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.tile.h"
#include "VeinminerEnchantment.h"
#include "PickaxeItem.h"
#include "HtmlString.h"

VeinminerEnchantment::VeinminerEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::digger)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

HtmlString VeinminerEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Veinminer%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int VeinminerEnchantment::getMinCost(int level)
{
	return 15 + (level - 1) * 15;
}

int VeinminerEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 30;
}

int VeinminerEnchantment::getMaxLevel()
{
	return 3;
}

bool VeinminerEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	// Pickaxe only - deliberately narrower than DiggingEnchantment, which also
	// allows shovels/axes/shears. Chain-breaking dirt or leaves isn't the goal here.
	return dynamic_cast<PickaxeItem *>(item->getItem()) != nullptr;
}

bool VeinminerEnchantment::isVeinmineable(int tileId)
{
	switch (tileId)
	{
	case Tile::coal_ore_Id:
	case Tile::iron_ore_Id:
	case Tile::gold_ore_Id:
	case Tile::lapis_ore_Id:
	case Tile::diamond_ore_Id:
	case Tile::redstone_ore_Id:
	case Tile::lit_redstone_ore_Id:
	case Tile::emerald_ore_Id:
	case Tile::quartz_ore_Id:
		return true;
	default:
		// VERIFY: add neoLegacy ore ids here too (e.g. any new ore tiles you add later).
		return false;
	}
}

int VeinminerEnchantment::getMaxChainSize(int level)
{
	if (level <= 1) return 8;
	if (level == 2) return 16;
	return 32;
}
