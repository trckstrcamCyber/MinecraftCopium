#pragma once
#include "EntityRenderer.h"

class EvokerFangsModel;

class EvokerFangsRenderer : public EntityRenderer
{
private:
	static ResourceLocation FANGS_LOCATION;
	EvokerFangsModel *model;
public:
	EvokerFangsRenderer();
	virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
