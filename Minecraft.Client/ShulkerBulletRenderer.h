#pragma once
#include "EntityRenderer.h"

class Model;
class ModelPart;

class ShulkerBulletRenderer : public EntityRenderer
{
private:
	static ResourceLocation SPARK_LOCATION;
	Model *bulletModel;
	ModelPart *bulletPart;

public:
	ShulkerBulletRenderer();

	virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
