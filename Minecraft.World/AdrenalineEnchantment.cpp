#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.enchantment.h"
#include "AdrenalineEnchantment.h"
#include "SharedMonsterAttributes.h"
#include "AttributeInstance.h"
#include "AttributeModifier.h"
#include "HtmlString.h"

AdrenalineEnchantment::AdrenalineEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_torso)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId, same approach your ClimbEnchantment already uses -
	// avoids borrowing another enchantment's string and showing the wrong
	// name in-game.
}

HtmlString AdrenalineEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Adrenaline%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int AdrenalineEnchantment::getMinCost(int level)
{
	return 15 + (level - 1) * 15;
}

int AdrenalineEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 30;
}

int AdrenalineEnchantment::getMaxLevel()
{
	return 3;
}

bool AdrenalineEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other->id == Enchantment::berserk->id) return false;
	return Enchantment::isCompatibleWith(other);
}

float AdrenalineEnchantment::getHealthRatio(shared_ptr<LivingEntity> entity)
{
	float maxHealth = entity->getMaxHealth();
	if (maxHealth <= 0) return 1.0f;
	return entity->getHealth() / maxHealth;
}

void AdrenalineEnchantment::tickSpeedModifier(shared_ptr<LivingEntity> entity, int level)
{
	AttributeInstance *speed = entity->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
	if (speed == nullptr) return;

	if (speed->getModifier(eModifierId_ENCHANT_ADRENALINE_SPEED) != nullptr)
	{
		speed->removeModifier(eModifierId_ENCHANT_ADRENALINE_SPEED);
	}

	if (level <= 0) return;

	float ratio = getHealthRatio(entity);
	if (ratio >= 0.5f) return; // only kicks in below half health

	float missingBelowHalf = (0.5f - ratio) * 2.0f; // 0..1 as health falls from 50% to 0%
	double amount = missingBelowHalf * (0.10 * level); // up to +30% speed at level 3, near death

	speed->addModifier(new AttributeModifier(eModifierId_ENCHANT_ADRENALINE_SPEED, amount, AttributeModifier::OPERATION_MULTIPLY_TOTAL));
}

float AdrenalineEnchantment::getDamageReduction(shared_ptr<LivingEntity> entity, int level, float dmg)
{
	if (level <= 0) return dmg;

	float ratio = getHealthRatio(entity);
	if (ratio >= 0.5f) return dmg;

	float missingBelowHalf = (0.5f - ratio) * 2.0f;
	float reduction = missingBelowHalf * (0.12f * level); // up to ~36% less damage at level 3, near death

	return dmg * (1.0f - reduction);
}
