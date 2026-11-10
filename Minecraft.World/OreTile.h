#pragma once
#include "Tile.h"

class Random;

class OreTile : public Tile
{
public:

	OreTile(int id);
    virtual int getResource(int data, Random *random, int playerBonusLevel);

	// neoLegacy Added - Auto Smelt (iron/gold ore are the only OreTile
	// variants that still drop their own raw block from getResource(), so
	// they're the only ones with anything meaningful to smelt)
	virtual int getSmeltedResource(int data);
    virtual int getResourceCount(Random *random);
	virtual int getResourceCountForLootBonus(int bonusLevel, Random *random);
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
protected:
	virtual int getSpawnResourcesAuxValue(int data);
};