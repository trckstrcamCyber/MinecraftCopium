#pragma once
#include "Zombie.h"

// neoLegacy - Drowned (modern Minecraft's aquatic zombie).
//   * a zombie that swims: in water it chases its target in 3D at full speed,
//   * spawns around rivers and oceans, and plain zombies that stay submerged for 30 seconds turn into
//     drowned (see Zombie::convertToDrowned),
//   * by day, out of water, it heads back to the nearest water; it does NOT burn in sunlight and
//     breathes underwater,
//   * drops rotten flesh and, rarely, a gold ingot,
//   * drawn with a second "outer layer" skin (DrownedRenderer).
class Drowned : public Zombie
{
public:
	eINSTANCEOF GetType() { return eTYPE_DROWNED; }
	static Entity *create(Level *level) { return new Drowned(level); }

private:
	int waterSearchCooldown;

public:
	Drowned(Level *level);

	virtual void aiStep();
	virtual bool canSpawn();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual bool isSunSensitive();
	virtual int decreaseAirSupply(int currentSupply);
	virtual void dropRareDeathLoot(int rareLootLevel);

private:
	bool findNearbyWater(double *wx, double *wy, double *wz);
};
