#pragma once
#include "Feature.h"

// A small abandoned traveler's tent: a low wool "canvas" hut with fence
// support poles, a chest of early-game loot, a torch, and an optional
// hay bale outside. Rarely, the chest holds a "Traveler's Note" with a
// short flavor-text lore line.
//
// Deliberately built with plain full-block wool rather than stairs/slopes
// to keep this simple and low-risk (no block-orientation guessing) - see
// the class comment in LuckyStructureConfig.h for the rarity model.
class AbandonedTentFeature : public Feature
{
public:
	AbandonedTentFeature();

	virtual bool place(Level *level, Random *random, int x, int y, int z) override;

private:
	void buildTent(Level *level, Random *random, int x, int groundY, int z, int size, int facing);
	void fillChest(Level *level, Random *random, int x, int y, int z);
};
