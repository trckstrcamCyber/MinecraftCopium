#pragma once

#include "Enchantment.h"

class LivingEntity;

// 4J neoLegacy Added
// Chestplate-only armor enchantment. Damage dealt scales up as the wearer's own
// health drops - a glass-cannon enchant. Incompatible with Adrenaline (see
// AdrenalineEnchantment.h for why).
class BerserkEnchantment : public Enchantment
{
public:
	BerserkEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);

	// Called from Player::attack() alongside the existing
	// EnchantmentHelper::getDamageBonus(thisPlayer, mob) call. Deliberately NOT
	// wired through Enchantment::getDamageBonus(level, target), since that
	// signature only receives the target, and Berserk needs the *attacker's*
	// health - EnchantmentHelper::getDamageBonus() also only ever scans the
	// carried item, not armor, so it wouldn't see a chestplate enchant anyway.
	static float getDamageBonus(shared_ptr<LivingEntity> attacker, int level);
};
