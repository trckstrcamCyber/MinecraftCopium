#pragma once

#include "FoodItem.h"

class Player;
class Level;

// neoLegacy - Copper Age (Phase 6). Rose-gold foods (jar: rosegold mod,
// ModFoodProperties). FoodItem supports exactly ONE eat effect via
// setEatEffect; the rose-gold foods need up to four (stew: Instant
// Health + Strength + Haste + Regeneration), so this subclass carries
// its own effect list (durations in TICKS, as-read from the jar) and
// an eat-chain conversion (full jelly -> half jelly, omelette ->
// partial -> crumbs -> bowl, stew -> bowl; GoldenAppleItem pattern
// for addEatEffect, BowlFoodItem pattern for the conversion).
//
// The conversion is stored as an ITEM ID, not an Item*: at staticCtor
// time the chain targets (half jelly, partial, crumbs) are not
// constructed yet when their parents register, so a pointer would
// capture nullptr. The id resolves at eat time, when all items exist.
class RoseGoldFoodItem : public FoodItem
{
public:
	static const int MAX_EFFECTS = 4;

	struct EatEffect
	{
		int effectId;
		int durationTicks;
		int amplifier;
	};

private:
	EatEffect effects[MAX_EFFECTS];
	int effectCount;
	int convertsToId;
	int useDurationTicks;

public:
	RoseGoldFoodItem(int id, int nutrition, float saturationMod);

	RoseGoldFoodItem *addEatEffectEntry(int effectId, int durationTicks, int amplifier);
	RoseGoldFoodItem *setConvertsTo(int itemId);
	RoseGoldFoodItem *setUseDurationTicks(int ticks);

	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player) override;

protected:
	virtual void addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player) override;

public:
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance) override;
};
