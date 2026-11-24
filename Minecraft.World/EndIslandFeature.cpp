#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "EndIslandFeature.h"

bool EndIslandFeature::place(Level *level, Random *random, int x, int y, int z)
{
	float radius = static_cast<float>(random->nextInt(3)) + 4.0f;
	for (int yy = 0; radius > 0.5f; radius -= static_cast<float>(random->nextInt(2)) + 0.5f, --yy)
	{
		for (int xx = static_cast<int>(floorf(-radius)); xx <= static_cast<int>(ceilf(radius)); ++xx)
		{
			for (int zz = static_cast<int>(floorf(-radius)); zz <= static_cast<int>(ceilf(radius)); ++zz)
			{
				if (static_cast<float>(xx * xx + zz * zz) <= (radius + 1.0f) * (radius + 1.0f))
				{
					placeBlock(level, x + xx, y + yy, z + zz, Tile::end_stone_Id, 0);
				}
			}
		}
	}
	return true;
}
