#pragma once
using namespace std;

#include "Animal.h"

class Player;
class Level;

// neoLegacy - Camel (modern Minecraft's desert mount).
//   * 32 health, tall (1.7 x 2.375) and steps up 1.5 blocks; exactly one lives at the centre of each
//     desert village (it never respawns),
//   * bred with cactus; babies follow their parent,
//   * right-click with a saddle to saddle it, then right-click to ride: no taming needed. Steering is
//     like a horse. Press jump while riding to DASH: it launches ~12 blocks forward and 1 block up, then
//     needs 2.75 seconds to recover,
//   * sits down now and then when it is idle, and won't move while sitting.
// Two players can ride at once: the first to mount drives (front seat), the second sits behind. (The
// engine only tracks one rider per mount, so Camel keeps its own seat list: every player whose "riding" is
// this camel is a passenger, and the driver's entity id is synched.)
class Camel : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_CAMEL; }
	static Entity *create(Level *level) { return new Camel(level); }

private:
	static const int DATA_FLAGS = 17;			// bit 0 = saddled, bit 1 = sitting
	static const int DATA_DRIVER = 18;			// entity id of the driving player, -1 = none
	static const int FLAG_SADDLED = 1;
	static const int FLAG_SITTING = 2;

	int sitTicksLeft;
	bool wasJumping;			// previous tick's jump key, to detect a fresh press
	int dashCooldown;

public:
	Camel(Level *level);

	bool isSaddled();
	void setSaddled(bool saddled);
	bool isSitting();
	void setSitting(bool sitting);

	virtual bool useNewAi();
	virtual bool canSpawn();
	virtual bool isFood(shared_ptr<ItemInstance> item);
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual void travel(float xa, float ya);
	virtual void aiStep();
	virtual double getRideHeight();
	virtual void die(DamageSource *source);
	virtual void positionRider();

	// Seats
	int countRiders();
	shared_ptr<Entity> getDriver();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual void defineSynchedData();
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual float getVoicePitch();

private:
	void collectRiders(vector<shared_ptr<Entity> > &out);
	void setFlag(int flag, bool value);
	bool getFlag(int flag);
};
