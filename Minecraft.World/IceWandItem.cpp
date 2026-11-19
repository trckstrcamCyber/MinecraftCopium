#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "IceCube.h"
#include "IceWandItem.h"

IceWandItem::IceWandItem(int id) : Item(id)
{
	maxStackSize = 1;
}

bool IceWandItem::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob)
{
	if (mob == nullptr || itemInstance == nullptr) return false;
	Level *level = player->level;
	if (level->isClientSide) return true;	// the server decides

	// cooldown: stored on the stack as the game tick it is next usable
	int now = static_cast<int>(level->getGameTime());
	if (itemInstance->hasTag() && itemInstance->getTag()->contains(L"IceWandNext") && itemInstance->getTag()->getInt(L"IceWandNext") > now)
	{
		return false;
	}
	if (!itemInstance->hasTag()) itemInstance->setTag(new CompoundTag());
	itemInstance->getTag()->putInt(L"IceWandNext", now + COOLDOWN_TICKS);

	level->addEntity(std::make_shared<IceCube>(level, dynamic_pointer_cast<LivingEntity>(player), mob, 20, true));
	itemInstance->hurtAndBreak(1, player);
	level->playEntitySound(player, eSoundType_MOB_ICEOLOGER_PREPARE, 1.0f, 1.0f);
	return true;
}
