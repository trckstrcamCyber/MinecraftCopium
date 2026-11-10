#pragma once
#include "HalfSlabTile.h"
#include "EntityTile.h"

class TileEntity;
class Player;
class LivingEntity;
class ItemInstance;

// neoLegacy - Concrete slabs, all 16 colors on ONE half-slab ID (+ one full-slab ID).
//
// Same idea as ConcreteStairTile: the block data stays exactly what HalfSlabTile expects
// (TOP_SLOT_BIT for upper/lower half) and the 0-15 color lives in a
// ConcreteColorTileEntity. A half slab and a full slab still need separate tile IDs
// (the engine's "is this a solid full cube" flag is per-ID, not per-block), so a slab is
// 2 IDs total for all colors -- down from 4, and the stairs are down from 16 to 1.
//
// Item aux value = color 0-15 (same global order as Tile::concrete).
class ConcreteSlabTile : public HalfSlabTile, public EntityTile
{
public:
	static const unsigned int SLAB_NAMES[16];	// indexed by the 0-15 color

private:
	int resourceTileId;	// the HALF slab id that breaking either half or full slab drops
	int m_dropColor;	// captured in playerWillDestroy, before the tile entity is removed

public:
	ConcreteSlabTile(int id, int resourceTileId);

	virtual int isFullSize() = 0;

	// EntityTile
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);

	// World: color comes from the tile entity
	virtual Icon        *getTexture(LevelSource *level, int x, int y, int z, int face);
	// Inventory / hand / dropped item: "data" here is the item aux value = color
	virtual Icon        *getTexture(int face, int data);
	virtual void         registerIcons(IconRegister *iconRegister);
	virtual int          getResource(int data, Random *random, int playerBonusLevel);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual int          getAuxName(int auxValue);

	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	using HalfSlabTile::spawnResources;
	virtual int cloneTileData(Level *level, int x, int y, int z);
};

class HalfConcreteSlabTile : public ConcreteSlabTile
{
public:
	HalfConcreteSlabTile(int id, int resourceTileId) : ConcreteSlabTile(id, resourceTileId) { DerivedInit(); }
	virtual int isFullSize() override { return 0; }
};

class FullConcreteSlabTile : public ConcreteSlabTile
{
public:
	FullConcreteSlabTile(int id, int resourceTileId) : ConcreteSlabTile(id, resourceTileId) { DerivedInit(); }
	virtual int isFullSize() override { return 1; }
};
