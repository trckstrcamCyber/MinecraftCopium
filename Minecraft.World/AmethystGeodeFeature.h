#pragma once

#include "Feature.h"

class Level;
class Random;

// One attempt per Overworld chunk; the feature itself applies the 1-in-24 roll.
class AmethystGeodeFeature : public Feature
{
public:
    AmethystGeodeFeature();
    virtual bool place(Level* level, Random* random, int chunkX, int y, int chunkZ) override;
};
