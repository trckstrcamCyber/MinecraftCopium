#pragma once
#include "Tile.h"
#include "Facing.h"

class Random;

class EndRodTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
protected:
	EndRodTile(int id);
public:
	static int getFacing(int data) { return (data & 7) > 5 ? Facing::UP : (data & 7); }

	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void updateDefaultShape();
	using Tile::setShape;
	void setShapeForData(int data);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
	virtual int getPistonPushReaction();
	virtual int getSpawnResourcesAuxValue(int data);
};
