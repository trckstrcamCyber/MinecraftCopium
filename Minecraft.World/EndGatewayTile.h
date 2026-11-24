#pragma once
#include "BaseEntityTile.h"

class EndGatewayTile : public BaseEntityTile
{
public:
	EndGatewayTile(int id, Material *material);

	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getResourceCount(Random *random);
	virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	virtual int getRenderShape();
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
};
