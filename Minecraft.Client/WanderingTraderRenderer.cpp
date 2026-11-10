#include "stdafx.h"
#include "WanderingTraderRenderer.h"
#include "WanderingTraderModel.h"

ResourceLocation WanderingTraderRenderer::TRADER_LOCATION = ResourceLocation(TN_MOB_WANDERING_TRADER);

WanderingTraderRenderer::WanderingTraderRenderer() : MobRenderer(new WanderingTraderModel(), 0.5f)
{
}

ResourceLocation *WanderingTraderRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &TRADER_LOCATION;
}

void WanderingTraderRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	float s = 15 / 16.0f;		// same size as a villager
	glScalef(s, s, s);
}
