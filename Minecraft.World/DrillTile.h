#pragma once

#include "Tile.h"

// neoLegacy - Drill.
//
// A redstone-powered block breaker. While it receives a redstone signal it mines the block
// directly in front of its face (the drill_face side), at exactly the speed a player holding
// an IRON pickaxe would mine it, and it can only mine what an iron pickaxe can harvest (so
// stone, ores, iron, gold, diamond, redstone... but not obsidian). The mining itself lives in
// DrillTileEntity, which ticks every game tick and keeps the progress between ticks.
//
// Data layout:
//   bits 0-2 : facing, one of Facing::{DOWN,UP,NORTH,SOUTH,WEST,EAST} -- the face points at
//              the block being drilled. Set on placement exactly like the Observer's.
//   bit  3   : unused
//
// Textures are the Observer's (top / side / back) except the front, which is "drill_face"
// (already registered in PreStitchedTextureMap at row 27, col 9).
class DrillTile : public Tile
{
public:
	static const int FACING_MASK = 0x7;

	DrillTile(int id);

	virtual bool isCubeShaped() override { return true; }
	virtual bool isSolidRender(bool isServerLevel = false) override { return true; }

	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;

	virtual Icon *getTexture(int face, int data) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

	// neoLegacy - the drill is a plain block now (no tile entity), so pistons can push and pull it. It mines with scheduled ticks:
	// powered -> tick every game tick; the break progress lives in a small table keyed by position (see DrillTile.cpp).
	virtual void onPlace(Level *level, int x, int y, int z) override;
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;
	virtual void neighborChanged(Level *level, int x, int y, int z, int type) override;
	virtual void tick(Level *level, int x, int y, int z, Random *random) override;

	static int getFacing(int data) { return data & FACING_MASK; }

private:
	void mine(Level *level, int x, int y, int z);
	Icon *frontIcon;
	Icon *backIcon;
	Icon *sideIcon;
	Icon *topIcon;
};
