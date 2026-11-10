#pragma once
#include "Illager.h"
#include "RangedAttackMob.h"

// neoLegacy - Pillager: the crossbow illager. 24 health, keeps its distance and shoots arrows (speed 1.6, same shot as a skeleton's).
// It spawns holding a Crossbow (which it can drop) and hunts players, villagers and iron golems.
class Pillager : public Illager, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_PILLAGER; }
	static Entity *create(Level *level) { return new Pillager(level); }

	Pillager(Level *level);

	virtual ArmPose getArmPose();
	bool isChargingCrossbow();			// synched: drawing the string back
	bool isCrossbowLoaded();			// synched: arrow loaded, aiming
	void setChargingCrossbow(bool charging);
	void setCrossbowLoaded(bool loaded);
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
