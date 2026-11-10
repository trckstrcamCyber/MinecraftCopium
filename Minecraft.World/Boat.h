#pragma once
using namespace std;

#include "Entity.h"

class Player;
class Level;
class CompoundTag;
class DamageSource;

class Boat : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_BOAT; };
	static Entity *create(Level *level) { return new Boat(level); }

private:
	// 4J - added for common ctor code
	void _init();
public:
	static const int serialVersionUID = 0;

private:
	static const int DATA_ID_HURT = 17;
	static const int DATA_ID_HURTDIR = 18;
	static const int DATA_ID_DAMAGE = 19;
	static const int DATA_ID_DRIVER = 21;	// neoLegacy - entity id of the player steering the boat (front seat), -1 = nobody
	static const int MAX_RIDERS = 2;		// neoLegacy - two-seat boats like Java Edition
	static const int DATA_ID_WOOD = 20;		// neoLegacy - wood type (WoodTile data: 0 oak,1 spruce,2 birch,3 jungle,4 acacia,5 dark oak, 6 bamboo = the raft)
	static const double MAX_SPEED;
	static const double MAX_COLLISION_SPEED;
	static const double MIN_ACCELERATION;
	static const double MAX_ACCELERATION;

	bool doLerp;
	double acceleration;

public:
	Boat(Level *level);

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	virtual AABB *getCollideAgainstBox(shared_ptr<Entity> entity);
	virtual AABB *getCollideBox();
	virtual bool isPushable();

	Boat(Level *level, double x, double y, double z);

	// neoLegacy - each boat is built from one kind of planks and keeps its own texture
	int getWoodType();
	void setWoodType(int woodType);

	// neoLegacy - seats. Entity only remembers ONE rider (the weak "rider" pointer holds whoever mounted last), so the
	// boat keeps its own seat list exactly like Camel does: every player whose "riding" is this boat is a passenger,
	// and the driver's entity id is synched. The first player aboard steers; the second sits behind them.
	int countRiders();
	shared_ptr<Entity> getDriver();

	// neoLegacy - paddle animation clock (client visual only): advances while the boat is being driven
	float rowingTime, rowingTimeO;	// radians
	float paddleBlend, paddleBlendO;	// 0 = resting pose, 1 = full rowing stroke
	double paddleLastX, paddleLastZ;
	bool paddleInit;

	virtual double getRideHeight();
	virtual bool hurt(DamageSource *source, float damage);
	virtual void animateHurt();
	virtual bool isPickable();

private:
	void collectRiders(vector<shared_ptr<Entity> > &out);
	void updateDriverSeat();

	int lSteps;
	double lx, ly, lz, lyr, lxr;
	double lxd, lyd, lzd;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	void createSplash(double particleStrengh);
	virtual void positionRider();

protected:
	virtual void addAdditonalSaveData(CompoundTag *base);
	virtual void readAdditionalSaveData(CompoundTag *base);

public:
	virtual float getShadowHeightOffs();
	wstring getName();
	virtual bool interact(shared_ptr<Player> player);

	virtual void setDamage(float damage);
	virtual float getDamage();
	virtual void setHurtTime(int hurtTime);
	virtual int getHurtTime();
	virtual void setHurtDir(int hurtDir);
	virtual int getHurtDir();

	bool getDoLerp();
	void setDoLerp(bool doLerp);
};
