#pragma once
#include "BaseEntityTile.h"
#include "Material.h"

class Player;
class Random;

class ChestTile : public BaseEntityTile
{
	friend class Tile;
	friend class Minecraft;

public:
	static const int TYPE_BASIC = 0;
	static const int TYPE_TRAP = 1;
	static const int TYPE_COPPER = 2;	// neoLegacy - Copper Age (Phase 3)

	static const int EVENT_SET_OPEN_COUNT = 1;

private:
	Random *random;

public:
	int type;

protected:
	ChestTile(int id, int type);
	~ChestTile();

public:
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	void recalcLockDir(Level *level, int x, int y, int z);
	virtual bool mayPlace(Level *level, int x, int y, int z);

private:
	bool isFullChest(Level *level, int x, int y, int z);

public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	// neoLegacy - Copper Age (review fix, 2026-11-20). Snapshot the
	// oxidation before the TE is removed, so spawnResources can still
	// drop the right aux (see ChestTile.cpp).
	virtual void onRemoving(Level *level, int x, int y, int z, int data) override;
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual shared_ptr<Container> getContainer(Level *level, int x, int y, int z);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool isSignalSource();
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);

private:
	bool isCatSittingOnChest(Level *level, int x, int y, int z);

public:
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);

protected:
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Legacy
	// nibble fallback only: the oxidation lives in the ChestTileEntity
	// now, so this can only see stale bits 3-5 on old saves (at most
	// bit 3 — the nibble is 4 bits). Drops and pick-block go through
	// the spawnResources / cloneTileData overrides below, which read
	// the TE.
	virtual int getSpawnResourcesAuxValue(int data) override;
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The
	// oxidation lives in the ChestTileEntity now, not in data bits 3-5,
	// so the drop / pick-block aux must be read from the TE.
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel) override;
	virtual int cloneTileData(Level *level, int x, int y, int z) override;
	virtual void registerIcons(IconRegister *iconRegister);
};