#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "IconRegister.h"
#include "PurpurSlabTile.h"

PurpurSlabTile::PurpurSlabTile(int id)
	: HalfSlabTile(id, Material::stone)
{
}

Icon *PurpurSlabTile::getTexture(int face, int data)
{
	return icon;
}

void PurpurSlabTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"purpur_block");
}

int PurpurSlabTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::purpur_slab_Id;
}

unsigned int PurpurSlabTile::getDescriptionId(int iData)
{
	return IDS_TILE_PURPUR_SLAB;
}

int PurpurSlabTile::getAuxName(int auxValue)
{
	return IDS_TILE_PURPUR_SLAB;
}

shared_ptr<ItemInstance> PurpurSlabTile::getSilkTouchItemInstance(int data)
{
	return make_shared<ItemInstance>(Tile::purpur_slab_Id, 2, 0);
}
