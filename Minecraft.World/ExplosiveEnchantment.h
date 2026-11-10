#pragma once

#include "Enchantment.h"

class Entity;
class Level;

// 4J neoLegacy Added
// Bow enchantment. Marks fired arrows as "explosive" (mirrors how ArrowFireEnchantment
// is just a marker checked in BowItem::releaseUsing / applied to the Arrow entity).
// The actual concussion burst is triggered from Arrow::tick() on impact via
// triggerExplosion() below. destroyBlocks is always false - this is a concussive/
// anti-mob burst, not a TNT replacement.
class ExplosiveEnchantment : public Enchantment
{
public:
	ExplosiveEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual HtmlString getFullname(int level);

	static void triggerExplosion(Level *level, shared_ptr<Entity> owner, double x, double y, double z, int enchantLevel);
};
