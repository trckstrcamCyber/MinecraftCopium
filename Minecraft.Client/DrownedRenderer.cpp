#include "stdafx.h"
#include "DrownedRenderer.h"
#include "ZombieModel.h"
#include "HumanoidModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/SharedConstants.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"

ResourceLocation DrownedRenderer::DROWNED_LOCATION = ResourceLocation(TN_MOB_DROWNED);
ResourceLocation DrownedRenderer::DROWNED_OUTER_LOCATION = ResourceLocation(TN_MOB_DROWNED_OUTER);

DrownedRenderer::DrownedRenderer() : HumanoidMobRenderer(new ZombieModel(0.0f, false), 0.5f, 1.0f)
{
	outerModel = new ZombieModel(0.25f, false);		// same shape, inflated by 0.25 like the Java outer layer
	createArmorParts();
}

void DrownedRenderer::createArmorParts()
{
	delete armorParts1;
	delete armorParts2;
	armorParts1 = new ZombieModel(1.0f, true);
	armorParts2 = new ZombieModel(0.5f, true);
}

int DrownedRenderer::prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a)
{
	// Layer 0 is the helmet slot. A drowned without a helmet draws its outer skin there; one with a
	// helmet draws the helmet as usual (the outer skin is skipped, like armor hiding it).
	if (layer == 0 && _mob->getArmor(3) == nullptr && !_mob->isInvisible())
	{
		bindTexture(&DROWNED_OUTER_LOCATION);

		// whole model visible, pose copied from the main model (same as the armor models)
		outerModel->head->visible = true;
		outerModel->hair->visible = true;
		outerModel->body->visible = true;
		outerModel->arm0->visible = true;
		outerModel->arm1->visible = true;
		outerModel->leg0->visible = true;
		outerModel->leg1->visible = true;
		setArmor(outerModel);
		outerModel->attackTime = model->attackTime;
		outerModel->riding = model->riding;
		outerModel->young = model->young;

		float brightness = SharedConstants::TEXTURE_LIGHTING ? 1.0f : _mob->getBrightness(a);
		glColor3f(brightness, brightness, brightness);
		return 1;
	}
	return HumanoidMobRenderer::prepareArmor(_mob, layer, a);
}

ResourceLocation *DrownedRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &DROWNED_LOCATION;
}
