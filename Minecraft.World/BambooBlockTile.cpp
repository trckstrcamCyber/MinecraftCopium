#include "stdafx.h"
#include "BambooBlockTile.h"
#include "IconRegister.h"

BambooBlockTile::BambooBlockTile(int id, const wstring &sideName, const wstring &topName)
	: RotatedPillarTile(id, Material::wood), sideName(sideName), topName(topName)
{
	sideIcon = nullptr;
	iconTop = nullptr;
}

Icon *BambooBlockTile::getTypeTexture(int type)
{
	return sideIcon;
}

void BambooBlockTile::registerIcons(IconRegister *iconRegister)
{
	sideIcon = iconRegister->registerIcon(sideName);
	iconTop = iconRegister->registerIcon(topName);
}
