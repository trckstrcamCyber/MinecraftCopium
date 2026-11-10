#include "stdafx.h"
#include "Random.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "IconRegister.h"
#include "CrossbowItem.h"

CrossbowItem::CrossbowItem(int id, bool loaded) : Item(id)
{
	this->loaded = loaded;
	maxStackSize = 1;
	pullingIcons[0] = pullingIcons[1] = pullingIcons[2] = nullptr;
}

void CrossbowItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);
	pullingIcons[0] = iconRegister->registerIcon(L"crossbow_pulling_0");
	pullingIcons[1] = iconRegister->registerIcon(L"crossbow_pulling_1");
	pullingIcons[2] = iconRegister->registerIcon(L"crossbow_pulling_2");
}

Icon *CrossbowItem::getDrawnIcon(int amount)
{
	return pullingIcons[amount];
}

int CrossbowItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60;
}

UseAnim CrossbowItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_none;	// not UseAnim_bow: the bow's "pull back" arm/hand transforms make a crossbow look wrong
}

shared_ptr<ItemInstance> CrossbowItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	return instance;
}

shared_ptr<ItemInstance> CrossbowItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (loaded)
	{
		// fire: always a critical arrow at 3.15 speed (power 2.1 in this engine's units), then the crossbow is empty again
		if (!level->isClientSide)
		{
			shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, player, 2.1f);
			arrow->setCritArrow(true);
			if (player->abilities.instabuild) arrow->pickup = Arrow::PICKUP_CREATIVE_ONLY;
			level->addEntity(arrow);
		}
		level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_SHOOT, 1.0f, 1.0f / (player->getRandom()->nextFloat() * 0.5f + 1.8f) + 0.45f);

		shared_ptr<ItemInstance> empty = std::make_shared<ItemInstance>(Item::crossbow, 1, instance->getAuxValue());
		empty->hurtAndBreak(1, player);
		return empty;
	}

	if (player->abilities.instabuild || player->inventory->hasResource(Item::arrow_Id))
	{
		player->startUsingItem(instance, getUseDuration(instance));
		level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_LOADING_START, 0.5f, 1.0f);
	}
	return instance;
}

// released after a full draw: take an arrow from the inventory and swap to the loaded crossbow
void CrossbowItem::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	if (loaded) return;
	int held = getUseDuration(itemInstance) - durationLeft;
	if (held < CHARGE_TICKS) return;

	if (!player->abilities.instabuild)
	{
		if (!player->inventory->hasResource(Item::arrow_Id)) return;
		player->inventory->removeResource(Item::arrow_Id);
	}

	level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_LOADING_END, 1.0f, 1.0f);
	player->inventory->setItem(player->inventory->selected, std::make_shared<ItemInstance>(Item::crossbow_loaded, 1, itemInstance->getAuxValue()));
}
