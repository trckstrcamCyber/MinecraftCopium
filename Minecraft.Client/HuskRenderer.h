#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - Husk: a zombie on the modern 64x64 skin layout, drawn 6.25% larger like the Java husk.
class HuskRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation HUSK_LOCATION;

public:
	HuskRenderer();

protected:
	virtual void createArmorParts();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
};
