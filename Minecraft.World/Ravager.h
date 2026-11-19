#pragma once
#include "Monster.h"
#include "RaidMember.h"

// neoLegacy - Ravager: the big illager beast. 100 health, hits for 12 and throws its victim away, 0.3 speed (0.35 once it has a target),
// 75% knockback resistance, tears through leaves when it runs into them, drops a saddle. Hunts players, villagers and iron golems.
// Not in yet: the stun / roar that follow a blocked hit (needs shields), riders (needs raids) and natural spawning (needs outposts/raids).
class Ravager : public Monster, public RaidMember
{
public:
	eINSTANCEOF GetType() { return eTYPE_RAVAGER; }
	static Entity *create(Level *level) { return new Ravager(level); }

	Ravager(Level *level);

	virtual void aiStep();
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual void handleEntityEvent(byte id);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	// RaidMember
	virtual bool isInRaid() { return inRaid; }
	virtual void getRaidCenter(int *x, int *y, int *z) { *x = raidX; *y = raidY; *z = raidZ; }
	virtual void joinRaid(int x, int y, int z) { inRaid = true; raidX = x; raidY = y; raidZ = z; }
	virtual void leaveRaid() { inRaid = false; }

	int getAttackTick() { return attackTick; }	// 10 down to 0 while it bites (the model lunges its head and opens its mouth)

protected:
	// Without this the mob falls back to the old pre-goal AI (see Illager::useNewAi)
	virtual bool useNewAi() { return true; }
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);

private:
	int attackTick;
	bool inRaid;
	int raidX, raidY, raidZ;
};
