#pragma once
#include "SpellcasterIllager.h"
#include "RangedAttackMob.h"

// neoLegacy - Provoker, the first mob ported from the Illager Invasion mod: an illager war-caller. 24 health, shoots a bow from a distance and, every
// fifteen seconds while it has a target, calls out a war cry that gives every illager within 12 blocks Strength and Speed for six seconds (itself
// included). It joins raids from the first wave on and now and then marches with a patrol. Drops arrows, and emeralds when a player kills it.
class Provoker : public SpellcasterIllager, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_PROVOKER; }
	static Entity *create(Level *level) { return new Provoker(level); }

	Provoker(Level *level);

	virtual int getCastingSoundId();
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
