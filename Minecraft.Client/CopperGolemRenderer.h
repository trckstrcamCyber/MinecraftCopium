#pragma once

#include "MobRenderer.h"

class CopperGolemModel;

// neoLegacy - Copper Age (Phase 5). Renderer for the copper golem (Java
// CopperGolemRenderer :1551, CopperGolemRenderState :3965). Body and eyes
// textures vary by oxidation state; the eyes are rendered as a second
// pass with additive blending (EndermanRenderer::prepareArmor precedent:
// LivingEntityEmissiveLayer in Java).
class CopperGolemRenderer : public MobRenderer
{
private:
	CopperGolemModel *golemModel;
	static ResourceLocation GOLEM_LOCATION;
	static ResourceLocation GOLEM_EXPOSED_LOCATION;
	static ResourceLocation GOLEM_WEATHERED_LOCATION;
	static ResourceLocation GOLEM_OXIDIZED_LOCATION;
	static ResourceLocation GOLEM_EYES_LOCATION;
	static ResourceLocation GOLEM_EXPOSED_EYES_LOCATION;
	static ResourceLocation GOLEM_WEATHERED_EYES_LOCATION;
	static ResourceLocation GOLEM_OXIDIZED_EYES_LOCATION;

public:
	CopperGolemRenderer();
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual int prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
};
