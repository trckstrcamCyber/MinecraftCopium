#pragma once
#include "BaseEntityTile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). The copper shelf (Java shelf block).
// One tile id; aux = wood index 0-6 (same global order as
// StrippedLogTile: OAK, SPRUCE, BIRCH, JUNGLE, ACACIA, DARK_OAK, BAMBOO).
// The shelf has its OWN per-wood texture in jar1201 (32x32, downscaled to
// 16x16 for the atlas) — it does NOT reuse the wood/log textures.
//
// Phase 6: storage + redstone output. The shelf is a BaseEntityTile
// with a 3-slot ShelfTileEntity. The wood aux lives in data bits 0-2,
// so getSpawnResourcesAuxValue (data & 7) is unchanged.
// Round-5 fix (2026-11-20): the facing lives in the ShelfTileEntity,
// NOT in data bits 3-5 — block data is a 4-bit nibble, so the Phase 6
// "(facing << 3)" write (16-40) could never be read back and every
// shelf rendered with the NORTH shape. See ShelfTile::updateShape.
// use() swaps one item with the slot under the click (thirds along
// the front face); empty hand takes the item. Redstone output is the
// container-fullness signal (chest precedent).
// Display rendering is NOT in Phase 6 (see the report): no renderer
// in this tree draws item icons on a block face yet.
class ShelfTile : public BaseEntityTile
{
private:
	Icon *woodIcons[7];

public:
	ShelfTile(int id);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false) override;
	virtual bool hasAnalogOutputSignal() override;
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir) override;
	virtual bool isSolidRender(bool isServerLevel = false) override;
	virtual bool isCubeShaped() override;
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>()) override;
	virtual void updateDefaultShape() override;
	virtual AABB *getTileAABB(Level *level, int x, int y, int z) override;
	virtual AABB *getAABB(Level *level, int x, int y, int z) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
