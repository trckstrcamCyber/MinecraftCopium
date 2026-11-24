#pragma once

#include "StructureFeature.h"
#include "StructureStart.h"

class TheEndLevelRandomLevelSource;

class EndCityFeature : public StructureFeature
{
private:
	TheEndLevelRandomLevelSource *source;

public:
	EndCityFeature(TheEndLevelRandomLevelSource *source);

	wstring getFeatureName();

	static int getYPositionForFeature(int chunkX, int chunkZ, TheEndLevelRandomLevelSource *source);

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
	virtual StructureStart *createStructureStart(int x, int z);

public:
	class EndCityStart : public StructureStart
	{
	public:
		static StructureStart *Create() { return new EndCityStart(); }
		virtual EStructureStart GetType() { return eStructureStart_EndCity; }

		EndCityStart();
		EndCityStart(Level *level, TheEndLevelRandomLevelSource *source, Random *random, int chunkX, int chunkZ);
	};
};
