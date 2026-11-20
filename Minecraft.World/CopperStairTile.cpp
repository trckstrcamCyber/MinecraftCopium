#include "stdafx.h"
#include "CopperStairTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"

CopperStairTile::CopperStairTile(int id) : StairTile(id, Tile::cutCopper, 0)
{
}

Icon *CopperStairTile::getTexture(int face, int data)
{
	// Oxidation lives in bits 3-5 of the stairs' data.
	return Tile::cutCopper->getTexture(face, (data >> 3) & 7);
}

Icon *CopperStairTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return getTexture(face, level->getData(x, y, z));
}

void CopperStairTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	// Base StairTile::setPlacedBy rewrites the data as DIR | UPSIDEDOWN and
	// would wipe the oxidation bits; preserve them (bits 3-5).
	int data = level->getData(x, y, z);
	int oxidation = data & 0x38;
	int usd = data & UPSIDEDOWN_BIT;
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;

	if (dir == 0) level->setData(x, y, z, DIR_SOUTH | usd | oxidation, Tile::UPDATE_CLIENTS);
	if (dir == 1) level->setData(x, y, z, DIR_WEST | usd | oxidation, Tile::UPDATE_CLIENTS);
	if (dir == 2) level->setData(x, y, z, DIR_NORTH | usd | oxidation, Tile::UPDATE_CLIENTS);
	if (dir == 3) level->setData(x, y, z, DIR_EAST | usd | oxidation, Tile::UPDATE_CLIENTS);
}

int CopperStairTile::getSpawnResourcesAuxValue(int data)
{
	return (data >> 3) & 7;
}
