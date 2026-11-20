#pragma once
#include "ThinFenceTile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). Thin fence-like tile (iron-bars shape)
// whose texture varies by oxidation state. Used for the copper bars and the
// copper chain (both are pane-like in LCE; the chain reads as a chain-link
// panel, the closest available shape to Java's thin vertical rod).
//
// Aux layout: same as CopperTile (bits 0-1 oxidation, bit 2 waxed).
// Icon names are the base name prefixed with "exposed_" / "weathered_" /
// "oxidized_" (e.g. L"copper_bars" -> L"exposed_copper_bars").
//
// NOTE: ThinFenceTile's edge texture (iconSide, via getEdgeTexture()) has no
// data parameter, so the thin edges always show the unaffected icon; only the
// main faces vary by oxidation state. Cosmetic; the faces carry the state.
class CopperFenceTile : public ThinFenceTile
{
private:
	wstring baseIconName;
	Icon *stateIcons[4];

public:
	CopperFenceTile(int id, const wstring &baseIconName);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
