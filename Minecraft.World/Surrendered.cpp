#include "stdafx.h"
#include "AttributeInstance.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "MobEffectInstance.h"
#include "SoundTypes.h"
#include "Surrendered.h"

Surrendered::Surrendered(Level *level) : Vex(level)
{
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(14);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5);
	setHealth(getMaxHealth());
	xpReward = 5;
	summoned = false;
	shortened = false;
}

// Java: if its owner is gone and it has more than a second to live, it only has a second left
void Surrendered::tick()
{
	Vex::tick();
	if (!level->isClientSide && summoned && !shortened)
	{
		shared_ptr<Mob> owner = getOwner();
		if (owner == nullptr || !owner->isAlive())
		{
			setLifeTicks(20);
			shortened = true;
		}
	}
}

bool Surrendered::doHurtTarget(shared_ptr<Entity> target)
{
	if (!Vex::doHurtTarget(target)) return false;
	if (target != nullptr && target->instanceof(eTYPE_LIVINGENTITY))
	{
		dynamic_pointer_cast<LivingEntity>(target)->addEffect(new MobEffectInstance(MobEffect::movementSlowdown->id, 60, 1));
	}
	return true;
}

// no iron sword (the Vex's own spawn gives it one)
MobGroupData *Surrendered::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	return Monster::finalizeMobSpawn(groupData);
}

int Surrendered::getAmbientSound() { return eSoundType_MOB_SURRENDERED_IDLE; }
int Surrendered::getHurtSound() { return eSoundType_MOB_SURRENDERED_HURT; }
int Surrendered::getDeathSound() { return eSoundType_MOB_SURRENDERED_DEATH; }
