#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "CopperSlabTile.h"

CopperSlabTile::CopperSlabTile(int id, int resourceTileId)
	: HalfSlabTile(id, Material::metal)
{
	this->resourceTileId = resourceTileId;
}

Icon *CopperSlabTile::getTexture(int face, int data)
{
	// The slab's "type" field (bits 0-2) IS the oxidation state.
	return Tile::cutCopper->getTexture(face, data & TYPE_MASK);
}

Icon *CopperSlabTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return Tile::cutCopper->getTexture(face, level->getData(x, y, z) & TYPE_MASK);
}

void CopperSlabTile::registerIcons(IconRegister *iconRegister)
{
	// Texture is delegated to the cut-copper tile; just give the tile a
	// valid icon field for particles / pick-block.
	icon = iconRegister->registerIcon(L"cut_copper");
}

int CopperSlabTile::getResource(int data, Random *random, int playerBonusLevel)
{
	// Breaking either the half or the full slab drops the half-slab item.
	return resourceTileId;
}

unsigned int CopperSlabTile::getDescriptionId(int iData)
{
	return IDS_TILE_CUT_COPPER_SLAB;
}

int CopperSlabTile::getAuxName(int auxValue)
{
	return IDS_TILE_CUT_COPPER_SLAB;
}
