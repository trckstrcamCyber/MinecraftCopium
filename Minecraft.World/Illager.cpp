#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "SynchedEntityData.h"
#include "Illager.h"
#include "Vex.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "BannerItem.h"
#include "SoundTypes.h"
#include "Random.h"
#include "JumpControl.h"
#include "OminousBottleItem.h"

Illager::Illager(Level *level) : Monster(level)
{
	// Monster's constructor can't reach this class's synched data, so it is defined here
	getEntityData()->define(DATA_FLAGS_ID, static_cast<byte>(0));

	inRaid = false;
	raidX = raidY = raidZ = 0;
	celebrateRaid = false;
	patrolling = false;
	patrolTargetSet = false;
	patrolX = patrolZ = 0;
}

void Illager::tick()
{
	if (!level->isClientSide)
	{
		// only the aggressive and celebrating bits are decided here; the captain and crossbow bits must survive
		byte flags = getEntityData()->getByte(DATA_FLAGS_ID);
		byte wanted = (getTarget() != nullptr) ? (flags | FLAG_AGGRESSIVE) : (flags & ~FLAG_AGGRESSIVE);
		bool cheer = celebrateRaid && getTarget() == nullptr && isAlive();
		wanted = cheer ? static_cast<byte>(wanted | FLAG_CELEBRATING) : static_cast<byte>(wanted & ~FLAG_CELEBRATING);
		if (flags != wanted) getEntityData()->set(DATA_FLAGS_ID, wanted);

		if (cheer)
		{
			if (onGround && random->nextInt(50) == 0) getJumpControl()->jump();
			if (random->nextInt(100) == 0) playSound(getCelebrateSoundId(), getSoundVolume(), getVoicePitch());
		}
	}
	Monster::tick();
}

bool Illager::isCelebrating()
{
	return (getEntityData()->getByte(DATA_FLAGS_ID) & FLAG_CELEBRATING) != 0;
}

int Illager::getCelebrateSoundId()
{
	switch (GetType())
	{
	case eTYPE_VINDICATOR:	return eSoundType_MOB_VINDICATOR_CELEBRATE;
	case eTYPE_PILLAGER:	return eSoundType_MOB_PILLAGER_CELEBRATE;
	case eTYPE_PROVOKER:	return eSoundType_MOB_PROVOKER_CELEBRATE;
	default:				return eSoundType_MOB_EVOKER_CELEBRATE;
	}
}

bool Illager::isAggressive()
{
	return (getEntityData()->getByte(DATA_FLAGS_ID) & 1) != 0;
}

bool Illager::useNewAi()
{
	return true;
}

Illager::ArmPose Illager::getArmPose()
{
	return isAggressive() ? ATTACKING : CROSSED;
}

bool Illager::isAlliedTo(shared_ptr<LivingEntity> other)
{
	if (other == nullptr) return false;
	if (other.get() == this) return true;
	if (dynamic_cast<Illager *>(other.get()) != nullptr || dynamic_cast<Vex *>(other.get()) != nullptr) return true;
	return Monster::isAlliedTo(other);
}

bool Illager::isCaptain()
{
	return (getEntityData()->getByte(DATA_FLAGS_ID) & FLAG_CAPTAIN) != 0;
}

void Illager::makeCaptain()
{
	byte flags = getEntityData()->getByte(DATA_FLAGS_ID);
	getEntityData()->set(DATA_FLAGS_ID, static_cast<byte>(flags | FLAG_CAPTAIN));

	setEquippedSlot(SLOT_HELM, BannerItem::createOminousBanner());
	dropChances[SLOT_HELM] = 1.0f;
}

void Illager::startPatrol(shared_ptr<Illager> leader, int targetX, int targetZ)
{
	patrolling = true;
	if (leader != nullptr && leader.get() != this) patrolLeaderRef = leader;
	if (leader == nullptr || leader.get() == this) setPatrolTarget(targetX, targetZ);
}

// A captain killed by a player drops an Ominous Bottle of a random level I-V (as in modern Minecraft; the banner it wears drops as well).
void Illager::die(DamageSource *source)
{
	if (!level->isClientSide && isCaptain() && source != nullptr)
	{
		shared_ptr<Entity> killer = source->getEntity();
		if (killer != nullptr && killer->instanceof(eTYPE_PLAYER))
		{
			spawnAtLocation(OminousBottleItem::create(1 + random->nextInt(OminousBottleItem::MAX_LEVEL)), 0.0f);
		}
	}
	Monster::die(source);
}
