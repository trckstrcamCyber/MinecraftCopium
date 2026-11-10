#include "stdafx.h"
#include "MaceEnchantments.h"
#include "EnchantmentCategory.h"
#include "DamageEnchantment.h"

// ---- helpers ---------------------------------------------------------------------------------------------------------------
// "Exclusive set" shared by Density, Breach, Sharpness, Smite and Bane of Arthropods.
static bool inMaceDamageSet(Enchantment *e)
{
	if (e == nullptr) return false;
	return e == Enchantment::density || e == Enchantment::breach || dynamic_cast<DamageEnchantment *>(e) != nullptr;
}

// ---- Density ---------------------------------------------------------------------------------------------------------------
DensityEnchantment::DensityEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::mace)
{
	setDescriptionId(IDS_ENCHANTMENT_DENSITY);
}

int DensityEnchantment::getMinCost(int level)
{
	return 5 + (level - 1) * 8;
}

int DensityEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 20;
}

int DensityEnchantment::getMaxLevel()
{
	return 5;
}

bool DensityEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other == this) return false;
	return !inMaceDamageSet(other);
}

// ---- Breach ----------------------------------------------------------------------------------------------------------------
BreachEnchantment::BreachEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::mace)
{
	setDescriptionId(IDS_ENCHANTMENT_BREACH);
}

int BreachEnchantment::getMinCost(int level)
{
	return 15 + (level - 1) * 9;
}

int BreachEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 20;
}

int BreachEnchantment::getMaxLevel()
{
	return 4;
}

bool BreachEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other == this) return false;
	return !inMaceDamageSet(other);
}

float BreachEnchantment::getArmorEffectiveness(int level)
{
	if (level <= 0) return 1.0f;
	float effectiveness = 1.0f - 0.15f * level;
	return effectiveness < 0.4f ? 0.4f : effectiveness;
}

// ---- Wind Burst ------------------------------------------------------------------------------------------------------------
WindBurstEnchantment::WindBurstEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::mace)
{
	setDescriptionId(IDS_ENCHANTMENT_WIND_BURST);
}

int WindBurstEnchantment::getMinCost(int level)
{
	return 15 + (level - 1) * 9;
}

int WindBurstEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 20;
}

int WindBurstEnchantment::getMaxLevel()
{
	return 3;
}
