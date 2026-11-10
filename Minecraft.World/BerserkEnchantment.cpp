#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.enchantment.h"
#include "BerserkEnchantment.h"
#include "HtmlString.h"

BerserkEnchantment::BerserkEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_torso)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

HtmlString BerserkEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Berserk%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int BerserkEnchantment::getMinCost(int level)
{
	return 15 + (level - 1) * 15;
}

int BerserkEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 30;
}

int BerserkEnchantment::getMaxLevel()
{
	return 3;
}

bool BerserkEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other->id == Enchantment::adrenaline->id) return false;
	return Enchantment::isCompatibleWith(other);
}

float BerserkEnchantment::getDamageBonus(shared_ptr<LivingEntity> attacker, int level)
{
	if (level <= 0) return 0.0f;

	float maxHealth = attacker->getMaxHealth();
	if (maxHealth <= 0) return 0.0f;

	float missingRatio = 1.0f - (attacker->getHealth() / maxHealth); // 0 at full health, 1 near death
	return missingRatio * (2.0f * level); // up to +6 damage at level 3, empty health
}
