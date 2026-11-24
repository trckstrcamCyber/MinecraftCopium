#pragma once
using namespace std;

#include "Monster.h"
#include "Goal.h"
#include "TargetGoal.h"

class Shulker : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_SHULKER; }
	static Entity *create(Level *level) { return new Shulker(level); }

	class ShulkerAttackGoal : public Goal
	{
	public:
		ShulkerAttackGoal(Shulker *shulker);
		virtual bool canUse();
		virtual void start();
		virtual void stop();
		virtual void tick();
	private:
		Shulker *shulker;
		int attackTime;
	};

	class ShulkerPeekGoal : public Goal
	{
	public:
		ShulkerPeekGoal(Shulker *shulker);
		virtual bool canUse();
		virtual bool canContinueToUse();
		virtual void start();
		virtual void stop();
		virtual void tick();
	private:
		Shulker *shulker;
		int peekTime;
	};

	class ShulkerNearestAttackGoal : public TargetGoal
	{
	public:
		ShulkerNearestAttackGoal(Shulker *shulker);
		virtual bool canUse();
		virtual void start();
	private:
		Shulker *shulker;
		weak_ptr<LivingEntity> target;
	};

private:
	static const int DATA_ATTACH_FACE = 16;
	static const int DATA_ATTACH_X = 17;
	static const int DATA_ATTACH_Y = 18;
	static const int DATA_ATTACH_Z = 19;
	static const int DATA_PEEK = 20;
	static const int DATA_COLOR = 21;
	static const int NO_ATTACH_POSITION = -1000;

	float oldPeekAmount;
	float peekAmount;
	int oldAttachX, oldAttachY, oldAttachZ;
	bool hasOldAttach;
	int clientSideTeleportInterpolation;

	bool isSolidAt(int x, int y, int z);
	void snapToAttachPosition();

protected:
	virtual void defineSynchedData();
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual bool makeStepSound();
	virtual bool removeWhenFarAway();

public:
	Shulker(Level *level);

	virtual bool useNewAi();
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void tick();
	virtual void aiStep();
	virtual bool hurt(DamageSource *source, float dmg);
	virtual AABB *getCollideBox();
	virtual bool isPushable();
	virtual void push(shared_ptr<Entity> e);
	virtual float getHeadHeight();
	virtual int getMaxHeadXRot();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

	bool teleportSomewhere();
	bool isClosed();

	int getAttachFace();
	void setAttachFace(int face);
	bool hasAttachPosition();
	int getAttachX();
	int getAttachY();
	int getAttachZ();
	void setAttachPosition(int x, int y, int z);

	int getRawPeekAmount();
	void setRawPeekAmount(int amount);
	float getClientPeekAmount(float a);

	int getClientSideTeleportInterpolation() { return clientSideTeleportInterpolation; }
	bool hasOldAttachPosition() { return hasOldAttach; }
	int getOldAttachX() { return oldAttachX; }
	int getOldAttachY() { return oldAttachY; }
	int getOldAttachZ() { return oldAttachZ; }

	int getColor();
};
