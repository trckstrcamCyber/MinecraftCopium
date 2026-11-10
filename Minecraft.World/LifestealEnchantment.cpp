#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.enchantment.h"
#include "LifestealEnchantment.h"
#include "HtmlString.h"

LifestealEnchantment::LifestealEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::weapon)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

HtmlString LifestealEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Lifesteal%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int LifestealEnchantment::getMinCost(int level)
{
	return 10 + (level - 1) * 15;
}

int LifestealEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 30;
}

int LifestealEnchantment::getMaxLevel()
{
	return 3;
}

void LifestealEnchantment::applyLifesteal(shared_ptr<LivingEntity> attacker, float dmgDealt, int level)
{
	if (level <= 0 || dmgDealt <= 0) return;

	float healPercent = 0.10f * level; // 10/20/30%
	attacker->heal(dmgDealt * healPercent);
}


// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Zoro sword combo enchantments (see LifestealEnchantment.h)
// ---------------------------------------------------------------------------------------------------------------------------
#include "WeaponItem.h"
#include "HtmlString.h"

static bool isZoroSwordStack(shared_ptr<ItemInstance> item)
{
	if (item == nullptr || item->getItem() == nullptr) return false;
	WeaponItem *w = dynamic_cast<WeaponItem *>(item->getItem());
	return w != nullptr && w->isZoroSword();
}

CritStackingEnchantment::CritStackingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::zoro_sword)
{
}
int CritStackingEnchantment::getMinCost(int level) { return 12 + (level - 1) * 10; }
int CritStackingEnchantment::getMaxCost(int level) { return getMinCost(level) + 30; }
int CritStackingEnchantment::getMaxLevel() { return 3; }
bool CritStackingEnchantment::canEnchant(shared_ptr<ItemInstance> item) { return isZoroSwordStack(item); }
bool CritStackingEnchantment::isCompatibleWith(Enchantment *other) const
{
	return Enchantment::isCompatibleWith(other) && other != Enchantment::rageStacking;	// Crit Stacking and Rage Stacking exclude each other
}
HtmlString CritStackingEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Crit Stacking%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

RageStackingEnchantment::RageStackingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::zoro_sword)
{
}
int RageStackingEnchantment::getMinCost(int level) { return 12 + (level - 1) * 10; }
int RageStackingEnchantment::getMaxCost(int level) { return getMinCost(level) + 30; }
int RageStackingEnchantment::getMaxLevel() { return 3; }
bool RageStackingEnchantment::canEnchant(shared_ptr<ItemInstance> item) { return isZoroSwordStack(item); }
bool RageStackingEnchantment::isCompatibleWith(Enchantment *other) const
{
	return Enchantment::isCompatibleWith(other) && other != Enchantment::critStacking;
}
HtmlString RageStackingEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Rage Stacking%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}
