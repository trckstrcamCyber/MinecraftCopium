#pragma once
#include "VillagerModel.h"

// neoLegacy - the Wandering Trader uses the villager body plus the hood layer baked into its texture
// (an inflated copy of the head box at (32,0), like the Java villager "hat" part).
class WanderingTraderModel : public VillagerModel
{
public:
	ModelPart *hood;
	WanderingTraderModel();
};
