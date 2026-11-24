#pragma once
#include "MobRenderer.h"

class ShulkerModel;

class ShulkerRenderer : public MobRenderer
{
private:
	static ResourceLocation SHULKER_LOCATION;
	ShulkerModel *shulkerModel;

public:
	ShulkerRenderer();

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual void setupRotations(shared_ptr<LivingEntity> mob, float bob, float bodyRot, float a);
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
};
