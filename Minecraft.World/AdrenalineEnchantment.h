#pragma once

#include "Enchantment.h"

class LivingEntity;

// 4J neoLegacy Added
// Chestplate-only armor enchantment (category = armor_torso, same as Thorns).
// Below 50% health, grants scaling movement speed and incoming-damage reduction
// that ramps up the closer the wearer is to death. Incompatible with Berserk -
// the two read the same "how hurt am I" signal in opposite directions and
// stacking them would double-dip the same design space.
class AdrenalineEnchantment : public Enchantment
{
public:
	AdrenalineEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);

	static float getHealthRatio(shared_ptr<LivingEntity> entity);

	// Called every tick (server-side, alive entities only) from LivingEntity::tick(),
	// mirroring how frostWalker/waterWalker are re-evaluated every tick.
	static void tickSpeedModifier(shared_ptr<LivingEntity> entity, int level);

	// Called from LivingEntity::getDamageAfterMagicAbsorb(), alongside the existing
	// EnchantmentHelper::getDamageProtection() call.
	static float getDamageReduction(shared_ptr<LivingEntity> entity, int level, float dmg);
};
