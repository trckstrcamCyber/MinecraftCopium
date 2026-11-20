#pragma once
#include "Tile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). The copper shelf (Java shelf block).
// One tile id; aux = wood index 0-6 (same global order as
// StrippedLogTile: OAK, SPRUCE, BIRCH, JUNGLE, ACACIA, DARK_OAK, BAMBOO).
// The shelf has its OWN per-wood texture in jar1201 (32x32, downscaled to
// 16x16 for the atlas) — it does NOT reuse the wood/log textures.
//
// Phase 3 scope: the tile + recipe only. Storing / displaying an item on
// the shelf (Java shelf behaviour) needs a tile entity + interaction and is
// Phase 6.
class ShelfTile : public Tile
{
private:
	Icon *woodIcons[7];

public:
	ShelfTile(int id);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
