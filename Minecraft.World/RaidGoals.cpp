#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.util.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "Random.h"
#include "Mth.h"
#include "Illager.h"
#include "RaidMember.h"
#include "RaidGoals.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// RaidMoveGoal
// ---------------------------------------------------------------------------------------------------------------------------------
RaidMoveGoal::RaidMoveGoal(PathfinderMob *mob, RaidMember *member, double speedModifier)
{
	this->mob = mob;
	this->member = member;
	this->speedModifier = speedModifier;
	cooldown = 0;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool RaidMoveGoal::canUse()
{
	if (!member->isInRaid() || mob->getTarget() != nullptr) return false;

	int x, y, z;
	member->getRaidCenter(&x, &y, &z);
	return mob->distanceToSqr(x + 0.5, y, z + 0.5) > 14.0 * 14.0;
}

bool RaidMoveGoal::canContinueToUse()
{
	if (!member->isInRaid() || mob->getTarget() != nullptr) return false;

	int x, y, z;
	member->getRaidCenter(&x, &y, &z);
	return mob->distanceToSqr(x + 0.5, y, z + 0.5) > 8.0 * 8.0;
}

void RaidMoveGoal::moveTowardsCenter()
{
	int x, y, z;
	member->getRaidCenter(&x, &y, &z);

	Vec3 *pos = RandomPos::getPosTowards(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 7, Vec3::newTemp(x + 0.5, y, z + 0.5));
	if (pos != nullptr) mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, speedModifier);
}

void RaidMoveGoal::start()
{
	cooldown = 0;
	moveTowardsCenter();
}

void RaidMoveGoal::tick()
{
	if (--cooldown <= 0)
	{
		cooldown = 40;
		if (mob->getNavigation()->isDone()) moveTowardsCenter();
	}
}

void RaidMoveGoal::stop()
{
	mob->getNavigation()->stop();
}

// ---------------------------------------------------------------------------------------------------------------------------------
// PatrolGoal
// ---------------------------------------------------------------------------------------------------------------------------------
PatrolGoal::PatrolGoal(Illager *mob, double speedModifier)
{
	this->mob = mob;
	this->speedModifier = speedModifier;
	cooldown = 0;
	setRequiredControlFlags(Control::MoveControlFlag);
}

bool PatrolGoal::canUse()
{
	return mob->isPatrolling() && mob->getTarget() == nullptr;
}

bool PatrolGoal::canContinueToUse()
{
	return mob->isPatrolling() && mob->getTarget() == nullptr;
}

void PatrolGoal::start()
{
	cooldown = 0;
}

void PatrolGoal::tick()
{
	if (--cooldown > 0) return;
	cooldown = 30;

	if (mob->isCaptain())
	{
		// a fresh far-off point when it has none or has come close to it
		double dx = mob->patrolTargetX() + 0.5 - mob->x;
		double dz = mob->patrolTargetZ() + 0.5 - mob->z;
		if (!mob->hasPatrolTarget() || dx * dx + dz * dz < 12.0 * 12.0)
		{
			double angle = mob->getRandom()->nextDouble() * 2 * PI;
			mob->setPatrolTarget(static_cast<int>(mob->x + cos(angle) * 160), static_cast<int>(mob->z + sin(angle) * 160));
		}

		Vec3 *pos = RandomPos::getPosTowards(dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16, 7, Vec3::newTemp(mob->patrolTargetX() + 0.5, mob->y, mob->patrolTargetZ() + 0.5));
		if (pos != nullptr) mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, speedModifier);
	}
	else
	{
		shared_ptr<Illager> leader = mob->getPatrolLeader();
		if (leader == nullptr || leader->removed)
		{
			mob->endPatrol();	// the captain is gone: the patrol disbands
			return;
		}
		if (mob->distanceToSqr(leader) > 9.0 * 9.0) mob->getNavigation()->moveTo(leader, speedModifier * 1.15);
	}
}

void PatrolGoal::stop()
{
	mob->getNavigation()->stop();
}
