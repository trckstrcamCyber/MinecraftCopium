#pragma once

#include "Tile.h"
#include "BaseEntityTile.h"

class Player;
class HitResult;

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The copper
// trapdoor is an entity tile: its oxidation field (0-7) lives in a
// CopperOxidationTileEntity, because the 4-bit data nibble cannot hold
// oxidation alongside dir/open/top. The nibble now holds structural bits
// only (dir 0-3, open bit 2, top bit 3). Other trapdoors get the same
// (unused, oxidation-0) TE.
class TrapDoorTile : public BaseEntityTile
{
	friend class Tile;
private:
	static const int TOP_MASK = 0x8;

	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20).
	// Per-oxidation-state icons for the copper trapdoor. The oxidation
	// field lives in the CopperOxidationTileEntity (see class comment);
	// the item form's aux IS the field, so getTexture(int, int) indexes
	// it directly, and the in-world form reads the TE. Only the copper
	// trapdoor registers all four states; other trapdoors use state 0.
	Icon *stateIcons[4];

protected:
	TrapDoorTile(int id, Material *material);

public:
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;

/*
 * public int getTexture(int face, int data) { if (face == 0 || face == 1)
 * return tex; int dir = getDir(data); if ((dir == 0 || dir == 2) ^ (face <= 3))
 * { return tex; } int tt = (dir / 2 + ((face & 1) ^ dir)); tt += ((data & 4) /
 * 4); int texture = tex - (data & 8) * 2; if ((tt & 1) != 0) { texture =
 * -texture; } // if (getDir(data)==0 // tt-=((face+data&3)&1)^((data&4)>>2);
 * return texture; }
 */

public:
	bool blocksLight();
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Drops
	// the trapdoor item with the TE's oxidation aux (the base
	// spawnResources only knows the nibble data, which no longer holds
	// oxidation). The TE is usually already gone when this runs, so it
	// falls back to the oxidation onRemoving snapshotted just before
	// removal. Copper only; other trapdoors use the base behaviour.
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel) override;
	// neoLegacy - Copper Age (review fix, 2026-11-20). Snapshot the
	// oxidation before the TE is removed, so spawnResources can still
	// drop the right aux (see TrapDoorTile.cpp).
	virtual void onRemoving(Level *level, int x, int y, int z, int data) override;
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) reads the TE oxidation so the picked trapdoor item
	// matches the placed trapdoor.
	virtual int cloneTileData(Level *level, int x, int y, int z) override;
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual void createBlockStateDefinition() override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
	virtual int defaultBlockState() override;
	virtual int convertBlockStateToLegacyData(BlockState *state) override;
	virtual Tile::BlockState getBlockState(LevelSource *level, int x, int y, int z) override;
	virtual Tile::BlockState getBlockState(int data);

public:
	bool isSolidRender(bool isServerLevel = false);

public:
	bool isCubeShaped();
	bool isPathfindable(LevelSource *level, int x, int y, int z);

public:
	int getRenderShape();

public:
	AABB *getTileAABB(Level *level, int x, int y, int z);

public:
	AABB *getAABB(Level *level, int x, int y, int z);

public:
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param

public:
	void updateDefaultShape();

public:
	using Tile::setShape;
	void setShape(int data);

public:
	void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);

public:
	virtual bool TestUse();
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param

public:
	void setOpen(Level *level, int x, int y, int z, bool shouldOpen);


public:
	void neighborChanged(Level *level, int x, int y, int z, int type);


public:
	HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);

public:
	int getDir(int dir);

public:
	int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);

public:
	bool mayPlace(Level *level, int x, int y, int z, int face);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;

public:
	static bool isOpen(int data);

private:
	static bool attachesTo(int id);
};