#pragma once
#include "Goal.h"

class Goat;
class LivingEntity;

// neoLegacy - the goat's random headbutt. Every so often an adult goat picks a nearby player or mob,
// lowers its head for a moment (wind-up), then charges and rams it: 2 damage and a hard knockback.
// After a ram the goat leaves it alone for a good while (30-90 seconds).
class GoatRamGoal : public Goal
{
private:
	Goat *goat;
	weak_ptr<LivingEntity> target;
	int phase;			// 0 = idle, 1 = wind-up, 2 = charging
	int timer;			// ticks left in the current phase
	int nextRamTick;	// goat->tickCount before which it won't start another ram

	shared_ptr<LivingEntity> findTarget();

public:
	GoatRamGoal(Goat *goat);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};
