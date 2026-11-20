#pragma once
#include "Illager.h"
#include "RangedAttackMob.h"

// neoLegacy - Alchemist, from the Illager Invasion mod: a bow illager that every eight seconds swaps its bow for a potion and throws it at its
// target - Poison, Slowness or Weakness. (The mod throws lingering potions; this build only has splash potions, so it throws those.)
// 24 health. Drops gunpowder, glass bottles, and emeralds when a player kills it. Joins raids from wave 4 on.
class Alchemist : public Illager, public RangedAttackMob
{
private:
	// server only: counts down to the next potion; once it is out the potion is what the next ranged attack throws
	int potionCooldown;
	bool potionReady;
	int potionValue;

public:
	eINSTANCEOF GetType() { return eTYPE_ALCHEMIST; }
	static Entity *create(Level *level) { return new Alchemist(level); }

	Alchemist(Level *level);

	virtual void tick();
	virtual ArmPose getArmPose();
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
