#include "stdafx.h"
#include "CopperFenceTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

const unsigned int CopperFenceTile::COPPER_BARS_NAMES[8] = {
	IDS_TILE_COPPER_BARS,
	IDS_TILE_EXPOSED_COPPER_BARS,
	IDS_TILE_WEATHERED_COPPER_BARS,
	IDS_TILE_OXIDIZED_COPPER_BARS,
	IDS_TILE_WAXED_COPPER_BARS,
	IDS_TILE_WAXED_EXPOSED_COPPER_BARS,
	IDS_TILE_WAXED_WEATHERED_COPPER_BARS,
	IDS_TILE_WAXED_OXIDIZED_COPPER_BARS
};

CopperFenceTile::CopperFenceTile(int id, const wstring &baseIconName)
	: ThinFenceTile(id, baseIconName, baseIconName, Material::metal, true)
{
	this->baseIconName = baseIconName;
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

Icon *CopperFenceTile::getTexture(int face, int data)
{
	return stateIcons[data & 3];
}

Icon *CopperFenceTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperFenceTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring name = wstring(prefixes[i]) + baseIconName;
		stateIcons[i] = iconRegister->registerIcon(name);
	}
	// Base field (used by anything that reads the icon directly). The edge
	// texture comes from the getEdgeTexture() override below.
	icon = stateIcons[0];
}

Icon *CopperFenceTile::getEdgeTexture()
{
	return stateIcons[0];
}

int CopperFenceTile::getSpawnResourcesAuxValue(int data)
{
	return data & 7;
}

unsigned int CopperFenceTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	return COPPER_BARS_NAMES[iData & 7];
}
