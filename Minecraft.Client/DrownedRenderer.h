#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - Drowned: zombie body on the modern 64x64 skin plus the translucent-cutout "outer layer"
// skin (barnacle-covered clothes) drawn as an inflated second model on top.
class DrownedRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation DROWNED_LOCATION;
	static ResourceLocation DROWNED_OUTER_LOCATION;

	HumanoidModel *outerModel;

public:
	DrownedRenderer();

protected:
	virtual void createArmorParts();
	virtual int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
