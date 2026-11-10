#pragma once
#include "Feature.h"

// Seven small (3-10 block) exploration mini-structures. Each is its own
// independent Feature with its own rarity (see LuckyStructureConfig.h)
// and biome rule, so they can be tuned or disabled individually. None
// carry powerful loot - these are atmosphere, not reward.

class BrokenCartFeature : public Feature
{
public:
	BrokenCartFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class OreShrineFeature : public Feature
{
public:
	OreShrineFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class RuinedFenceFeature : public Feature
{
public:
	RuinedFenceFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class AbandonedCampfireFeature : public Feature
{
public:
	AbandonedCampfireFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class RockPileFeature : public Feature
{
public:
	RockPileFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class HunterPostFeature : public Feature
{
public:
	HunterPostFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};

class MiniCaveFeature : public Feature
{
public:
	MiniCaveFeature();
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};
