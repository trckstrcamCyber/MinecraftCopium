#pragma once
#include "MetalTile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). One reusable cube tile for the copper
// families whose texture varies only by oxidation state (copper block, cut
// copper, chiseled copper, copper grate, copper lantern, lightning rod).
//
// Aux layout (shared by the whole copper family, analysis §0.1):
//   bits 0-1  oxidation 0=unaffected, 1=exposed, 2=weathered, 3=oxidized
//   bit 2     waxed (+4); waxed reuses the same texture as the unwaxed state,
//             so getTexture masks with 3 and never needs a separate icon.
// The four icon names are derived from the base name by prefixing
// "exposed_" / "weathered_" / "oxidized_" (e.g. base L"copper_block" gives
// L"copper_block", L"exposed_copper_block", L"weathered_copper_block",
// L"oxidized_copper_block").
class CopperTile : public MetalTile
{
private:
	wstring baseIconName;
	Icon *stateIcons[4];

public:
	CopperTile(int id, const wstring &baseIconName);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;
	// Per-oxidation display names (2026-11-23), Java 26.3. Index = aux 0-7
	// (0 unaffected, 1 exposed, 2 weathered, 3 oxidized, 4-7 waxed).
	static const unsigned int COPPER_BLOCK_NAMES[8];
	static const unsigned int CUT_COPPER_NAMES[8];
	static const unsigned int CHISELED_COPPER_NAMES[8];
	virtual unsigned int getDescriptionId(int iData = -1) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
};
