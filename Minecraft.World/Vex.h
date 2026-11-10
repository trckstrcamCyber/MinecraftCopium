#pragma once
#include "Monster.h"

class Pos;

// neoLegacy - Vex: the small winged spirit an Evoker summons. It flies through walls, hunts whatever its owner is hunting,
// charges at its target with an iron sword and withers away after a limited life.
class Vex : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_VEX; }
	static Entity *create(Level *level) { return new Vex(level); }

private:
	static const int DATA_FLAGS_ID = 16;	// bit 0: charging
	weak_ptr<Mob> owner;
	bool hasBound;
	int boundX, boundY, boundZ;
	bool hasLimitedLife;
	int limitedLifeTicks;
	bool hasWanted;
	double wantedX, wantedY, wantedZ;

public:
	Vex(Level *level);

	virtual void tick();
	virtual void travel(float xa, float ya);

	void setOwner(shared_ptr<Mob> owner);
	shared_ptr<Mob> getOwner();
	void setBoundOrigin(int x, int y, int z);
	void setLifeTicks(int ticks);

	bool isCharging();
	void setIsCharging(bool charging);

	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual bool isAlliedTo(shared_ptr<LivingEntity> other);
	virtual bool isInWall();	// it flies through walls, so being inside one never hurts

protected:
	virtual void registerAttributes();
	virtual bool useNewAi();
	virtual void newServerAiStep();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual bool makeStepSound();
	virtual void causeFallDamage(float distance);
	virtual void checkFallDamage(double ya, bool onGround);
	virtual bool isIgnoringTileTriggers();
};
