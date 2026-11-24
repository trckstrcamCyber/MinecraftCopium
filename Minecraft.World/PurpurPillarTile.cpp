#include "stdafx.h"
#include "net.minecraft.world.h"
#include "IconRegister.h"
#include "PurpurPillarTile.h"

PurpurPillarTile::PurpurPillarTile(int id) : RotatedPillarTile(id, Material::stone)
{
}

Icon *PurpurPillarTile::getTypeTexture(int type)
{
	return icon;
}

void PurpurPillarTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = iconRegister->registerIcon(L"purpur_pillar_top");
	icon = iconRegister->registerIcon(L"purpur_pillar");
}
