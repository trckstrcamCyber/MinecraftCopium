#pragma once
using namespace std;

#include "Entity.h"
#include "Projectile.h"

class Level;
class CompoundTag;

class Arrow : public Entity, public Projectile
{
public:
	eINSTANCEOF GetType() {	return eTYPE_ARROW; }
	static Entity *create(Level *level) { return new Arrow(level); }

private:
	// base damage, multiplied with velocity
	static const double ARROW_BASE_DAMAGE;

public:
	static const int PICKUP_DISALLOWED = 0;
	static const int PICKUP_ALLOWED = 1;
	static const int PICKUP_CREATIVE_ONLY = 2;

private:
	static const int ID_FLAGS = 16;
	static const int FLAG_CRIT = 1;
	static const int ID_ARROW_TYPE = 17;
	static const int ID_POTION = 18;

private:
	int xTile;
	int yTile;
	int zTile;
	int lastTile;
	int lastData;
	bool inGround;

public:
	int pickup;
	int shakeTime;
	shared_ptr<Entity> owner;

private:
	double baseDamage;

	int knockback;

	// neoLegacy Added - Explosive enchantment level (0 = not explosive)
	int explosiveLevel;

	// neoLegacy Added - crossbow enchantments (set by CrossbowItem when it fires; server side only, nothing here is synched)
	int pierceLevel;				// Piercing: how many EXTRA mobs the arrow passes through (0 = stops at the first one)
	bool torchArrow;				// Torched: this is a shot torch - it ignites every mob it hits and is placed where it hits a block
	int lootingLevel;				// Looting of the crossbow that shot it; -1 = not set, so the shooter's held item decides as for any arrow
	vector<int> piercedEntities;	// ids of the mobs a piercing arrow has already hit

private:
	int life;
	int flightTime;

	// 4J - added common ctor code.
	void _init();

public:
	Arrow(Level *level);
	Arrow(Level *level, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> target, float power, float uncertainty);
	Arrow(Level *level, double x, double y, double z);
	Arrow(Level *level, shared_ptr<LivingEntity> mob, float power);

protected:
	virtual void defineSynchedData();

public:
	void shoot(double xd, double yd, double zd, float pow, float uncertainty);
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void playerTouch(shared_ptr<Player> player);

protected:
	virtual bool makeStepSound();

public:
	virtual float getShadowHeightOffs();

	void setBaseDamage(double baseDamage);
	double getBaseDamage();
	void setKnockback(int knockback);
	virtual bool isAttackable();
	void setCritArrow(bool critArrow);
	bool isCritArrow();

	static const int TYPE_NORMAL = 0;
	static const int TYPE_SPECTRAL = 1;
	static const int TYPE_TIPPED = 2;
	static const int SPECTRAL_GLOW_DURATION = 200;

	void setArrowType(int type, int potionValue = 0);
	int getArrowType();
	int getPotionValue();
	void setFromItem(shared_ptr<ItemInstance> item);
	shared_ptr<ItemInstance> getPickupItem();
	static bool isArrowItem(int itemId);
	void setCustomEffect(int effectId, int duration, int amplifier);

private:
	void applyArrowEffects(shared_ptr<LivingEntity> mob);
	int inGroundTime;
	int customEffectId;
	int customEffectDuration;
	int customEffectAmplifier;

	// neoLegacy Added
	void setExplosiveLevel(int level);
	int getExplosiveLevel();

	// neoLegacy Added - crossbow enchantments
	void setPierceLevel(int level) { pierceLevel = level; }
	int getPierceLevel() { return pierceLevel; }
	void setTorchArrow(bool torch) { torchArrow = torch; }
	bool isTorchArrow() { return torchArrow; }
	void setLootingLevel(int level) { lootingLevel = level; }
	int getLootingLevel() { return lootingLevel; }

private:
	void landTorch(int hitX, int hitY, int hitZ, int face);	// a torch arrow hit a block: plant the torch, or drop it if it cannot stand there
};