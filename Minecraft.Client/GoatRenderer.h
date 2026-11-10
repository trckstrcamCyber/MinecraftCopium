#pragma once
#include "MobRenderer.h"

class GoatRenderer : public MobRenderer
{
private:
	static ResourceLocation GOAT_LOCATION;

public:
	GoatRenderer(Model *model, float shadow);

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
