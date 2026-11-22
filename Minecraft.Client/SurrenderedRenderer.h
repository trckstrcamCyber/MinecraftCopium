#pragma once
#include "HumanoidMobRenderer.h"
#include "SkeletonModel.h"

// neoLegacy - the Surrendered (the Invoker's Vex servants) drawn as pale skeletons: the ordinary skeleton model on the mod's own skin, with a second skin while it charges
// and its arms thrown out in front of it. It floats a little lower than a Skeleton's feet would.
class SurrenderedModel : public SkeletonModel
{
public:
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
};

class SurrenderedRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation SURRENDERED_LOCATION;
	static ResourceLocation CHARGE_LOCATION;

public:
	SurrenderedRenderer();

protected:
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
};
