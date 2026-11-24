#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "Facing.h"
#include "EndRodTile.h"

EndRodTile::EndRodTile(int id) : Tile(id, Material::decoration, false)
{
	setLightBlock(0);
	updateDefaultShape();
}

void EndRodTile::setShapeForData(int data)
{
	const float a = 6.0f / 16.0f;
	const float b = 10.0f / 16.0f;
	switch (getFacing(data))
	{
	case Facing::WEST:
	case Facing::EAST:
		setShape(0.0f, a, a, 1.0f, b, b);
		break;
	case Facing::NORTH:
	case Facing::SOUTH:
		setShape(a, a, 0.0f, b, b, 1.0f);
		break;
	default:
		setShape(a, 0.0f, a, b, 1.0f, b);
		break;
	}
}

AABB *EndRodTile::getAABB(Level *level, int x, int y, int z)
{
	setShapeForData(level->getData(x, y, z));
	return Tile::getAABB(level, x, y, z);
}

AABB *EndRodTile::getTileAABB(Level *level, int x, int y, int z)
{
	setShapeForData(level->getData(x, y, z));
	return Tile::getTileAABB(level, x, y, z);
}

void EndRodTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	setShapeForData(forceData >= 0 ? forceData : level->getData(x, y, z));
}

void EndRodTile::updateDefaultShape()
{
	setShapeForData(Facing::UP);
}

bool EndRodTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool EndRodTile::isCubeShaped()
{
	return false;
}

int EndRodTile::getRenderShape()
{
	return Tile::SHAPE_END_ROD;
}

int EndRodTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int ox = x - Facing::STEP_X[face];
	int oy = y - Facing::STEP_Y[face];
	int oz = z - Facing::STEP_Z[face];
	if (level->getTile(ox, oy, oz) == id && getFacing(level->getData(ox, oy, oz)) == face)
	{
		return Facing::OPPOSITE_FACING[face];
	}
	return face;
}

void EndRodTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	int facing = getFacing(level->getData(xt, yt, zt));
	double x = xt + 0.55 - random->nextFloat() * 0.1f;
	double y = yt + 0.55 - random->nextFloat() * 0.1f;
	double z = zt + 0.55 - random->nextFloat() * 0.1f;
	double spread = 0.4f - (random->nextFloat() + random->nextFloat()) * 0.4f;
	if (random->nextInt(5) == 0)
	{
		level->addParticle(eParticleType_endRod,
			x + Facing::STEP_X[facing] * spread,
			y + Facing::STEP_Y[facing] * spread,
			z + Facing::STEP_Z[facing] * spread,
			random->nextGaussian() * 0.005, random->nextGaussian() * 0.005, random->nextGaussian() * 0.005);
	}
}

int EndRodTile::getPistonPushReaction()
{
	return Material::PUSH_NORMAL;
}

int EndRodTile::getSpawnResourcesAuxValue(int data)
{
	return 0;
}
