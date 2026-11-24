#pragma once
#include "Tile.h"

class Random;

class ChorusPlantTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
protected:
	ChorusPlantTile(int id);
public:
	static const float CORE_MIN;
	static const float CORE_MAX;

	bool connectsTo(LevelSource *level, int x, int y, int z, int face);

	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void updateDefaultShape();
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCount(Random *random);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual bool canSurvive(Level *level, int x, int y, int z);
};
