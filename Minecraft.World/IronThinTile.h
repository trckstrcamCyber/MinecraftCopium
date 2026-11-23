#pragma once
#include "CopperThinTile.h"
class IconRegister;

// neoLegacy - Iron additions (2026-11-22). Iron lantern (2330 chain / 2331
// lantern -- see Tile.h) and iron chain are single-state thin tiles: iron
// has no oxidation stages and no waxed state, so this subclasses
// CopperThinTile (reusing its shape/bounds plumbing and the
// CopperThinTile* renderer signatures) but registers exactly ONE icon and
// ignores data bits in getTexture. It must NOT use
// CopperThinTile::registerIcons: that registers the four oxidation-prefixed
// names (exposed_/weathered_/oxidized_<name>), and registerIcon DEBUG_BREAKs
// (int3, 0x80000003) on the first missing name (see PreStitchedTextureMap.cpp
// row 36 note) -- exposed_iron_chain does not exist.
class IronThinTile : public CopperThinTile
{
private:
	wstring ironIconName;
	Icon *ironIcon;
public:
	IronThinTile(int id, const wstring &baseIconName,
		float x0, float y0, float z0, float x1, float y1, float z1);
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	virtual int getRenderShape() override;
protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
