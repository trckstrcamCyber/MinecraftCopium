#pragma once

#include "StairTile.h"
#include "EntityTile.h"

class TileEntity;
class Player;
class LivingEntity;
class ItemInstance;

// neoLegacy - Concrete stairs, ONE tile ID for all 16 colors.
//
// Orientation / upside-down stays in the normal 3 data bits (StairTile is untouched).
// The color can't also fit in the remaining 4th bit, so it lives in a
// ConcreteColorTileEntity (see that header). The item form carries the color as its aux
// value (0-15, same global order as Tile::concrete).
class ConcreteStairTile : public StairTile, public EntityTile
{
private:
	int m_dropColor;	// captured in playerWillDestroy, before the tile entity is removed

public:
	ConcreteStairTile(int id);

	// EntityTile
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);

	// World: color comes from the tile entity (block data is orientation only)
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face);
	// Inventory / hand / dropped item: "data" here is the item aux value = color
	virtual Icon *getTexture(int face, int data);

	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	using StairTile::spawnResources;
	virtual int cloneTileData(Level *level, int x, int y, int z);
};
