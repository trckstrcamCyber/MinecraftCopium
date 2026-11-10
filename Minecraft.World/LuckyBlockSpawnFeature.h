#pragma once
#include "Feature.h"

// Places a single Lucky Block on the surface, extremely rarely, in safe
// overworld biomes only. No particles/aura - optionally places a colored
// wool block directly underneath as a visible "Lucky Marker" instead.
// See LuckyStructureConfig.h for all the tunable knobs.
class LuckyBlockSpawnFeature : public Feature
{
public:
	LuckyBlockSpawnFeature();

	// x, z are the candidate column; y is ignored - the feature finds its
	// own safe surface height.
	virtual bool place(Level *level, Random *random, int x, int y, int z) override;
};
