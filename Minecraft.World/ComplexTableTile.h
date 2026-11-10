#pragma once

// neoLegacy - Complex Table
//
// A WorkbenchTile clone: identical shape, sounds, and icons (reuses
// "workbench_side/top/front" -- no new texture assets), but rendered with
// a blue tint via the getColor() override below so it's visually
// distinguishable in-world, and it opens a ComplexCraftingMenu (its own
// exclusive concrete-powder recipe set) instead of the normal CraftingMenu.

#include "Tile.h"

class Player;
class ChunkRebuildData;

class ComplexTableTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
private:
	Icon *iconTop;
	Icon *iconFront;

protected:
	ComplexTableTile(int id);

public:
	Icon *getTexture(int face, int data);
	void registerIcons(IconRegister *iconRegister);

	// Universal per-face tint hook (see TileRenderer::tesselateBlockInWorld) --
	// multiplies the reused workbench texture by this color at render time,
	// so no new texture asset is needed for the blue hue. Tune the hex value
	// below to taste; it's a straight RGB multiply against the wood texture.
	int getColor(LevelSource *level, int x, int y, int z);

	// Same tint for the item forms (inventory slot, held item, dropped item). Those go through
	// TileRenderer::renderTile(), which asks for getColor(auxData) -- NOT the in-world
	// getColor(level,x,y,z) above -- so without this the block was blue when placed but
	// plain wood in the inventory.
	int getColor(int auxData);

public:
	virtual bool TestUse();
	bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
};
