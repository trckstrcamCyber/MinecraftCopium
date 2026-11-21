#pragma once
#include "Tile.h"
#include "BaseEntityTile.h"
#include "Definitions.h"

class Player;
class HitResult;
class ChunkRebuildData;

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The copper
// door is an entity tile: its oxidation field (0-7) lives in a
// CopperOxidationTileEntity on the LOWER half (the upper half also gets
// one from setTileAndData, but it is ignored), because the 4-bit data
// nibble cannot hold oxidation alongside dir/open/hinge. The nibble now
// holds structural bits only: lower = dir (0-3) [+ open bit 2], upper =
// 8 | hinge bit. Other doors get the same (unused, oxidation-0) TE.
class DoorTile : public BaseEntityTile
{
	friend class Tile;
	friend class ChunkRebuildData;

private:
	static const int TEXTURE_NORMAL = 0;
	static const int TEXTURE_FLIPPED = 1;

public:
	static const int UPPER_BIT = 8;
	static const int C_DIR_MASK = 3;
	static const int C_OPEN_MASK = 4;
	static const int C_LOWER_DATA_MASK = 7;
	static const int C_IS_UPPER_MASK = 8;
	static const int C_RIGHT_HINGE_MASK = 16;
	wstring doorType;

private:
	static const int DOOR_TILE_TEXTURE_COUNT = 4;
	int texBase;
	// neoLegacy - Copper Age (Phase 3). Per-oxidation-state icons:
	// [oxidation 0-3][normal/flipped]. Only the copper door registers all
	// four states; every other door registers state 0 only (as before).
	Icon *iconTop[4][2];
	Icon *iconBottom[4][2];

	// Copper door oxidation lives in the LOWER half's
	// CopperOxidationTileEntity (see class comment). Returns 0 for
	// non-copper doors, or when the TE is missing (old save / not yet
	// created). isUpper selects which half (x, y, z) is: the lower
	// half's TE is authoritative either way.
	int getOxidation(LevelSource *level, int x, int y, int z, bool isUpper);
	// Copper + rose-gold doors are hand-openable (Java), unlike the iron door.
	bool isHandOpenable();

protected:
	DoorTile(int id, Material *material, const wstring& doorType);
public:
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;
	virtual Icon *getTexture(int face, int data);
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual bool blocksLight();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	int getDir(LevelSource *level, int x, int y, int z);
	bool isOpen(LevelSource *level, int x, int y, int z);
private:
	using Tile::setShape;
	virtual void setShape(int compositeData);
public:
	virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	void setOpen(Level *level, int x, int y, int z, bool shouldOpen);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The
	// copper door's oxidation lives in the LOWER half's
	// CopperOxidationTileEntity, but the drop is produced from the LOWER
	// half, so this override reads that TE and drops a door item with
	// the matching oxidation/waxed aux. If the TE is missing (support
	// lost after the half was already removed), falls back to aux 0 —
	// the same residual gap the old nibble code had. Non-copper doors
	// fall through to the base implementation unchanged.
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) reads the TE oxidation so the picked door item
	// matches the placed door.
	virtual int cloneTileData(Level *level, int x, int y, int z) override;
	virtual HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	static bool isOpen(int data);
	virtual int getPistonPushReaction();
	int getCompositeData(LevelSource *level, int x, int y, int z);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);

	virtual void createBlockStateDefinition() override;
	virtual int convertBlockStateToLegacyData(BlockState *state) override;
	virtual int defaultBlockState() override;

	Tile::BlockState getBlockState(LevelSource *level, int x, int y, int z);
	Tile::BlockState getBlockState(int data);

	void fillVirtualBlockStateProperties(Tile::BlockState *state, LevelSource *level, const BlockPos &pos);

	virtual bool use(Level *level, const BlockPos &pos, Tile::BlockState *state, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
};
