#pragma once
#include "EntityRenderer.h"

class IceCubeModel;

class IceCubeRenderer : public EntityRenderer
{
private:
	static ResourceLocation ICE_CUBE_LOCATION;
	IceCubeModel *model;
public:
	IceCubeRenderer();
	virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
