#pragma once

#include "Enchantment.h"

class LivingEntity;

// 4J neoLegacy Added
// Boots-only armor enchantment (category = armor_feet, same restriction
// mechanism Protection uses for its FALL type). Flat movement speed boost,
// unconditional (unlike Adrenaline, doesn't depend on health).
class SonicEnchantment : public Enchantment
{
public:
	SonicEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual HtmlString getFullname(int level);

	// Called every tick from LivingEntity::tick(), same call site as
	// AdrenalineEnchantment::tickSpeedModifier().
	static void tickSpeedModifier(shared_ptr<LivingEntity> entity, int level);
};
