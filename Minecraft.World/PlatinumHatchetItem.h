#pragma once
#include "Item.h"

// neoLegacy - Platinum Infused Hatchet, from the Illager Invasion mod: a throwing axe. Held like a bow: draw it for at least half a second, then release to throw it
// (8 damage). It bounces off whatever it hits and drops where it lands, so it can be picked up again; each throw costs one point of durability and it will not be thrown
// when that would break it. In the hand it hits for 7 (6 + the player's 1). 327 durability, repaired with Platinum Sheets.
// The thrown hatchet is a flagged Snowball (see Snowball::setHatchet), which carries the real stack so that the pickup keeps its damage and enchantments.
class PlatinumHatchetItem : public Item
{
public:
	static constexpr int DURABILITY = 327;
	static constexpr float ATTACK_DAMAGE = 6.0f;
	static const int MIN_DRAW_TICKS = 10;

	PlatinumHatchetItem(int id);

	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual bool isHandEquipped();
	virtual int getEnchantmentValue();
	virtual bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	virtual attrAttrModMap *getDefaultAttributeModifiers();

	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft);
};
