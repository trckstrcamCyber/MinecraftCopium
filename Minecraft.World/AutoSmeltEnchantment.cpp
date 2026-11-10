#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "AutoSmeltEnchantment.h"
#include "PickaxeItem.h"
#include "HtmlString.h"

AutoSmeltEnchantment::AutoSmeltEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::digger)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

int AutoSmeltEnchantment::getMinCost(int level)
{
	return 15;
}

int AutoSmeltEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 50;
}

int AutoSmeltEnchantment::getMaxLevel()
{
	// Single level, same as Silk Touch - it's a binary effect, not something
	// that scales.
	return 1;
}

bool AutoSmeltEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	// NOTE: this override is only consulted by the anvil/book/`/enchant`
	// command paths (AnvilMenu::canEnchant / RepairMenu / EnchantItemCommand),
	// NOT by the enchant table's actual roll - that calls
	// `category->canEnchant(item)` directly (see
	// EnchantmentHelper::getAvailableEnchantmentResults), which only checks
	// "is this a DiggerItem" for the digger category and can't distinguish
	// pickaxe from shovel/axe/hoe. There's no dedicated "pickaxe-only"
	// EnchantmentCategory in this codebase, so the enchant table itself can
	// still offer Auto Smelt on a shovel/axe/hoe - it'll just never do
	// anything there, since iron/gold ore already requires a pickaxe to
	// harvest at all (Tile::canDestroy gates on tool class before
	// playerDestroy(), where Auto Smelt is checked, ever runs).
	return dynamic_cast<PickaxeItem *>(item->getItem()) != nullptr;
}

bool AutoSmeltEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other->id == Enchantment::untouching->id) return false;
	return Enchantment::isCompatibleWith(other);
}

HtmlString AutoSmeltEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Auto Smelt%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}
