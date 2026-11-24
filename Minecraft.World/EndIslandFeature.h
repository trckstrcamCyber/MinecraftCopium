#pragma once
#include "Feature.h"

class EndIslandFeature : public Feature
{
public:
	virtual bool place(Level *level, Random *random, int x, int y, int z);
};
