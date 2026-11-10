#pragma once

#include "BaseEntityTile.h"

// neoLegacy - the Crusher and the Refiner. Same block class, same tile entity
// (ProcessorTileEntity); the two differ only in their recipe table and look:
//
//   * Crusher: top = "crusher_top", bottom = "crusher_bottom", four sides = "observer_side".
//   * Refiner: the same textures with a gold tint (see getColor) so the two are told apart
//     without needing extra texture art. Swap in dedicated textures whenever they exist.
//
// Not directional: a hopper feeds the TOP, the product leaves out of the BOTTOM, so there is no
// facing and the block data is unused.
class ProcessorTile : public BaseEntityTile
{
public:
	// Gold tint for the Refiner -- straight RGB multiply against the shared textures.
	static const int REFINER_TINT = 0xF0C060;

	ProcessorTile(int id, bool isRefiner);

	virtual bool isCubeShaped() override { return true; }
	virtual bool isSolidRender(bool isServerLevel = false) override { return true; }

	virtual Icon *getTexture(int face, int data) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

	// In-world tint, and the same tint for the item forms (inventory / held / dropped), which
	// ask for getColor(auxData) instead -- see ComplexTableTile for why both are needed.
	virtual int getColor(LevelSource *level, int x, int y, int z) override;
	virtual int getColor(int auxData) override;

	// Spill the contents when the block is broken.
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;

	virtual shared_ptr<TileEntity> newTileEntity(Level *level) override;

private:
	bool refiner;
	Icon *topIcon;
	Icon *bottomIcon;
	Icon *sideIcon;
};
