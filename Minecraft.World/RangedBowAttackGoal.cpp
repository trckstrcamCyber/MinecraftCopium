#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.phys.h"
#include "RangedBowAttackGoal.h"

RangedBowAttackGoal::RangedBowAttackGoal(Skeleton *mob, double speedModifier, int attackInterval, float attackRadius)
{
	this->mob = mob;
	this->speedModifier = speedModifier;
	this->attackInterval = attackInterval;
	attackRadiusSqr = attackRadius * attackRadius;
	attackTime = -1;
	seeTime = 0;
	drawTime = -1;
	strafingClockwise = false;
	strafingBackwards = false;
	strafingTime = -1;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

void RangedBowAttackGoal::setMinAttackInterval(int attackInterval)
{
	this->attackInterval = attackInterval;
}

bool RangedBowAttackGoal::isHoldingBow()
{
	shared_ptr<ItemInstance> item = mob->getCarriedItem();
	return item != nullptr && item->id == Item::bow_Id;
}

bool RangedBowAttackGoal::canUse()
{
	return mob->getTarget() != nullptr && isHoldingBow();
}

bool RangedBowAttackGoal::canContinueToUse()
{
	return (canUse() || !mob->getNavigation()->isDone()) && isHoldingBow();
}

void RangedBowAttackGoal::start()
{
	Goal::start();
	mob->setAggressive(true);
}

void RangedBowAttackGoal::stop()
{
	Goal::stop();
	mob->setAggressive(false);
	seeTime = 0;
	attackTime = -1;
	drawTime = -1;
}

void RangedBowAttackGoal::tick()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	if (target == nullptr) return;

	double distance = mob->distanceToSqr(target->x, target->bb->y0, target->z);
	bool canSee = mob->getSensing()->canSee(target);
	if (canSee != (seeTime > 0)) seeTime = 0;

	if (canSee) ++seeTime;
	else --seeTime;

	if (distance > attackRadiusSqr || seeTime < 20)
	{
		mob->getNavigation()->moveTo(target, speedModifier);
		strafingTime = -1;
	}
	else
	{
		mob->getNavigation()->stop();
		++strafingTime;
	}

	if (strafingTime >= 20)
	{
		if (mob->getRandom()->nextFloat() < 0.3) strafingClockwise = !strafingClockwise;
		if (mob->getRandom()->nextFloat() < 0.3) strafingBackwards = !strafingBackwards;
		strafingTime = 0;
	}

	if (strafingTime > -1)
	{
		if (distance > attackRadiusSqr * 0.75f) strafingBackwards = false;
		else if (distance < attackRadiusSqr * 0.25f) strafingBackwards = true;

		mob->getMoveControl()->strafe(strafingBackwards ? -0.5f : 0.5f, strafingClockwise ? 0.5f : -0.5f);
		mob->lookAt(target, 30, 30);
	}
	else
	{
		mob->getLookControl()->setLookAt(target, 30, 30);
	}

	if (drawTime >= 0)
	{
		if (!canSee)
		{
			if (seeTime < -60) drawTime = -1;
		}
		else if (++drawTime >= BowItem::MAX_DRAW_DURATION)
		{
			float power = drawTime / static_cast<float>(BowItem::MAX_DRAW_DURATION);
			power = (power * power + power * 2) / 3;
			if (power > 1) power = 1;
			drawTime = -1;
			mob->performRangedAttack(target, power);
			attackTime = attackInterval;
		}
	}
	else if (--attackTime <= 0 && seeTime >= -60)
	{
		drawTime = 0;
	}
}
