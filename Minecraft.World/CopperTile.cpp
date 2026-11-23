#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "CopperTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

const unsigned int CopperTile::COPPER_BLOCK_NAMES[8] = {
	IDS_TILE_COPPER_BLOCK,
	IDS_TILE_EXPOSED_COPPER,
	IDS_TILE_WEATHERED_COPPER,
	IDS_TILE_OXIDIZED_COPPER,
	IDS_TILE_WAXED_COPPER_BLOCK,
	IDS_TILE_WAXED_EXPOSED_COPPER,
	IDS_TILE_WAXED_WEATHERED_COPPER,
	IDS_TILE_WAXED_OXIDIZED_COPPER
};
const unsigned int CopperTile::CUT_COPPER_NAMES[8] = {
	IDS_TILE_CUT_COPPER,
	IDS_TILE_EXPOSED_CUT_COPPER,
	IDS_TILE_WEATHERED_CUT_COPPER,
	IDS_TILE_OXIDIZED_CUT_COPPER,
	IDS_TILE_WAXED_CUT_COPPER,
	IDS_TILE_WAXED_EXPOSED_CUT_COPPER,
	IDS_TILE_WAXED_WEATHERED_CUT_COPPER,
	IDS_TILE_WAXED_OXIDIZED_CUT_COPPER
};
const unsigned int CopperTile::CHISELED_COPPER_NAMES[8] = {
	IDS_TILE_CHISELED_COPPER,
	IDS_TILE_EXPOSED_CHISELED_COPPER,
	IDS_TILE_WEATHERED_CHISELED_COPPER,
	IDS_TILE_OXIDIZED_CHISELED_COPPER,
	IDS_TILE_WAXED_CHISELED_COPPER,
	IDS_TILE_WAXED_EXPOSED_CHISELED_COPPER,
	IDS_TILE_WAXED_WEATHERED_CHISELED_COPPER,
	IDS_TILE_WAXED_OXIDIZED_CHISELED_COPPER
};

CopperTile::CopperTile(int id, const wstring &baseIconName) : MetalTile(id)
{
	this->baseIconName = baseIconName;
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

Icon *CopperTile::getTexture(int face, int data)
{
	// Waxed (bit 2) reuses the unwaxed texture for the same oxidation state.
	return stateIcons[data & 3];
}

Icon *CopperTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperTile::registerIcons(IconRegister *iconRegister)
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

int CopperTile::getSpawnResourcesAuxValue(int data)
{
	// Breaking a copper block drops the same oxidation state (waxed included).
	return data & 7;
}

unsigned int CopperTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	int aux = iData & 7;
	if (id == Tile::copper_block_Id) return COPPER_BLOCK_NAMES[aux];
	if (id == Tile::cut_copper_Id) return CUT_COPPER_NAMES[aux];
	if (id == Tile::chiseled_copper_Id) return CHISELED_COPPER_NAMES[aux];
	return Tile::getDescriptionId(iData);
}
