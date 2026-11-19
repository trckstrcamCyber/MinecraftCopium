#pragma once

#include "Entity.h"

class FireworksRocketEntity : public Entity
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_FIREWORKS_ROCKET; }
	static Entity *create(Level *level) { return new FireworksRocketEntity(level); }

private:
	static const int DATA_ID_FIREWORKS_ITEM = 8;
	static const int DATA_ID_ATTACHED = 9;	// neoLegacy - entity id of the elytra flier this rocket is boosting, -1 = none
	static const int DATA_ID_SHOT = 10;		// neoLegacy - 1 when the rocket was fired from a crossbow: flies straight and bursts on what it hits
	int shooterId = -1;						// neoLegacy - who fired it (server side), so kills are credited to them

	int life;
	int lifetime;

	// constructor needed for level loader
public:
	FireworksRocketEntity(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual bool shouldRenderAtSqrDistance(double distance);

	FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem);

	// neoLegacy - boosting rocket (modern Java behaviour): launched in mid-air by a player who is gliding on an
	// elytra. It rides along with that player and pushes them forward along their look direction until it burns out.
	FireworksRocketEntity(Level *level, shared_ptr<ItemInstance> sourceItem, shared_ptr<Entity> attachedTo);
	shared_ptr<Entity> getAttachedEntity();

	// neoLegacy - fired from a crossbow: starts at (x, y, z) already moving at (xd, yd, zd), straight, no acceleration
	FireworksRocketEntity(Level *level, shared_ptr<ItemInstance> sourceItem, shared_ptr<LivingEntity> shooter, double x, double y, double z, double xd, double yd, double zd);
	bool isShot();

	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void handleEntityEvent(byte eventId);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual float getShadowHeightOffs();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);
	virtual bool isAttackable();

private:
	// neoLegacy
	int getExplosionCount();		// number of firework stars packed into the rocket
	void dealExplosionDamage();		// stars hurt the entity being boosted and everything within 5 blocks (like Java)
	bool shotHitsSomething();		// a crossbow rocket's path this tick crosses a living thing
	void burst();					// damage, the firework bang on every client, and gone
	shared_ptr<Entity> findEntityById(int id);
	void initFlight(shared_ptr<ItemInstance> sourceItem);	// stores the item + works out the lifetime from the flight duration
};