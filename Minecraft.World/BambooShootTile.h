#pragma once
#include "Bush.h"

class IconRegister;
class Random;

// neoLegacy - the Bamboo shoot: what a Bamboo item becomes when planted on soil. Grows into a Bamboo plant.
// Material is "plant", so flowing water washes it away (a grown stalk survives water).
class BambooShootTile : public Bush
{
private:
	Icon *shootIcon;

public:
	BambooShootTile(int id);

	virtual void registerIcons(IconRegister *iconRegister);
	virtual Icon *getTexture(int face, int data);
	virtual bool canSurvive(Level *level, int x, int y, int z);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual void tick(Level *level, int x, int y, int z, Random *random);

protected:
	virtual bool mayPlaceOn(int tile);
};
