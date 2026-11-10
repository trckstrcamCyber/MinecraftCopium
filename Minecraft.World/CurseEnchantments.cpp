#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.enchantment.h"
#include "ItemInstance.h"
#include "CurseEnchantments.h"
#include "HtmlString.h"

// ---- Hook of Muddy Waters ----------------------------------------------------------------------------------------------

HookOfMuddyWatersEnchantment::HookOfMuddyWatersEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::fishing_rod)
{
	// the name comes from getFullname() below, like the other neoLegacy enchantments
}

int HookOfMuddyWatersEnchantment::getMinCost(int level) { return 9 * level + 6; }
int HookOfMuddyWatersEnchantment::getMaxCost(int level) { return Enchantment::getMinCost(level) + 50; }
int HookOfMuddyWatersEnchantment::getMaxLevel() { return 3; }

bool HookOfMuddyWatersEnchantment::isCompatibleWith(Enchantment *other) const
{
	// it is the exact opposite of Luck of the Sea, so having both would just cancel out into a plain rod
	return Enchantment::isCompatibleWith(other) && other != Enchantment::luckOfTheSea;
}

HtmlString HookOfMuddyWatersEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Curse of Muddy Waters%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_c);
}

// ---- Curse of Binding --------------------------------------------------------------------------------------------------

BindingCurseEnchantment::BindingCurseEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::all)
{
}

int BindingCurseEnchantment::getMinCost(int level) { return 25; }
int BindingCurseEnchantment::getMaxCost(int level) { return 50; }
int BindingCurseEnchantment::getMaxLevel() { return 1; }

bool BindingCurseEnchantment::isWearable(shared_ptr<ItemInstance> item)
{
	if (item == nullptr) return false;
	Item *i = item->getItem();
	if (i == nullptr) return false;
	if (dynamic_cast<ArmorItem *>(i) != nullptr) return true;
	return i->id == Tile::pumpkin_Id || i->id == Item::skull_Id || i->id == Item::elytra_Id;
}

bool BindingCurseEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	return isWearable(item);
}

HtmlString BindingCurseEnchantment::getFullname(int level)
{
	return HtmlString(L"Curse of Binding", eHTMLColor_c);
}

// ---- Curse of Vanishing ------------------------------------------------------------------------------------------------

VanishingCurseEnchantment::VanishingCurseEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::all)
{
}

int VanishingCurseEnchantment::getMinCost(int level) { return 25; }
int VanishingCurseEnchantment::getMaxCost(int level) { return 50; }
int VanishingCurseEnchantment::getMaxLevel() { return 1; }

bool VanishingCurseEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	if (item == nullptr) return false;
	return item->isDamageableItem() || BindingCurseEnchantment::isWearable(item) || item->id == Item::compass_Id;
}

HtmlString VanishingCurseEnchantment::getFullname(int level)
{
	return HtmlString(L"Curse of Vanishing", eHTMLColor_c);
}
