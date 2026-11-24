#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "EndCityFeature.h"
#include "EndCityPieces.h"
#include "BoundingBox.h"
#include "TheEndDimension.h"
#include "TheEndLevelRandomLevelSource.h"

EndCityFeature::EndCityFeature(TheEndLevelRandomLevelSource *source) : StructureFeature()
{
	this->source = source;
}

wstring EndCityFeature::getFeatureName()
{
	return L"EndCity";
}

bool EndCityFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	return TheEndDimension::isOuterAreaCentre(x, z);
}

StructureStart *EndCityFeature::createStructureStart(int x, int z)
{
	return new EndCityStart(level, source, random, x, z);
}

int EndCityFeature::getYPositionForFeature(int chunkX, int chunkZ, TheEndLevelRandomLevelSource *source)
{
	Random random(chunkX + chunkZ * 10387313);
	int rotation = random.nextInt(4);

	unsigned int blocksSize = Level::genDepth * 16 * 16;
	byteArray blocks = byteArray(blocksSize);
	memset(blocks.data, 0, blocksSize);
	BiomeArray biomes;
	source->prepareHeights(chunkX, chunkZ, blocks, biomes);

	int xOffset = 5;
	int zOffset = 5;
	if (rotation == EndCityPieces::ROT_CLOCKWISE_90) xOffset = -5;
	else if (rotation == EndCityPieces::ROT_CLOCKWISE_180) { xOffset = -5; zOffset = -5; }
	else if (rotation == EndCityPieces::ROT_COUNTERCLOCKWISE_90) zOffset = -5;

	const int columns[4][2] = { { 7, 7 }, { 7, 7 + zOffset }, { 7 + xOffset, 7 }, { 7 + xOffset, 7 + zOffset } };
	int lowest = Level::genDepth;
	for (int i = 0; i < 4; i++)
	{
		int top = 0;
		for (int y = Level::genDepth - 1; y > 0; y--)
		{
			int offs = (columns[i][0] << Level::genDepthBitsPlusFour) | (columns[i][1] << Level::genDepthBits) | y;
			if (blocks[offs] != 0)
			{
				top = y;
				break;
			}
		}
		if (top < lowest) lowest = top;
	}

	delete [] blocks.data;
	delete [] biomes.data;
	return lowest;
}

EndCityFeature::EndCityStart::EndCityStart()
{
}

EndCityFeature::EndCityStart::EndCityStart(Level *level, TheEndLevelRandomLevelSource *source, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	int rotation = random->nextInt(4);
	int y = getYPositionForFeature(chunkX, chunkZ, source);

	if (y < 10)
	{
		boundingBox = new BoundingBox(chunkX * 16, 0, chunkZ * 16, chunkX * 16, 0, chunkZ * 16);
		return;
	}

	EndCityPieces::Builder builder(pieces, random);
	builder.startHouseTower(chunkX * 16 + 8, y, chunkZ * 16 + 8, rotation);
	calculateBoundingBox();
}
