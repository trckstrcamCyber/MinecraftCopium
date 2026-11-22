#pragma once
#include "Illager.h"

// neoLegacy - Basher, from the Illager Invasion mod: a shield-bearing illager. 32 health, hits for 3. While it is hunting and holds its shield it blocks every
// melee hit and arrow outright. The shield breaks - and the Basher is stunned for three seconds, swapping to a stone axe - when it is hit with an axe, hit by an
// iron golem, or after it has blocked four times.
// The mod's shield is the vanilla shield, which this build does not have: BASHER_SHIELD_ITEM below stands in for it (an iron door, drawn as a flat panel).
// Drops emeralds and some iron. Joins raids from the second wave.
class Basher : public Illager
{
public:
	eINSTANCEOF GetType() { return eTYPE_BASHER; }
	static Entity *create(Level *level) { return new Basher(level); }

	Basher(Level *level);

	virtual void tick();
	virtual bool hurt(DamageSource *source, float dmg);
	virtual bool isImmobile();
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	// the item standing in for the shield; change this once there is a real one
	static Item *shieldItem();

protected:
	int stunTicks;
	int blockedCount;

	bool hasShield();
	void setStunTicks(int ticks);
	virtual int getStunDuration() { return 60; }	// ticks
	virtual void onShieldBroken() {}				// the Inquisitor roars here

	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
