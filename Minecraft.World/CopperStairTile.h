#pragma once
#include "StairTile.h"
#include "EntityTile.h"

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Cut-copper
// stairs.
//
// StairTile is NOT an entity tile, so CopperStairTile extends both
// StairTile and EntityTile and sets _isEntityTile itself (the
// BaseEntityTile pattern, without the BaseEntityTile base — a stair
// cannot extend two Tile subclasses). Its oxidation field (0-7) lives
// in a CopperOxidationTileEntity, because the 4-bit data nibble cannot
// hold oxidation alongside facing (bits 0-1) + upside-down (bit 2).
// The nibble now holds structural bits only (dir | UPSIDEDOWN_BIT);
// the item form's aux IS the oxidation field. setPlacedBy is
// overridden because the base StairTile version rewrites the data as
// DIR | UPSIDEDOWN.
class CopperStairTile : public StairTile, public EntityTile
{
public:
	CopperStairTile(int id);

	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel) override;
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) reads the TE oxidation so the picked stair item
	// matches the placed stair.
	virtual int cloneTileData(Level *level, int x, int y, int z) override;
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
