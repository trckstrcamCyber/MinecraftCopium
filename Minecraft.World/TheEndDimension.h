#pragma once
#include "Dimension.h"

class EndDragonFight;

class TheEndDimension : public Dimension
{
public:
	virtual void init();
	virtual ChunkSource *createRandomLevelSource() const;
	virtual float getTimeOfDay(int64_t time, float a) const;
	virtual float *getSunriseColor(float td, float a);
	virtual Vec3 *getFogColor(float td, float a) const;
	virtual bool hasGround();
	virtual bool mayRespawn() const;
	virtual bool isNaturalDimension();
	virtual float getCloudHeight();
	virtual bool isValidSpawn(int x, int z) const;
	virtual Pos *getSpawnPos();
	virtual int getSpawnYPosition();
	virtual bool isFoggyAt(int x, int z);

	static const int AREA_MAX = 5;
	static const int AREA_CHUNK_X[AREA_MAX];
	static const int AREA_CHUNK_Z[AREA_MAX];
	static int AREA_COUNT;
	static void setWorldSize(int overworldXZSize);
	static bool isOuterAreaCentre(int chunkX, int chunkZ);
	static const int AREA_HALF_CHUNKS = 6;

	static int closestArea(int chunkX, int chunkZ);
	static bool isChunkInArea(int chunkX, int chunkZ);
	bool isModernEnd() const;
	EndDragonFight *getDragonFight();

	virtual int getXZSize();
	virtual bool isOutsideWorldEdge(int x, int z);

private:
	EndDragonFight *dragonFight;
};
