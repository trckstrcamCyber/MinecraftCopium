#include "stdafx.h"
#include "CopperFenceTile.h"
#include "net.minecraft.world.level.h"

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
	// Base fields (used by the edge renderer, which has no data parameter).
	icon = stateIcons[0];
	iconSide = stateIcons[0];
}

int CopperFenceTile::getSpawnResourcesAuxValue(int data)
{
	return data & 7;
}
