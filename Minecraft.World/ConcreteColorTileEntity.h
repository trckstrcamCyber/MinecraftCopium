#pragma once
using namespace std;

#include "TileEntity.h"

class Level;
class LevelSource;

// neoLegacy - Concrete color storage for concrete stairs / slabs.
//
// A tile ID only has 4 bits of metadata. Concrete needs all 4 of them for its 16 colors,
// but stairs need 3 bits for orientation and slabs need 1 for top/bottom, so a stair or
// slab can't hold its color in the block data and still share one ID across all colors.
// Instead the orientation / half stays in the normal block data (so StairTile and
// HalfSlabTile keep working untouched) and the 0-15 color lives here, same idea as
// BannerTileEntity's base color.
class ConcreteColorTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_CONCRETECOLORTILEENTITY; }
	static TileEntity *create() { return new ConcreteColorTileEntity(); }

private:
	int color;

public:
	ConcreteColorTileEntity();
	virtual ~ConcreteColorTileEntity() {}

	int getColor() const { return color; }
	void setColor(int c) { color = c & 15; }

	// Convenience for the tiles/items: color of the concrete stair/slab at a position,
	// or -1 if there's no ConcreteColorTileEntity there (yet). setColorAt() also
	// marks the tile entity changed so it is saved and re-sent to clients.
	static int getColorAt(LevelSource *level, int x, int y, int z);
	static void setColorAt(Level *level, int x, int y, int z, int color);

	virtual void save(CompoundTag *tag);
	virtual void load(CompoundTag *tag);
	virtual shared_ptr<Packet> getUpdatePacket();
	virtual shared_ptr<TileEntity> clone();
};
