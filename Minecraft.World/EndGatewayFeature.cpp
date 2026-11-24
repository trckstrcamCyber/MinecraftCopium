#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "EndGatewayFeature.h"

bool EndGatewayFeature::place(Level *level, Random *random, int x, int y, int z)
{
	for (int xx = x - 1; xx <= x + 1; xx++)
	{
		for (int zz = z - 1; zz <= z + 1; zz++)
		{
			for (int yy = y - 2; yy <= y + 2; yy++)
			{
				bool atX = xx == x;
				bool atY = yy == y;
				bool atZ = zz == z;
				bool edgeY = abs(yy - y) == 2;

				int tile = Tile::bedrock_Id;
				if (atX && atY && atZ) tile = Tile::end_gateway_Id;
				else if (atY) tile = 0;
				else if (edgeY && atX && atZ) tile = Tile::bedrock_Id;
				else if ((!atX && !atZ) || edgeY) tile = 0;

				placeBlock(level, xx, yy, zz, tile, 0);
			}
		}
	}
	return true;
}
