#include "stdafx.h"
#include "FrostbiteRenderer.h"
#include "FrostbiteModel.h"
#include "ZombieModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"

ResourceLocation FrostbiteRenderer::FROSTBITE_LOCATION = ResourceLocation(TN_MOB_FROSTBITE);

FrostbiteRenderer::FrostbiteRenderer() : HumanoidMobRenderer(new FrostbiteModel(), 0.5f, 1.0f)
{
	createArmorParts();
}

void FrostbiteRenderer::createArmorParts()
{
	delete armorParts1;
	delete armorParts2;
	armorParts1 = new ZombieModel(1.0f, true);
	armorParts2 = new ZombieModel(0.5f, true);
}

ResourceLocation *FrostbiteRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &FROSTBITE_LOCATION;
}
