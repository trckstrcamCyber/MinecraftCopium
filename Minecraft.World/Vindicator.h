#pragma once
#include "Illager.h"

// neoLegacy - Vindicator: the axe-wielding illager. 24 health, hits for 5, hunts players, villagers and iron golems.
class Vindicator : public Illager
{
public:
	eINSTANCEOF GetType() { return eTYPE_VINDICATOR; }
	static Entity *create(Level *level) { return new Vindicator(level); }

	Vindicator(Level *level);

	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
