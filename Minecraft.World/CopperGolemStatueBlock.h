#pragma once

#include "BaseEntityTile.h"
#include "Material.h"

class IconRegister;
class Player;
class Random;

// neoLegacy - Copper Age (Phase 5). The copper golem statue: the final,
// oxidized form of a copper golem. Replaces the Phase 3 placeholder (a
// plain CopperTile cube) on the EXISTING Tile 2320 — no new Tile ID.
//
// Data layout (must preserve Phase 4: bits 0-2 are oxidation/waxed, and
// Level::advanceCopperOxidation + the waxing recipes already handle the
// statue with that layout):
//   bits 0-1  oxidation 0=unaffected, 1=exposed, 2=weathered, 3=oxidized
//   bit 2     waxed (+4); waxed reuses the unwaxed texture
//   bits 3-4  pose 0=STANDING, 1=SITTING, 2=RUNNING, 3=STAR
//   bits 5-6  facing 0=NORTH, 1=EAST, 2=SOUTH, 3=WEST (ChestTile dir
//             convention: (floor(yRot*4/360+0.5))&3)
// Waterlogging is CUT: this engine has no waterlogging.
// The statue has no item form and no recipe (Phase 3): it only arises
// when an OXIDIZED golem turns to statue. Breaking it drops nothing and
// does NOT spawn a golem (see removeStatue on the block entity — the
// method exists, but no trigger is wired; that is an open decision).
class CopperGolemStatueBlock : public BaseEntityTile
{
	friend class Tile;

public:
	static const int POSE_STANDING = 0;
	static const int POSE_SITTING = 1;
	static const int POSE_RUNNING = 2;
	static const int POSE_STAR = 3;

	static const int FACING_NORTH = 0;
	static const int FACING_EAST = 1;
	static const int FACING_SOUTH = 2;
	static const int FACING_WEST = 3;

	// Data helpers (shared with the golem's turnToStatue and the renderer).
	static int makeData(int oxidation, int pose, int facing);
	static int getOxidation(int data);
	static int getPose(int data);
	static int getFacing(int data);
	// ChestTile dir convention -> yaw in degrees (0 = south).
	static float facingToYaw(int facing);

protected:
	CopperGolemStatueBlock(int id);

private:
	Icon *stateIcons[4];

public:
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()) override;
	virtual void updateDefaultShape() override;
	virtual AABB *getTileAABB(Level *level, int x, int y, int z) override;
	virtual AABB *getAABB(Level *level, int x, int y, int z) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false) override;
	virtual bool hasAnalogOutputSignal() override;
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir) override;
	virtual int getResourceCount(Random *random) override;
};
