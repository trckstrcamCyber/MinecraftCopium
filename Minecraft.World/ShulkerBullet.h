#pragma once
using namespace std;

#include "Entity.h"

class LivingEntity;
class HitResult;

class ShulkerBullet : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_SHULKER_BULLET; }
	static Entity *create(Level *level) { return new ShulkerBullet(level); }

	static const int LEVITATION_DURATION = 200;

private:
	weak_ptr<LivingEntity> owner;
	weak_ptr<Entity> target;
	int moveDirection;
	int steps;
	double powerX, powerY, powerZ;
	wstring ownerUUID;
	wstring targetUUID;
	int ownerX, ownerY, ownerZ;
	int targetX, targetY, targetZ;

	void _init();
	void resolveReferences();
	void selectNextMoveDirection(int axis);
	void onHit(HitResult *res);

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	ShulkerBullet(Level *level);
	ShulkerBullet(Level *level, double x, double y, double z, double xd, double yd, double zd);
	ShulkerBullet(Level *level, shared_ptr<LivingEntity> owner, shared_ptr<Entity> target, int axis);

	virtual void tick();
	virtual bool hurt(DamageSource *source, float damage);
	virtual bool isPickable();
	virtual bool isOnFire();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);
	virtual bool shouldRenderAtSqrDistance(double distance);
};
