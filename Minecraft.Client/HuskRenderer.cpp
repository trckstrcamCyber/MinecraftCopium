#include "stdafx.h"
#include "HuskRenderer.h"
#include "ZombieModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"

ResourceLocation HuskRenderer::HUSK_LOCATION = ResourceLocation(TN_MOB_HUSK);

HuskRenderer::HuskRenderer() : HumanoidMobRenderer(new ZombieModel(), 0.5f, 1.0f)
{
	createArmorParts();
}

void HuskRenderer::createArmorParts()
{
	delete armorParts1;
	delete armorParts2;
	armorParts1 = new ZombieModel(1.0f, true);
	armorParts2 = new ZombieModel(0.5f, true);
}

ResourceLocation *HuskRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &HUSK_LOCATION;
}

void HuskRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	HumanoidMobRenderer::scale(mob, a);
	glScalef(1.0625f, 1.0625f, 1.0625f);
}
