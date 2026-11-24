#pragma once

#include "Goal.h"

class Skeleton;

class RangedBowAttackGoal : public Goal
{
private:
	Skeleton *mob;
	double speedModifier;
	int attackInterval;
	float attackRadiusSqr;
	int attackTime;
	int seeTime;
	int drawTime;
	bool strafingClockwise;
	bool strafingBackwards;
	int strafingTime;

public:
	RangedBowAttackGoal(Skeleton *mob, double speedModifier, int attackInterval, float attackRadius);

	void setMinAttackInterval(int attackInterval);
	bool isHoldingBow();
	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};
