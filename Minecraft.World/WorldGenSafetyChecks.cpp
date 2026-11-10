#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "WorldGenSafetyChecks.h"

bool WorldGenSafetyChecks::isNaturalGroundTile(int tileId)
{
	return tileId == Tile::grass_Id
		|| tileId == Tile::dirt_Id
		|| tileId == Tile::stone_Id
		|| tileId == Tile::sand_Id
		|| tileId == Tile::gravel_Id;
}

bool WorldGenSafetyChecks::isSafeOutdoorSurface(Level *level, int x, int z, int &outGroundY)
{
	int groundY = level->getTopSolidBlock(x, z);
	outGroundY = groundY;

	// Must be natural ground, not e.g. a tree log, path, or a block a
	// structure already placed.
	int groundTile = level->getTile(x, groundY - 1, z);
	if (!isNaturalGroundTile(groundTile)) return false;

	// Must be dry - not a lake/river bed or ocean floor.
	if (groundTile == Tile::flowing_water_Id || groundTile == Tile::water_Id) return false;
	if (level->getTile(x, groundY, z) == Tile::water_Id || level->getTile(x, groundY, z) == Tile::flowing_water_Id) return false;

	// Must be open to the sky - rules out caves, overhangs, and anything
	// already roofed over by a structure.
	if (!level->canSeeSky(x, groundY, z)) return false;

	// The space we'd actually build/stand in must be clear.
	if (level->getTile(x, groundY, z) != Tile::air_Id) return false;
	if (level->getTile(x, groundY + 1, z) != Tile::air_Id) return false;

	return true;
}

bool WorldGenSafetyChecks::isFootprintNatural(Level *level, int x0, int z0, int x1, int z1, int y)
{
	int xMin = x0 < x1 ? x0 : x1;
	int xMax = x0 < x1 ? x1 : x0;
	int zMin = z0 < z1 ? z0 : z1;
	int zMax = z0 < z1 ? z1 : z0;

	for (int x = xMin; x <= xMax; x++)
	{
		for (int z = zMin; z <= zMax; z++)
		{
			int groundTile = level->getTile(x, y - 1, z);
			if (!isNaturalGroundTile(groundTile)) return false;
			if (level->getTile(x, y, z) != Tile::air_Id) return false;
			if (!level->canSeeSky(x, y, z)) return false;
		}
	}
	return true;
}
