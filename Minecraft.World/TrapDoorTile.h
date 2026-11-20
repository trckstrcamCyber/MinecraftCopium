#pragma once

#include "Tile.h"

class Player;
class HitResult;

class TrapDoorTile : public Tile
{
	friend class Tile;
private:
	static const int TOP_MASK = 0x8;

	// neoLegacy - Copper Age (Phase 3). Per-oxidation-state icons for the
	// copper trapdoor (oxidation in bits 4-6 of the data; the use() toggle
	// XORs bit 2 only, so it survives opening/closing). Only the copper
	// trapdoor registers all four states; other trapdoors use state 0.
	Icon *stateIcons[4];

protected:
	TrapDoorTile(int id, Material *material);

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