#include "stdafx.h"
#include "CopperTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

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
