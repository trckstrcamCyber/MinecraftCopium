#pragma once

#include "Biome.h"

class JungleBiome : public Biome
{
private:
	bool isEdge;
	bool isBamboo;	// neoLegacy - Bamboo Jungle variant: dense bamboo, podzol, fewer trees

public:
	JungleBiome(int id, bool isEdge, bool isBamboo = false);

	virtual Feature *getTreeFeature(Random *random) override;
	virtual Feature *getGrassFeature(Random *random) override;
	virtual void decorate(Level *level, Random *random, int xo, int zo) override;
};