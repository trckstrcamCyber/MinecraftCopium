#pragma once

using namespace std;

#include "Animal.h"

class Player;
class Level;

// neoLegacy - Goat (from modern Minecraft). A passive, breedable hill-dweller:
//   * spawns in Extreme Hills biomes (grass, stone or gravel underfoot),
//   * sure-footed: normal hops clear 2 blocks, it randomly springs ~4-5 blocks into the air
//     (GoatLeapGoal), and it takes no fall damage under ~10 blocks,
//   * randomly headbutts nearby players and mobs (GoatRamGoal),
//   * can be milked with a bucket (like a cow), bred with wheat, and drops nothing but XP.
// Its bleats reuse the sheep sounds at a higher pitch (no new audio assets needed).
class Goat : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_GOAT; }
	static Entity *create(Level *level) { return new Goat(level); }

private:
	static const int DATA_RAMMING = 17;		// synched flag: head lowered / charging (drives the model pose)

public:
	Goat(Level *level);
	bool isRamming();
	void setRamming(bool ramming);
	virtual bool useNewAi();
	virtual bool canSpawn();
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual void causeFallDamage(float distance);

protected:
	virtual void defineSynchedData();
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual float getVoicePitch();
	virtual void jumpFromGround();
};
