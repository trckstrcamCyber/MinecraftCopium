#include "stdafx.h"
#include "IllagerRenderer.h"
#include "IllagerModel.h"
#include "ZombieModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"
#include "../Minecraft.World/Illager.h"

ResourceLocation IllagerRenderer::VINDICATOR_LOCATION = ResourceLocation(TN_MOB_VINDICATOR);
ResourceLocation IllagerRenderer::EVOKER_LOCATION = ResourceLocation(TN_MOB_EVOKER);
ResourceLocation IllagerRenderer::PILLAGER_LOCATION = ResourceLocation(TN_MOB_PILLAGER);
ResourceLocation IllagerRenderer::PROVOKER_LOCATION = ResourceLocation(TN_MOB_PROVOKER);
ResourceLocation IllagerRenderer::SORCERER_LOCATION = ResourceLocation(TN_MOB_SORCERER);
ResourceLocation IllagerRenderer::ALCHEMIST_LOCATION = ResourceLocation(TN_MOB_ALCHEMIST);

// illagers are drawn at 15/16 size like in Java Edition
IllagerRenderer::IllagerRenderer(int texW, int texH, int hatKind) : HumanoidMobRenderer(new IllagerModel(texW, texH, static_cast<IllagerModel::Hat>(hatKind)), 0.5f, 0.9375f)
{
	createArmorParts();
}

// the weapon is only held out while the arms are not folded in front of the chest
void IllagerRenderer::additionalRendering(shared_ptr<LivingEntity> mob, float a)
{
	shared_ptr<Illager> illager = dynamic_pointer_cast<Illager>(mob);
	if (illager != nullptr && illager->getArmPose() == Illager::CROSSED) return;
	HumanoidMobRenderer::additionalRendering(mob, a);
}

void IllagerRenderer::createArmorParts()
{
	delete armorParts1;
	delete armorParts2;
	armorParts1 = new ZombieModel(1.0f, true);
	armorParts2 = new ZombieModel(0.5f, true);
}

ResourceLocation *IllagerRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	if (mob->GetType() == eTYPE_EVOKER) return &EVOKER_LOCATION;
	if (mob->GetType() == eTYPE_PILLAGER) return &PILLAGER_LOCATION;
	if (mob->GetType() == eTYPE_PROVOKER) return &PROVOKER_LOCATION;
	if (mob->GetType() == eTYPE_SORCERER) return &SORCERER_LOCATION;
	if (mob->GetType() == eTYPE_ALCHEMIST) return &ALCHEMIST_LOCATION;
	return &VINDICATOR_LOCATION;
}
