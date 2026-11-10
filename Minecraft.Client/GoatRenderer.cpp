#include "stdafx.h"
#include "GoatRenderer.h"

ResourceLocation GoatRenderer::GOAT_LOCATION = ResourceLocation(TN_MOB_GOAT);

GoatRenderer::GoatRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void GoatRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation *GoatRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &GOAT_LOCATION;
}
