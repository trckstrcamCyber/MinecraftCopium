#pragma once
#include "Zombie.h"

// neoLegacy - Frostbite: the frozen zombie of the cold biomes.
//   * melee hits do normal zombie damage (2.5 / 3 / 4.5 after difficulty scaling) and inflict Freezing for 3 seconds
//     (see MobEffect::freezing and LivingEntity::updateFreezing),
//   * immune to Freezing itself (the whole zombie family is),
//   * spawns in the snowy biomes in place of zombies (see IceBiome),
//   * melts back into an ordinary zombie after 15 seconds submerged in water,
//   * drops Ice Balls (the Potion of Freezing ingredient) as well as the usual zombie loot,
//   * one in three spawns carrying a stack of Ice Balls and pelts you with them from a distance before closing in,
//   * otherwise behaves like a zombie (still burns in daylight).
class Frostbite : public Zombie
{
public:
	eINSTANCEOF GetType() { return eTYPE_FROSTBITE; }
	static Entity *create(Level *level) { return new Frostbite(level); }

	Frostbite(Level *level);

	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual void tick();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);

	// the ice-throwing behaviour (see FrostbiteThrowIceGoal in Frostbite.cpp)
	bool hasIceBalls();
	void throwIceBall(shared_ptr<LivingEntity> target);

	// Synched to the clients (the target itself only exists on the server): lets the model raise its arms while it hunts.
	bool isAggressive();

private:
	static const int DATA_AGGRESSIVE_ID = 15;	// Zombie uses 12-14
	static const int MELT_TICKS = 300;			// 15 seconds in water
	int underWaterTicks;
	void meltIntoZombie();

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
};
