#pragma once

#include "Enchantment.h"

class LivingEntity;

// 4J neoLegacy Added
// Weapon enchantment. On a successful melee hit, heals the attacker a
// percentage of the damage just dealt.
class LifestealEnchantment : public Enchantment
{
public:
	LifestealEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual HtmlString getFullname(int level);

	// Called from Player::attack(), inside the existing if (wasHurt) block,
	// right alongside the ThornsEnchantment::doThornsAfterAttack() call.
	static void applyLifesteal(shared_ptr<LivingEntity> attacker, float dmgDealt, int level);
};


// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Zoro sword combo enchantments. Both only fit the four Zoro swords (EnchantmentCategory::zoro_sword), are found at
// the enchanting table, and cannot be combined with each other. The streak logic lives in Player::attack().
// ---------------------------------------------------------------------------------------------------------------------------
class CritStackingEnchantment : public Enchantment
{
public:
	CritStackingEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);

	// Every consecutive hit of a combo adds this much critical-hit chance (5 / 10 / 15 %); a crit resets it to 0.
	static float chancePerHit(int level) { return 0.05f * level; }
};

class RageStackingEnchantment : public Enchantment
{
public:
	RageStackingEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);

	// Every consecutive hit of a combo makes the NEXT hit this much stronger (5 / 10 / 15 %), capped at +200 %.
	static float bonusPerHit(int level) { return 0.05f * level; }
	static constexpr float MAX_BONUS = 2.0f;
};
