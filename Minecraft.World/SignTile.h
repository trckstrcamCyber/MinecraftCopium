#pragma once

#include "BaseEntityTile.h"
#include "TileEntity.h"

#include "stdafx.h"
#include "Material.h"

class SignTile : public BaseEntityTile
{
	friend class Tile;
private:
	eINSTANCEOF clas;
	bool onGround;
	int woodType;	// neoLegacy - 0 oak, 1 spruce, 2 birch, 3 jungle, 4 acacia, 5 dark oak

protected:
	SignTile(int id, eINSTANCEOF clas, bool onGround, int woodType = 0);

public:
	bool isStanding() const { return onGround; }
	int getWoodType() const { return woodType; }

public:
	Icon *getTexture(int face, int data);
    virtual void updateDefaultShape();
	AABB *getAABB(Level *level, int x, int y, int z);
	AABB *getTileAABB(Level *level, int x, int y, int z);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	int getRenderShape();
	bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
	bool isSolidRender(bool isServerLevel = false);

protected:
	shared_ptr<TileEntity> newTileEntity(Level *level);

public:
	int getResource(int data, Random *random, int playerBonusLevel);
	using BaseEntityTile::spawnResources;
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);	// neoLegacy - the dropped sign keeps its wood
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};