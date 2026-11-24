#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "IconRegister.h"
#include "LingeringPotionItem.h"
#include "SoundTypes.h"

const wstring LingeringPotionItem::LINGERING_ICON = L"potion_bottle_lingering";

LingeringPotionItem::LingeringPotionItem(int id) : PotionItem(id)
{
	iconLingering = nullptr;
}

bool LingeringPotionItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return true;
}

shared_ptr<ItemInstance> LingeringPotionItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (!player->abilities.instabuild) instance->count--;
	level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
	if (!level->isClientSide)
	{
		level->addEntity(std::make_shared<ThrownPotion>(level, player, std::make_shared<ItemInstance>(id, 1, instance->getAuxValue())));
	}
	return instance;
}

Icon *LingeringPotionItem::getIcon(int auxValue)
{
	return iconLingering;
}

wstring LingeringPotionItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->getAuxValue() == 0)
	{
		return app.GetString(IDS_ITEM_LINGERING_WATER_BOTTLE);
	}
	return PotionItem::getHoverName(itemInstance);
}

void LingeringPotionItem::registerIcons(IconRegister *iconRegister)
{
	PotionItem::registerIcons(iconRegister);
	iconLingering = iconRegister->registerIcon(LINGERING_ICON);
}
