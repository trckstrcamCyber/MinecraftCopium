#pragma once

#include "Goal.h"

class PathfinderMob;
class RaidMember;
class Illager;

// neoLegacy - walks a raider towards the village centre while it has nothing to fight. It moves in hops of about 16 blocks, because a
// path to a far-away point is not found in one go.
class RaidMoveGoal : public Goal
{
private:
	PathfinderMob *mob;
	RaidMember *member;
	double speedModifier;
	int cooldown;

	void moveTowardsCenter();

public:
	RaidMoveGoal(PathfinderMob *mob, RaidMember *member, double speedModifier);

	bool canUse();
	bool canContinueToUse();
	void start();
	void tick();
	void stop();
};

// neoLegacy - a patrol: the captain wanders towards a far-off point, the rest of the patrol follows the captain.
class PatrolGoal : public Goal
{
private:
	Illager *mob;
	double speedModifier;
	int cooldown;

public:
	PatrolGoal(Illager *mob, double speedModifier);

	bool canUse();
	bool canContinueToUse();
	void start();
	void tick();
	void stop();
};
