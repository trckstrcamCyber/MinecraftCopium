#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.effect.h"
#include "ItemInstance.h"
#include "RoseGoldFoodItem.h"

RoseGoldFoodItem::RoseGoldFoodItem(int id, int nutrition, float saturationMod)
	: FoodItem(id, nutrition, saturationMod, false), effectCount(0), convertsToId(-1), useDurationTicks(EAT_DURATION)
{
}

RoseGoldFoodItem *RoseGoldFoodItem::addEatEffectEntry(int effectId, int durationTicks, int amplifier)
{
	if (effectCount < MAX_EFFECTS)
	{
		effects[effectCount].effectId = effectId;
		effects[effectCount].durationTicks = durationTicks;
		effects[effectCount].amplifier = amplifier;
		effectCount++;
	}
	return this;
}

RoseGoldFoodItem *RoseGoldFoodItem::setConvertsTo(int itemId)
{
	convertsToId = itemId;
	return this;
}

RoseGoldFoodItem *RoseGoldFoodItem::setUseDurationTicks(int ticks)
{
	useDurationTicks = ticks;
	return this;
}

void RoseGoldFoodItem::addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (level->isClientSide) return;
	for (int i = 0; i < effectCount; i++)
	{
		player->addEffect(new MobEffectInstance(effects[i].effectId, effects[i].durationTicks, effects[i].amplifier));
	}
}

shared_ptr<ItemInstance> RoseGoldFoodItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> result = FoodItem::useTimeDepleted(instance, level, player);
	// FoodItem::useTimeDepleted already decremented the stack and ran
	// addEatEffect (virtual dispatch reaches this class). A stacksTo(1)
	// chain item is now at count 0: hand the player its conversion.
	// Stackable foods (beetroot / mushroom / cookie) have no convertsTo
	// and keep whatever is left of the stack.
	if (convertsToId >= 0 && result->count <= 0)
	{
		return std::make_shared<ItemInstance>(convertsToId, 1, 0);
	}
	return result;
}

int RoseGoldFoodItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return useDurationTicks;
}
