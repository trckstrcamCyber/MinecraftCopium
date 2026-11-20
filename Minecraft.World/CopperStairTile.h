#pragma once
#include "StairTile.h"

// neoLegacy - Copper Age (Phase 3). Cut-copper stairs.
//
// The stairs' data layout is the vanilla one (StairTile): bits 0-1 facing,
// bit 2 (UPSIDEDOWN_BIT) upside-down. Oxidation lives in bits 3-5, which
// no vanilla stair code touches, so the texture is the cut-copper texture
// at (data >> 3) & 7 and the dropped / picked item's aux is the oxidation
// state. setPlacedBy is overridden because the base StairTile version
// rewrites the data as DIR | UPSIDEDOWN and would wipe the oxidation bits.
class CopperStairTile : public StairTile
{
public:
	CopperStairTile(int id);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
