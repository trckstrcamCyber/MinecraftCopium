#pragma once

#include "Enchantment.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - the three Mace enchantments (Java 1.21). All of them only fit a Mace (EnchantmentCategory::mace).
//
//   Density   (I-V)  +0.5 smash damage per block fallen, per level        - read by MaceItem::getSmashBonusDamage
//   Breach    (I-IV) ignores 15% of the target's armor per level (max 60%) - read by LivingEntity::getDamageAfterArmorAbsorb
//   Wind Burst(I-III) a smash launches you back up; the fall hurts less    - read by MaceItem::applySmashToAttacker
//
// Density and Breach cannot be combined with each other or with Sharpness / Smite / Bane of Arthropods (the same exclusive set
// Java uses). Wind Burst combines with everything.
// ---------------------------------------------------------------------------------------------------------------------------

class DensityEnchantment : public Enchantment
{
public:
	DensityEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
};

class BreachEnchantment : public Enchantment
{
public:
	BreachEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;

	// Fraction of the target's armor that is still counted: 1 - 15% per level, never below 40%.
	static float getArmorEffectiveness(int level);
};

class WindBurstEnchantment : public Enchantment
{
public:
	WindBurstEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
};
