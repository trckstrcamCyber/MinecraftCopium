#include "stdafx.h"
#include "IceologerRenderer.h"
#include "IceologerModel.h"

ResourceLocation IceologerRenderer::ICEOLOGER_LOCATION = ResourceLocation(TN_MOB_ICEOLOGER);

IceologerRenderer::IceologerRenderer() : MobRenderer(new IceologerModel(), 0.5f)
{
}

void IceologerRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glScalef(0.9375f, 0.9375f, 0.9375f);
}

ResourceLocation *IceologerRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &ICEOLOGER_LOCATION;
}
