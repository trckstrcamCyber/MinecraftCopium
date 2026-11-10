#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "BambooFeature.h"
#include "BambooTile.h"
#include "DirtTile.h"

BambooFeature::BambooFeature(int attempts, int radius, bool podzol) : Feature(false), attempts(attempts), radius(radius), podzol(podzol)
{
}

bool BambooFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (radius > 4) radius = 4;
	bool placedAny = false;

	// podzol under the patch first, so the stalks below stand on it
	if (podzol)
	{
		int pr = radius - 1;
		if (pr < 1) pr = 1;
		for (int dx = -pr; dx <= pr; ++dx)
		{
			for (int dz = -pr; dz <= pr; ++dz)
			{
				if (dx * dx + dz * dz > pr * pr + 1) continue;
				int gx = x + dx, gz = z + dz;
				int gy = level->getHeightmap(gx, gz) - 1;
				int t = level->getTile(gx, gy, gz);
				if ((t == Tile::grass_Id || t == Tile::dirt_Id) && level->isEmptyTile(gx, gy + 1, gz))
				{
					level->setTileAndData(gx, gy, gz, Tile::dirt_Id, DirtTile::PODZOL, Tile::UPDATE_CLIENTS);
				}
			}
		}
	}

	for (int i = 0; i < attempts; ++i)
	{
		int bx = x + random->nextInt(radius * 2 + 1) - radius;
		int bz = z + random->nextInt(radius * 2 + 1) - radius;
		int by = level->getHeightmap(bx, bz);		// first free block above the ground

		if (!BambooTile::isValidSoil(level->getTile(bx, by - 1, bz))) continue;
		if (!level->isEmptyTile(bx, by, bz)) continue;

		int height = 5 + random->nextInt(12);		// 5..16
		int free = 0;
		while (free < height && level->isEmptyTile(bx, by + free, bz)) ++free;
		height = free;
		if (height < 3) continue;

		for (int h = 0; h < height; ++h)
		{
			int data = BambooTile::AGE_BIT;
			if (h == height - 1)
			{
				data |= BambooTile::STAGE_BIT | (BambooTile::LEAVES_LARGE << BambooTile::LEAVES_SHIFT);
			}
			else if (h == height - 2)
			{
				data |= BambooTile::LEAVES_SMALL << BambooTile::LEAVES_SHIFT;
			}
			level->setTileAndData(bx, by + h, bz, Tile::bamboo_Id, data, Tile::UPDATE_CLIENTS);
		}
		placedAny = true;
	}
	return placedAny;
}
