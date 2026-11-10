#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - draws the illagers that share IllagerModel (Vindicator, Evoker). Each type has its own skin.
class IllagerRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation VINDICATOR_LOCATION;
	static ResourceLocation EVOKER_LOCATION;
	static ResourceLocation PILLAGER_LOCATION;

public:
	IllagerRenderer();

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void createArmorParts();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
