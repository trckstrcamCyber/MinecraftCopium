#pragma once
#include "MobRenderer.h"

class WanderingTraderRenderer : public MobRenderer
{
private:
	static ResourceLocation TRADER_LOCATION;

public:
	WanderingTraderRenderer();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
};
