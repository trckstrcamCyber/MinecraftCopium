#pragma once
#include "HalfSlabTile.h"

class TileEntity;
class Player;
class LivingEntity;
class ItemInstance;

// neoLegacy - Copper Age (Phase 3). Cut-copper slab, half + full.
//
// Same idea as ConcreteSlabTile (a slab is 2 tile ids: half + full) but
// WITHOUT a tile entity: the oxidation state lives in the data bits
// (HalfSlabTile::TYPE_MASK, bits 0-2), exactly like the stone slab's type
// field, and the texture is delegated to the cut-copper tile at that
// oxidation state. The dropped / picked item's aux IS the oxidation state
// (HalfSlabTile::getSpawnResourcesAuxValue returns data & TYPE_MASK).
// Item aux value = oxidation 0-7 (0 unaffected, 1 exposed, 2 weathered,
// 3 oxidized, 4-7 waxed; waxed reuses the unwaxed texture).
class CopperSlabTile : public HalfSlabTile
{
private:
	int resourceTileId;	// the HALF slab id that breaking either half or full slab drops

public:
	CopperSlabTile(int id, int resourceTileId);

	virtual int isFullSize() = 0;

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual int getResource(int data, Random *random, int playerBonusLevel) override;
	virtual unsigned int getDescriptionId(int iData = -1) override;
	virtual int getAuxName(int auxValue) override;
	// Per-oxidation display names (2026-11-23), Java 26.3. Index = aux 0-7.
	static const unsigned int SLAB_NAMES[8];
};

class HalfCopperSlabTile : public CopperSlabTile
{
public:
	HalfCopperSlabTile(int id, int resourceTileId) : CopperSlabTile(id, resourceTileId) { DerivedInit(); }
	virtual int isFullSize() override { return 0; }
};

class FullCopperSlabTile : public CopperSlabTile
{
public:
	FullCopperSlabTile(int id, int resourceTileId) : CopperSlabTile(id, resourceTileId) { DerivedInit(); }
	virtual int isFullSize() override { return 1; }
};
