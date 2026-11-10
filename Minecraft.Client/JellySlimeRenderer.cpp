#include "stdafx.h"
#include "JellySlimeRenderer.h"

ResourceLocation JellySlimeRenderer::JELLY_SLIME_LOCATION = ResourceLocation(TN_MOB_JELLY_SLIME);

JellySlimeRenderer::JellySlimeRenderer(Model *model, Model *armor, float shadow) : SlimeRenderer(model, armor, shadow)
{
}

ResourceLocation *JellySlimeRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &JELLY_SLIME_LOCATION;
}
