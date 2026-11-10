#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.enchantment.h"
#include "SonicEnchantment.h"
#include "SharedMonsterAttributes.h"
#include "AttributeInstance.h"
#include "AttributeModifier.h"
#include "HtmlString.h"

SonicEnchantment::SonicEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

HtmlString SonicEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Sonic%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int SonicEnchantment::getMinCost(int level)
{
	return 5 + (level - 1) * 10;
}

int SonicEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 20;
}

int SonicEnchantment::getMaxLevel()
{
	return 3;
}

void SonicEnchantment::tickSpeedModifier(shared_ptr<LivingEntity> entity, int level)
{
	AttributeInstance *speed = entity->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
	if (speed == nullptr) return;

	if (speed->getModifier(eModifierId_ENCHANT_SONIC_SPEED) != nullptr)
	{
		speed->removeModifier(eModifierId_ENCHANT_SONIC_SPEED);
	}

	if (level <= 0) return;

	double amount = 0.08 * level; // +8% per level, multiplicative on top of base speed
	speed->addModifier(new AttributeModifier(eModifierId_ENCHANT_SONIC_SPEED, amount, AttributeModifier::OPERATION_MULTIPLY_TOTAL));
}
