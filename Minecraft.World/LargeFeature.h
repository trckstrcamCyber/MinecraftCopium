#pragma once
#include "ChunkSource.h"

class Level;

class LargeFeature
{
public:
	static const wstring STRONGHOLD;
	static const wstring VILLAGE;	// neoLegacy - for /locate
	static const wstring MINESHAFT;	// neoLegacy - for /locate
	static const wstring MONUMENT;		// neoLegacy - for /locate
	static const wstring TEMPLE;		// neoLegacy - for /locate
	static const wstring OUTPOST;		// neoLegacy - for /locate
	static const wstring ILLAGER_STRUCTURE;	// neoLegacy - for /locate: the nearest Sorcerer Hut, Illusioner Tower or Illager Fort

	// neoLegacy - /locate: how many chunks StructureFeature::searchForGeneratedFeature
	// spirals outward before giving up. 64 chunks (1024 blocks) comfortably covers
	// Village's spacing (16-32 chunks) even from a bad starting position relative to
	// its placement grid.
	static const int LOCATE_SEARCH_RADIUS_CHUNKS = 64;
protected:
    int radius;
    Random *random;
	Level *level;

public:
	LargeFeature();
	~LargeFeature();

    virtual void apply(ChunkSource *ChunkSource, Level *level, int xOffs, int zOffs, byteArray blocks);

protected:
	virtual void addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks) {}
};