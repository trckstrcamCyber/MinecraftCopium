#pragma once
#include "Vex.h"

// neoLegacy - Surrendered, from the Illager Invasion mod: the Invoker's undead servants. A Vex (it flies, hunts what its owner hunts and fades away after a limited life) that is
// drawn as a pale skeleton instead of a winged spirit. 14 health, hits for 5 and slows whatever it hits for three seconds (Slowness II). Carries nothing and drops nothing.
// When its owner dies, a summoned one has only a second left.
class Surrendered : public Vex
{
private:
	bool summoned;	// made by an Invoker (so it follows its owner's fate)
	bool shortened;

public:
	eINSTANCEOF GetType() { return eTYPE_SURRENDERED; }
	static Entity *create(Level *level) { return new Surrendered(level); }

	Surrendered(Level *level);

	void setSummoned() { summoned = true; }

	virtual void tick();
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
