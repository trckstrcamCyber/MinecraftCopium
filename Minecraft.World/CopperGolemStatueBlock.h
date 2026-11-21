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
// Data layout (oxidation workaround, 2026-11-20): the 4-bit nibble
// holds oxidation ONLY — bits 0-1 oxidation, bit 2 waxed — which is
// what Level::advanceCopperOxidation and the waxing recipes already
// handle. Pose and facing used to be packed into the same nibble
// (pose bits 3-4, facing bits 5-6), but the SparseDataStorage clamp
// truncates the 6-bit value to 15, so pose was 0/1 only, facing was
// always NORTH, and neither persisted. Pose/facing now live in the
// CopperGolemStatueBlockEntity ("Pose"/"Facing" NBT, 0-3 each):
//   pose    0=STANDING, 1=SITTING, 2=RUNNING, 3=STAR
//   facing  0=NORTH, 1=EAST, 2=SOUTH, 3=WEST (ChestTile dir
//           convention: (floor(yRot*4/360+0.5))&3)
// Waterlogging is CUT: this engine has no waterlogging.
// The statue arises when an OXIDIZED golem turns to statue. Breaking it
// drops the statue as an item (audit fix, Java behavior); it does NOT
// spawn a golem on break. An axe on an unoxidized, unwaxed statue
// reanimates it into a golem (see use() + removeStatue on the block
// entity) — only after the wax and all oxidation have been scraped off.
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

	// Data helpers. getOxidation reads the nibble (bits 0-1), which is
	// still where oxidation lives. makeData / getPose / getFacing are
	// LEGACY nibble decoders of the pre-workaround 6-bit packed layout
	// (pose bits 3-4, facing bits 5-6): the nibble no longer stores
	// pose/facing, so they decode stale/zero bits on live data and must
	// NOT be used for live reads — read pose/facing from the
	// CopperGolemStatueBlockEntity instead. They are kept (unchanged
	// signatures) so any out-of-tree caller still compiles.
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
	virtual bool isSolidRender(bool isServerLevel = false) override;
	virtual bool isCubeShaped() override;
	virtual int getRenderShape() override;
	virtual AABB *getTileAABB(Level *level, int x, int y, int z) override;
	virtual AABB *getAABB(Level *level, int x, int y, int z) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false) override;
	virtual bool hasAnalogOutputSignal() override;
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir) override;
	virtual int getResourceCount(Random *random) override;
	virtual int getSpawnResourcesAuxValue(int data) override;
};

