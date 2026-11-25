#include "stdafx.h"
#include "AABB.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "com.mojang.nbt.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "SoundTypes.h"
#include "HornOfSightItem.h"

HornOfSightItem::HornOfSightItem(int id) : Item(id)
{
	maxStackSize = 1;
}

shared_ptr<ItemInstance> HornOfSightItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	if (level->isClientSide) return itemInstance;

	// the cooldown lives on the horn: the game time it can be blown again
	int64_t now = level->getGameTime();
	if (!itemInstance->hasTag()) itemInstance->setTag(new CompoundTag());
	CompoundTag *tag = itemInstance->getTag();
	if (now < tag->getLong(L"HornReady")) return itemInstance;
	tag->putLong(L"HornReady", now + COOLDOWN_TICKS);

	bool sense = player->isSneaking();
	double range = sense ? SENSE_RANGE : RANGE;
	int ticks = sense ? SENSE_GLOW_TICKS : GLOW_TICKS;

	AABB *box = player->bb->grow(range, 48.0, range);
	vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(Monster), box);
	int marked = 0;
	if (found != nullptr)
	{
		for (size_t i = 0; i < found->size(); i++)
		{
			shared_ptr<Monster> monster = dynamic_pointer_cast<Monster>(found->at(i));
			if (monster == nullptr || !monster->isAlive()) continue;
			if (monster->distanceToSqr(player) > range * range) continue;
			if (sense)
			{
				// the Illager Sense: raiders and their kin only
				bool kin = dynamic_cast<Illager *>(monster.get()) != nullptr || monster->GetType() == eTYPE_RAVAGER || monster->GetType() == eTYPE_WITCH || monster->GetType() == eTYPE_SURRENDERED;
				if (!kin) continue;
			}
			monster->addEffect(new MobEffectInstance(MobEffect::glowing->id, ticks, 0));
			marked++;
		}
		delete found;
	}

	// a low war cry (higher and shorter when it found nothing)
	level->playSound(player->x, player->y, player->z, eSoundType_MOB_PILLAGER_CELEBRATE, 2.0f, marked > 0 ? 0.55f : 0.8f);
	return itemInstance;
}
