#pragma once
#include "Basher.h"

// neoLegacy - Inquisitor, from the Illager Invasion mod: the Basher's big brother, a tank. 80 health, hits for 10, hard to knock back (0.8 resistance). It has the Basher's shield
// (blocks everything while it hunts; breaks to an axe, an iron golem or four blocks) but is only stunned for 2 seconds - and when the shield breaks it roars and throws
// everything non-monster within 8 blocks back, weakened for 10 seconds. It also smashes through leaves, cobwebs and wooden doors it runs into (when mob griefing is on).
// Drops iron, leather, emeralds, and now and then a Platinum Chunk. Joins raids on waves 3, 5 and 7. Holds the same stand-in shield as the Basher (see Basher::shieldItem).
class Inquisitor : public Basher
{
public:
	eINSTANCEOF GetType() { return eTYPE_INQUISITOR; }
	static Entity *create(Level *level) { return new Inquisitor(level); }

	Inquisitor(Level *level);

	virtual void aiStep();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual int getStunDuration() { return 40; }
	virtual void onShieldBroken();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
