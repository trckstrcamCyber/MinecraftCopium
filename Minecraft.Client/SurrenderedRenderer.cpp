#include "stdafx.h"
#include "SurrenderedRenderer.h"
#include "../Minecraft.World/Vex.h"

ResourceLocation SurrenderedRenderer::SURRENDERED_LOCATION = ResourceLocation(TN_MOB_SURRENDERED);
ResourceLocation SurrenderedRenderer::CHARGE_LOCATION = ResourceLocation(TN_MOB_SURRENDERED_CHARGE);

// charging: both arms out in front (Java: the Vex pose, 3.77 radians)
void SurrenderedModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	SkeletonModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	shared_ptr<Vex> vex = dynamic_pointer_cast<Vex>(entity);
	if (vex != nullptr && vex->isCharging())
	{
		arm0->xRot = 3.77f;
		arm1->xRot = 3.77f;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}
}

SurrenderedRenderer::SurrenderedRenderer() : HumanoidMobRenderer(new SurrenderedModel(), 0.3f)
{
}

void SurrenderedRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glTranslatef(0.0f, -0.35f, 0.0f);
}

ResourceLocation *SurrenderedRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<Vex> vex = dynamic_pointer_cast<Vex>(entity);
	if (vex != nullptr && vex->isCharging()) return &CHARGE_LOCATION;
	return &SURRENDERED_LOCATION;
}
