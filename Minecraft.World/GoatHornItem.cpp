#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "com.mojang.nbt.h"
#include "SoundTypes.h"
#include "GoatHornItem.h"

GoatHornItem::GoatHornItem(int id) : Item(id)
{
	maxStackSize = 1;
}

shared_ptr<ItemInstance> GoatHornItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	if (level->isClientSide) return itemInstance;

	// Java: 7 seconds between blasts. The cooldown lives on the horn itself.
	int64_t now = level->getGameTime();
	if (!itemInstance->hasTag()) itemInstance->setTag(new CompoundTag());
	CompoundTag *tag = itemInstance->getTag();
	if (now < tag->getLong(L"HornReady")) return itemInstance;
	tag->putLong(L"HornReady", now + COOLDOWN_TICKS);

	// One of the eight horn calls, picked at random by the sound engine
	// (item.goat_horn.call -> call1..8.ogg). The horn is not used up.
	level->playSound(player->x, player->y, player->z, eSoundType_ITEM_GOAT_HORN_CALL, 1.0f, 1.0f);
	return itemInstance;
}
