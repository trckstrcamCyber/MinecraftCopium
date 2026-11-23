#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "CopperSlabTile.h"
#include "IconRegister.h"

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

const unsigned int CopperSlabTile::SLAB_NAMES[8] = {
	IDS_TILE_CUT_COPPER_SLAB,
	IDS_TILE_EXPOSED_CUT_COPPER_SLAB,
	IDS_TILE_WEATHERED_CUT_COPPER_SLAB,
	IDS_TILE_OXIDIZED_CUT_COPPER_SLAB,
	IDS_TILE_WAXED_CUT_COPPER_SLAB,
	IDS_TILE_WAXED_EXPOSED_CUT_COPPER_SLAB,
	IDS_TILE_WAXED_WEATHERED_CUT_COPPER_SLAB,
	IDS_TILE_WAXED_OXIDIZED_CUT_COPPER_SLAB
};

unsigned int CopperSlabTile::getDescriptionId(int iData)
{
	return getAuxName(iData);
}

int CopperSlabTile::getAuxName(int auxValue)
{
	if (auxValue < 0 || auxValue > 7) auxValue = 0;
	return SLAB_NAMES[auxValue & 7];
}
