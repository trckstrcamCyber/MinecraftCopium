#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "SoundTypes.h"
#include "IllusionaryDustItem.h"

IllusionaryDustItem::IllusionaryDustItem(int id) : Item(id)
{
}

shared_ptr<ItemInstance> IllusionaryDustItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	level->playSound(player->x, player->y, player->z, eSoundType_MOB_ILLUSIONER_MIRROR_MOVE, 1.0f, 1.0f);
	if (!level->isClientSide)
	{
		player->addEffect(new MobEffectInstance(MobEffect::invisibility->id, INVISIBILITY_TICKS, 0));
		player->addEffect(new MobEffectInstance(MobEffect::movementSpeed->id, SPEED_TICKS, 0));
		if (!player->abilities.instabuild) itemInstance->count--;
	}
	return itemInstance;
}
