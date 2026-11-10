#pragma once
#include "MobRenderer.h"

class CamelRenderer : public MobRenderer
{
private:
	static ResourceLocation CAMEL_LOCATION;
	static ResourceLocation SADDLE_LOCATION;

public:
	CamelRenderer(Model *model, float shadow);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	// The saddle + reins are a second, slightly inflated copy of the camel drawn with the saddle skin.
	virtual int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);
};
