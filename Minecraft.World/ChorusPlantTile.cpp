#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "ChorusPlantTile.h"

const float ChorusPlantTile::CORE_MIN = 3.0f / 16.0f;
const float ChorusPlantTile::CORE_MAX = 13.0f / 16.0f;

ChorusPlantTile::ChorusPlantTile(int id) : Tile(id, Material::plant, false)
{
	setLightBlock(0);
	updateDefaultShape();
}

bool ChorusPlantTile::connectsTo(LevelSource *level, int x, int y, int z, int face)
{
	int t = level->getTile(x + Facing::STEP_X[face], y + Facing::STEP_Y[face], z + Facing::STEP_Z[face]);
	if (t == id || t == Tile::chorus_flower_Id) return true;
	return face == Facing::DOWN && t == Tile::end_stone_Id;
}

void ChorusPlantTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	float x0 = connectsTo(level, x, y, z, Facing::WEST) ? 0.0f : CORE_MIN;
	float y0 = connectsTo(level, x, y, z, Facing::DOWN) ? 0.0f : CORE_MIN;
	float z0 = connectsTo(level, x, y, z, Facing::NORTH) ? 0.0f : CORE_MIN;
	float x1 = connectsTo(level, x, y, z, Facing::EAST) ? 1.0f : CORE_MAX;
	float y1 = connectsTo(level, x, y, z, Facing::UP) ? 1.0f : CORE_MAX;
	float z1 = connectsTo(level, x, y, z, Facing::SOUTH) ? 1.0f : CORE_MAX;
	setShape(x0, y0, z0, x1, y1, z1);
}

void ChorusPlantTile::updateDefaultShape()
{
	setShape(CORE_MIN, CORE_MIN, CORE_MIN, CORE_MAX, CORE_MAX, CORE_MAX);
}

void ChorusPlantTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	const float a = CORE_MIN;
	const float b = CORE_MAX;

	setShape(a, a, a, b, b, b);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
	if (connectsTo(level, x, y, z, Facing::WEST))
	{
		setShape(0.0f, a, a, a, b, b);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	if (connectsTo(level, x, y, z, Facing::EAST))
	{
		setShape(b, a, a, 1.0f, b, b);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	if (connectsTo(level, x, y, z, Facing::UP))
	{
		setShape(a, b, a, b, 1.0f, b);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	if (connectsTo(level, x, y, z, Facing::DOWN))
	{
		setShape(a, 0.0f, a, b, a, b);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	if (connectsTo(level, x, y, z, Facing::NORTH))
	{
		setShape(a, a, 0.0f, b, b, a);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	if (connectsTo(level, x, y, z, Facing::SOUTH))
	{
		setShape(a, a, b, b, b, 1.0f);
		Tile::addAABBs(level, x, y, z, box, boxes, source);
	}
	updateShape(level, x, y, z);
}

bool ChorusPlantTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool ChorusPlantTile::isCubeShaped()
{
	return false;
}

int ChorusPlantTile::getRenderShape()
{
	return Tile::SHAPE_CHORUS_PLANT;
}

bool ChorusPlantTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return false;
}

void ChorusPlantTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!canSurvive(level, x, y, z))
	{
		level->destroyTile(x, y, z, true);
	}
}

int ChorusPlantTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::chorus_fruit_Id;
}

int ChorusPlantTile::getResourceCount(Random *random)
{
	return random->nextInt(2);
}

bool ChorusPlantTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && canSurvive(level, x, y, z);
}

void ChorusPlantTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		level->addToTickNextTick(x, y, z, id, 1);
	}
}

bool ChorusPlantTile::canSurvive(Level *level, int x, int y, int z)
{
	bool aboveEmpty = level->isEmptyTile(x, y + 1, z);
	bool belowEmpty = level->isEmptyTile(x, y - 1, z);

	for (int face = Facing::NORTH; face <= Facing::EAST; ++face)
	{
		int sx = x + Facing::STEP_X[face];
		int sz = z + Facing::STEP_Z[face];
		if (level->getTile(sx, y, sz) == id)
		{
			if (!aboveEmpty && !belowEmpty)
			{
				return false;
			}
			int below = level->getTile(sx, y - 1, sz);
			if (below == id || below == Tile::end_stone_Id)
			{
				return true;
			}
		}
	}

	int below = level->getTile(x, y - 1, z);
	return below == id || below == Tile::end_stone_Id;
}
