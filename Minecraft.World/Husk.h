#pragma once
#include "Zombie.h"

// neoLegacy - Husk (modern Minecraft's desert zombie).
//   * does not burn in sunlight,
//   * its hits inflict Hunger (7 seconds x difficulty), on top of normal zombie damage,
//   * spawns in deserts in place of ordinary zombies; drops the same loot as a zombie,
//   * drawn slightly larger than a zombie (see HuskRenderer).
class Husk : public Zombie
{
public:
	eINSTANCEOF GetType() { return eTYPE_HUSK; }
	static Entity *create(Level *level) { return new Husk(level); }

	Husk(Level *level);

	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual void tick();

private:
	int underWaterTicks;
	void convertToZombie();

protected:
	virtual bool isSunSensitive();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
};
