#include "stdafx.h"
#include "CopperGrateTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

const unsigned int CopperGrateTile::COPPER_GRATE_NAMES[8] = {
	IDS_TILE_COPPER_GRATE,
	IDS_TILE_EXPOSED_COPPER_GRATE,
	IDS_TILE_WEATHERED_COPPER_GRATE,
	IDS_TILE_OXIDIZED_COPPER_GRATE,
	IDS_TILE_WAXED_COPPER_GRATE,
	IDS_TILE_WAXED_EXPOSED_COPPER_GRATE,
	IDS_TILE_WAXED_WEATHERED_COPPER_GRATE,
	IDS_TILE_WAXED_OXIDIZED_COPPER_GRATE
};

CopperGrateTile::CopperGrateTile(int id, const wstring &baseIconName)
	: HalfTransparentTile(id, baseIconName, Material::metal, false)
{
	this->baseIconName = baseIconName;
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

Icon *CopperGrateTile::getTexture(int face, int data)
{
	// Waxed (bit 2) reuses the unwaxed texture for the same oxidation state.
	return stateIcons[data & 3];
}

Icon *CopperGrateTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperGrateTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring name = wstring(prefixes[i]) + baseIconName;
		stateIcons[i] = iconRegister->registerIcon(name);
	}
	// Keep the base Tile::icon on the unaffected state so anything that reads
	// the icon field directly (particles, pick-block) gets a valid copper icon.
	icon = stateIcons[0];
}

int CopperGrateTile::getSpawnResourcesAuxValue(int data)
{
	// Breaking a copper grate drops the same oxidation state (waxed included).
	return data & 7;
}

unsigned int CopperGrateTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	return COPPER_GRATE_NAMES[iData & 7];
}
