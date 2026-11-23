#pragma once
#include "HalfTransparentTile.h"
class IconRegister;

// neoLegacy - Copper Age (playtest fix, 2026-11-20). The copper grate as a
// see-through grate instead of a solid metal cube: HalfTransparentTile
// gives isSolidRender() == false, blocksLight() == false, setLightBlock(0),
// and a shouldRenderFace() that hides faces between two adjacent grates
// (allowSame == false, the GlassTile precedent). Without this the floor
// block below a grate had its top face culled and looking down through
// the grate's transparent holes showed straight through the floor.
// Icon logic is CopperTile's: bits 0-1 oxidation, bit 2 waxed (same texture).
class CopperGrateTile : public HalfTransparentTile
{
private:
	wstring baseIconName;
	Icon *stateIcons[4];
public:
	CopperGrateTile(int id, const wstring &baseIconName);
	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	// Per-oxidation display names (2026-11-23), Java 26.3. Index = aux 0-7
	// (0 unaffected, 1 exposed, 2 weathered, 3 oxidized, 4-7 waxed).
	static const unsigned int COPPER_GRATE_NAMES[8];
	virtual unsigned int getDescriptionId(int iData = -1) override;
protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
