#include "stdafx.h"
#include "TheEndDimension.h"
#include "FixedBiomeSource.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.storage.h"
#include "EndDragonFight.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/Common/Colours/ColourTable.h"

void TheEndDimension::init()
{
	dragonFight = nullptr;
    biomeSource = new FixedBiomeSource(Biome::sky, 0.5f, 0);
    id = 1;
    hasCeiling = true;
}

ChunkSource *TheEndDimension::createRandomLevelSource() const
{
	if (level != nullptr && level->getLevelData() != nullptr) setWorldSize(level->getLevelData()->getXZSize());
	return new TheEndLevelRandomLevelSource(level, level->getSeed(), isModernEnd());
}

float TheEndDimension::getTimeOfDay(int64_t time, float a) const
{
	return 0.0f;
}

float *TheEndDimension::getSunriseColor(float td, float a)
{
	return nullptr;
}

Vec3 *TheEndDimension::getFogColor(float td, float a) const
{
	int fogColor = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_End_Fog_Colour ); //0xa080a0;
	float br = Mth::cos(td * PI * 2) * 2 + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	float r = ((fogColor >> 16) & 0xff) / 255.0f;
	float g = ((fogColor >> 8) & 0xff) / 255.0f;
	float b = ((fogColor) & 0xff) / 255.0f;
	r *= br * 0.0f + 0.15f;
	g *= br * 0.0f + 0.15f;
	b *= br * 0.0f + 0.15f;

	return Vec3::newTemp(r, g, b);
}

bool TheEndDimension::hasGround()
{
	return false;
}

bool TheEndDimension::mayRespawn() const
{
	return false;
}

bool TheEndDimension::isNaturalDimension()
{
	return false;
}

float TheEndDimension::getCloudHeight()
{
	return 8;
}

bool TheEndDimension::isValidSpawn(int x, int z) const
{
    int topTile = level->getTopTile(x, z);

    if (topTile == 0) return false;

    return Tile::tiles[topTile]->material->blocksMotion();
}

Pos *TheEndDimension::getSpawnPos()
{
	if (isModernEnd()) return new Pos(60, 50, 60);
	return new Pos(100, 50, 0);
}

bool TheEndDimension::isFoggyAt(int x, int z)
{
	return true;
}

int TheEndDimension::getSpawnYPosition()
{
	return 50;
}

const int TheEndDimension::AREA_CHUNK_X[TheEndDimension::AREA_MAX] = { 0, 0, 25, 0, -25 };
const int TheEndDimension::AREA_CHUNK_Z[TheEndDimension::AREA_MAX] = { 0, 25, 0, -25, 0 };
int TheEndDimension::AREA_COUNT = 2;

void TheEndDimension::setWorldSize(int overworldXZSize)
{
#ifdef _LARGE_WORLDS
	AREA_COUNT = overworldXZSize >= LEVEL_WIDTH_LARGE ? AREA_MAX : 2;
#else
	AREA_COUNT = 2;
#endif
}

bool TheEndDimension::isOuterAreaCentre(int chunkX, int chunkZ)
{
	for (int i = 1; i < AREA_COUNT; i++)
	{
		if (chunkX == AREA_CHUNK_X[i] && chunkZ == AREA_CHUNK_Z[i]) return true;
	}
	return false;
}

int TheEndDimension::closestArea(int chunkX, int chunkZ)
{
	int closest = 0;
	int closestDistance = INT_MAX;
	for (int i = 0; i < AREA_COUNT; i++)
	{
		int distance = max(abs(chunkX - AREA_CHUNK_X[i]), abs(chunkZ - AREA_CHUNK_Z[i]));
		if (distance < closestDistance)
		{
			closest = i;
			closestDistance = distance;
		}
	}
	return closest;
}

bool TheEndDimension::isChunkInArea(int chunkX, int chunkZ)
{
	int area = closestArea(chunkX, chunkZ);
	int dx = chunkX - AREA_CHUNK_X[area];
	int dz = chunkZ - AREA_CHUNK_Z[area];
	return dx >= -AREA_HALF_CHUNKS && dx < AREA_HALF_CHUNKS && dz >= -AREA_HALF_CHUNKS && dz < AREA_HALF_CHUNKS;
}

bool TheEndDimension::isModernEnd() const
{
	return level != nullptr && level->getLevelData() != nullptr && level->getLevelData()->isModernEnd();
}

int TheEndDimension::getXZSize()
{
	return isModernEnd() ? END_LEVEL_MODERN_WIDTH : Dimension::getXZSize();
}

bool TheEndDimension::isOutsideWorldEdge(int x, int z)
{
	if (!isModernEnd()) return Dimension::isOutsideWorldEdge(x, z);
	setWorldSize(level->getLevelData()->getXZSize());

	return !isChunkInArea(x >> 4, z >> 4) || Dimension::isOutsideWorldEdge(x, z);
}

EndDragonFight *TheEndDimension::getDragonFight()
{
	if (dragonFight == nullptr) dragonFight = new EndDragonFight(level);
	return dragonFight;
}
