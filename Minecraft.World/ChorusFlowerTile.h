#pragma once
#include "Tile.h"

class Random;

class ChorusFlowerTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
protected:
	ChorusFlowerTile(int id);
public:
	static const int DEAD_AGE = 5;

private:
	Icon *iconDead;
	Icon *iconPlant;

	static bool allNeighborsEmpty(Level *level, int x, int y, int z, int excludedFace);
	void placeGrownFlower(Level *level, int x, int y, int z, int age);
	void placeDeadFlower(Level *level, int x, int y, int z);
	static void growTreeRecursive(Level *level, int x, int y, int z, Random *random, int originX, int originZ, int radius, int depth);

public:
	static bool isDead(int age) { return age >= DEAD_AGE; }
	static void generatePlant(Level *level, int x, int y, int z, Random *random, int radius);

	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual bool canSurvive(Level *level, int x, int y, int z);
	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
	virtual int getSpawnResourcesAuxValue(int data);
	virtual Icon *getTexture(int face, int data);
	virtual void registerIcons(IconRegister *iconRegister);

protected:
	virtual bool isSilkTouchable();
};
