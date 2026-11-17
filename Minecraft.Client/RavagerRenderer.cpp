#include "stdafx.h"
#include "RavagerModel.h"
#include "RavagerRenderer.h"

ResourceLocation RavagerRenderer::RAVAGER_LOCATION = ResourceLocation(TN_MOB_RAVAGER);

RavagerRenderer::RavagerRenderer() : MobRenderer(new RavagerModel(), 1.1f)
{
}

ResourceLocation *RavagerRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &RAVAGER_LOCATION;
}
