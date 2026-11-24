#pragma once
#include "BaseEntityTile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). The copper shelf (Java shelf block).
// One tile id; aux = wood index 0-6 (same global order as
// StrippedLogTile: OAK, SPRUCE, BIRCH, JUNGLE, ACACIA, DARK_OAK, BAMBOO).
// The shelf has its OWN per-wood texture in jar1201 (32x32). The atlas
// holds two 16x16 quadrants per wood: the FRONT icon (top-left quadrant,
// the front view) and the BACK icon (top-right quadrant, the back panel
// and board tops/undersides). The multi-cuboid model (SHAPE_SHELF,
// TileRenderer) samples both; getTexture() still returns the front icon
// for particles / pick-block. It does NOT reuse the wood/log textures.
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
	Icon *backIcons[7];	// top-right quadrant per wood (back panel)
	// Java 1.21.9 powered fronts (Phase 7): one per wood, same layout
	// as the front icon with the divider bars recolored redstone-red.
	// Selected by getFrontIcon when data bit 3 is set.
	Icon *poweredIcons[7];

public:
	ShelfTile(int id);

	// Java 1.21.9 powered mode (Phase 7): data bit 3 is the powered
	// state (wood keeps bits 0-2). A shelf is powered while it receives
	// a redstone signal; the front texture switches to the powered icon
	// and using any shelf in a powered group swaps the group's contents
	// with the hotbar (see swapPoweredGroup).
	static bool isPowered(int data) { return (data & 8) != 0; }
	// Re-checks the redstone signal and sets/clears bit 3 (with the
	// activate/deactivate sound). Called from neighborChanged and from
	// setPlacedBy (a shelf placed next to a powered source starts on).
	void updatePoweredState(Level *level, int x, int y, int z);
	// Powered-group walk: the clicked shelf plus adjacent powered
	// shelves with the same facing, left-priority, max 3 total.
	// Positions are returned left-to-right (as seen by a player facing
	// the front); all share y. Returns the group size (1-3).
	static int getPoweredGroup(Level *level, int x, int y, int z, int facing, int *outX, int *outZ);
	// Swaps the clicked shelf's powered group with the hotbar (1 shelf
	// -> rightmost 3 slots, 2 -> rightmost 6, 3 -> all 9).
	void swapPoweredGroup(Level *level, int x, int y, int z, shared_ptr<Player> player);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual int getRenderShape() override;
	// Front (top-left quadrant) / back (top-right quadrant) atlas icons for
	// the multi-cuboid model. data & 7 is the wood index.
	Icon *getFrontIcon(int data);
	Icon *getBackIcon(int data);
	// Per-wood display names (2026-11-23), same order as registerIcons:
	// OAK, SPRUCE, BIRCH, JUNGLE, ACACIA, DARK_OAK, BAMBOO. Used by
	// ShelfTile::getDescriptionId and the MultiTextureTileItem in Tile.cpp
	// so each shelf variant is named "Oak Shelf", "Spruce Shelf", etc.
	static const unsigned int SHELF_NAMES[7];
	virtual unsigned int getDescriptionId(int iData = -1) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;
	virtual void neighborChanged(Level *level, int x, int y, int z, int type) override;
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
