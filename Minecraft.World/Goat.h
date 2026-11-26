#pragma once

using namespace std;

#include "Animal.h"

class Player;
class Level;
class CompoundTag;

// neoLegacy - Goat (from modern Minecraft). A passive, breedable hill-dweller:
//   * spawns in Extreme Hills biomes (grass, stone or gravel underfoot),
//   * sure-footed: normal hops clear 2 blocks, it randomly springs ~4-5 blocks into the air
//     (GoatLeapGoal), and it takes no fall damage under ~10 blocks,
//   * randomly headbutts nearby players and mobs (GoatRamGoal),
//   * can be milked with a bucket (like a cow), bred with wheat, and drops nothing but XP,
//   * loses a horn (up to two) when a ram slams it into a solid block — the horn drops as an
//     item (GoatRamGoal block-hit detection; see dropHorn()).
// Its sounds are the Java goat recordings (mob/goat/*: say/hurt/death/jump/ram_impact/step/eat/
// pre_ram/horn_break) plus the goat-horn calls (item/goat_horn/call1-8); the step sound used to
// reuse the sheep step before the goat recordings landed.
class Goat : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_GOAT; }
	static Entity *create(Level *level) { return new Goat(level); }

private:
	static const int DATA_RAMMING = 17;		// synched flag: head lowered / charging (drives the model pose)
	int hornsLeft;		// horns remaining to lose (starts at 2, one per horn); persisted in save data

public:
	Goat(Level *level);
	bool isRamming();
	void setRamming(bool ramming);
	bool dropHorn();		// drop one horn as an item if any remain; false if none left (or a baby)
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
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
