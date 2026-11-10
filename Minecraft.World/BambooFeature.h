#pragma once
#include "Feature.h"

// neoLegacy - a patch of natural Bamboo (Java's BambooFeature, simplified).
//   attempts: how many stalks to try around the patch centre
//   radius:   how far from the centre they may be (keep <= 4 so a patch never leaves the loaded chunks)
//   podzol:   also turns the ground under/around the patch into podzol (Bamboo Jungle)
// Natural stalks are fully grown thick bamboo, 5-16 blocks tall, with the usual leaves on the top three blocks and the
// "finished growing" flag on the top block, like Java.
class BambooFeature : public Feature
{
private:
	int attempts;
	int radius;
	bool podzol;

public:
	BambooFeature(int attempts, int radius, bool podzol);
	virtual bool place(Level *level, Random *random, int x, int y, int z);
};
