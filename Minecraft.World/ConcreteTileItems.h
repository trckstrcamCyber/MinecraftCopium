#pragma once
using namespace std;

#include "TileItem.h"
#include "HalfSlabTile.h"

// neoLegacy - Item forms of the single-ID concrete stairs / slabs.
// Aux value = color 0-15 (same global order as Tile::concrete). The aux value is NOT
// passed through as block data (getLevelDataForAuxValue returns 0) because the block data
// is orientation / half only -- the color goes into the ConcreteColorTileEntity instead,
// set by the tile's setPlacedBy().

class ConcreteStairTileItem : public TileItem
{
public:
	static const unsigned int STAIRS_NAMES[16];

	ConcreteStairTileItem(int id);

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};

class ConcreteSlabTileItem : public TileItem
{
private:
	bool isFull;
	HalfSlabTile *halfTile;
	HalfSlabTile *fullTile;

public:
	ConcreteSlabTileItem(int id, HalfSlabTile *halfTile, HalfSlabTile *fullTile, bool full);

	virtual Icon *getIcon(int itemAuxValue);
	virtual int getLevelDataForAuxValue(int auxValue);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual bool mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item);

private:
	bool tryConvertTargetTile(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, bool bTestUseOnOnly);
	void convertToFull(shared_ptr<ItemInstance> instance, Level *level, int x, int y, int z, int color);
};
