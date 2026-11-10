#pragma once
#include "LargeFeature.h"
#include "StructureFeatureSavedData.h"

class StructureStart;

//#define ENABLE_STRUCTURE_SAVING

class StructureFeature : public LargeFeature
{
public:
	// 4J added - Maps to values in the game rules xml
	enum EFeatureTypes
	{
		eFeature_Mineshaft,
		eFeature_NetherBridge,
		eFeature_Temples,
		eFeature_Stronghold,
		eFeature_Village,
	};

#ifdef ENABLE_STRUCTURE_SAVING
private:
	shared_ptr<StructureFeatureSavedData> savedData;
#endif


protected:
	unordered_map<int64_t, StructureStart *> cachedStructures;

public:
	StructureFeature();
	~StructureFeature();

	virtual wstring getFeatureName() = 0;

	virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks);

	bool postProcess(Level *level, Random *random, int chunkX, int chunkZ);
	bool isIntersection(int cellX, int cellZ);

	bool isInsideFeature(int cellX, int cellY, int cellZ);

protected:
	StructureStart *getStructureAt(int cellX, int cellY, int cellZ);

public:
	bool isInsideBoundingFeature(int cellX, int cellY, int cellZ);
	TilePos *getNearestGeneratedFeature(Level *level, int cellX, int cellY, int cellZ);

protected:
	// neoLegacy fix - this was missing `virtual`, so StrongholdFeature's own override of
	// this (a real, already-written fallback to its precomputed stronghold positions) was
	// silently never called: getNearestGeneratedFeature() below calls this from inside a
	// StructureFeature member function, so without `virtual` the compiler always resolved
	// it to this base implementation (return nullptr) at compile time, regardless of the
	// object's actual runtime type. This is why /locate stronghold could never fall back
	// to a guessed position and just kept reporting "not found".
	virtual vector<TilePos> *getGuesstimatedFeaturePositions();

	// neoLegacy - /locate. getNearestGeneratedFeature() below only looks at chunks that
	// have actually been decorated during real gameplay (plus one speculative check at
	// the query position's own chunk) -- for anything spaced further out than whatever
	// area has already been explored, it always reports "not found", which for something
	// like a Village (16-32 chunk spacing) is very easy to hit. isFeatureChunk() is a
	// pure function of the level seed and chunk x/z -- it doesn't need the chunk to
	// actually be generated -- so it's safe to call it speculatively for chunks the
	// player has never been near, same as real Minecraft's own /locate. This spirals
	// outward chunk by chunk (each one cached by addFeature() the first time it's
	// checked, so repeat searches don't redo the work) until it finds a match or gives
	// up at searchRadiusChunks.
public:
	TilePos *searchForGeneratedFeature(Level *level, int cellX, int cellY, int cellZ, int searchRadiusChunks);

private:
	virtual void restoreSavedData(Level *level);
	virtual void saveFeature(int chunkX, int chunkZ, StructureStart *feature);

	/**
	* Returns true if the given chunk coordinates should hold a structure
	* source.
	*
	* @param x
	*            chunk x
	* @param z
	*            chunk z
	* @return
	*/
protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat=false) = 0;

	/**
	* Creates a new instance of a structure source at the given chunk
	* coordinates.
	*
	* @param x
	*            chunk x
	* @param z
	*            chunk z
	* @return
	*/
	virtual StructureStart *createStructureStart(int x, int z) = 0;
};
