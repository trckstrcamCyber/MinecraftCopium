#include "stdafx.h"
#include "ShelfTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

ShelfTile::ShelfTile(int id) : Tile(id, Material::wood)
{
	for (int i = 0; i < 7; i++) woodIcons[i] = nullptr;
}

Icon *ShelfTile::getTexture(int face, int data)
{
	int wood = data & 7;
	if (wood > 6) wood = 0;
	return woodIcons[wood];
}

Icon *ShelfTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return getTexture(face, level->getData(x, y, z));
}

void ShelfTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *names[7] = {
		L"oak_shelf", L"spruce_shelf", L"birch_shelf", L"jungle_shelf",
		L"acacia_shelf", L"dark_oak_shelf", L"bamboo_shelf"
	};
	for (int i = 0; i < 7; i++)
	{
		woodIcons[i] = iconRegister->registerIcon(names[i]);
	}
	icon = woodIcons[0];
}

int ShelfTile::getSpawnResourcesAuxValue(int data)
{
	return data & 7;
}
