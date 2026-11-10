#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - Vex: drawn at 40% size, with red eyes while it is charging.
class VexRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation VEX_LOCATION;
	static ResourceLocation VEX_CHARGING_LOCATION;

public:
	VexRenderer();

protected:
	virtual void createArmorParts();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
