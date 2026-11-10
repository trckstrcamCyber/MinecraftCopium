#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "SynchedEntityData.h"
#include "Illager.h"
#include "Vex.h"

Illager::Illager(Level *level) : Monster(level)
{
	// Monster's constructor can't reach this class's synched data, so it is defined here
	getEntityData()->define(DATA_FLAGS_ID, static_cast<byte>(0));
}

void Illager::tick()
{
	if (!level->isClientSide)
	{
		byte aggressive = (getTarget() != nullptr) ? 1 : 0;
		if (getEntityData()->getByte(DATA_FLAGS_ID) != aggressive) getEntityData()->set(DATA_FLAGS_ID, aggressive);
	}
	Monster::tick();
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
