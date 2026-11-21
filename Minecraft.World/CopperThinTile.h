#pragma once
#include "Tile.h"
class IconRegister;

// neoLegacy - Copper Age (playtest fix, 2026-11-20). Thin non-opaque column
// with custom shape bounds. Icon logic is CopperTile's (bits 0-1
// oxidation, bit 2 waxed).
// COPPER_MODELS_RENDERER (2026-11-21): the lantern and rod are no longer
// rendered as SHAPE_BLOCK single boxes -- they have their own shapes
// (SHAPE_COPPER_LANTERN / SHAPE_LIGHTNING_ROD) and Java-style two-box
// models in TileRenderer. The chain stays SHAPE_BLOCK (single thin box).
class CopperThinTile : public Tile
{
private:
	wstring baseIconName;
	Icon *stateIcons[4];
	float sx0, sy0, sz0, sx1, sy1, sz1;
public:
	CopperThinTile(int id, const wstring &baseIconName,
		float x0, float y0, float z0, float x1, float y1, float z1);
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual bool isSolidRender(bool isServerLevel = false) override;
	virtual bool isCubeShaped() override;
	virtual int getRenderShape() override;
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()) override;
	virtual void updateDefaultShape() override;
protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
