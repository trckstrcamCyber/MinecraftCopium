#pragma once
#include "Goal.h"

class Goat;
class LivingEntity;

// neoLegacy - the goat's random headbutt. Every so often an adult goat picks a nearby player or mob,
// lowers its head for a moment (wind-up), then charges in a straight line and rams it: 2 damage and
// a hard knockback. If the charge slams the goat into a solid block instead (Java: Goat.dropHorn()),
// it loses a horn — up to two per goat, then it rams hornless. After a ram the goat leaves it alone
// for a good while (30-90 seconds).
class GoatRamGoal : public Goal
{
private:
	Goat *goat;
	weak_ptr<LivingEntity> target;
	int phase;			// 0 = idle, 1 = wind-up, 2 = charging
	int timer;			// ticks left in the current phase
	int nextRamTick;	// goat->tickCount before which it won't start another ram
	double lastChargeX, lastChargeZ;	// goat position on the previous charge tick (stuck detection)
	int stuckTicks;		// consecutive charge ticks the goat has barely moved

	shared_ptr<LivingEntity> findTarget();
	bool isBlockedBySolidBlock();	// true if a solid block is directly ahead at head height

public:
	GoatRamGoal(Goat *goat);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};
