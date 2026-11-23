#pragma once
#include "Illager.h"
#include "RangedAttackMob.h"

// neoLegacy - Marauder, from the Illager Invasion mod: a hatchet-throwing illager. 24 health. Up close (within 4 blocks) it fights with its hatchet; further out, up to
// 8 blocks, it throws it (8 damage, one throw every two seconds). It always drops an Ominous Bottle (level I-V), and emeralds when a player kills it.
// It carries a Platinum Infused Hatchet (the usual chance of dropping it when it dies) and throws copies of it: 8 damage, bounces off what it hits, cannot be picked up.
// The throw itself is a flagged Snowball (like the Ice Ball), so it needs no entity type of its own. Joins raids from wave 2 on.
class Marauder : public Illager, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_MARAUDER; }
	static Entity *create(Level *level) { return new Marauder(level); }

	Marauder(Level *level);

	// arms hang loose while it walks about; raised to attack when it has a
	// target (same pattern as the Pillager, 2026-11-23). The base Illager
	// pose is CROSSED when idle, which hides the arms entirely.
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
