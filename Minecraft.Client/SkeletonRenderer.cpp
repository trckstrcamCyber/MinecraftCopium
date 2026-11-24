#include "stdafx.h"
#include "SkeletonRenderer.h"
#include "SkeletonModel.h"
#include "../Minecraft.World/Skeleton.h"

ResourceLocation SkeletonRenderer::SKELETON_LOCATION = ResourceLocation(TN_MOB_SKELETON);
ResourceLocation SkeletonRenderer::WITHER_SKELETON_LOCATION = ResourceLocation(TN_MOB_WITHER_SKELETON);
ResourceLocation SkeletonRenderer::STRAY_LOCATION = ResourceLocation(TN_MOB_STRAY);
ResourceLocation SkeletonRenderer::STRAY_CLOTHES_LOCATION = ResourceLocation(TN_MOB_STRAY_OVERLAY);

SkeletonRenderer::SkeletonRenderer() : HumanoidMobRenderer(new SkeletonModel(), .5f)
{
	strayClothesModel = new SkeletonModel(0.25f);
}

void SkeletonRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
    if (dynamic_pointer_cast<Skeleton>(mob)->getSkeletonType() == Skeleton::TYPE_WITHER)
	{
        glScalef(1.2f, 1.2f, 1.2f);
    }
}

void SkeletonRenderer::translateWeaponItem()
{
    glTranslatef(1.5f / 16.0f, 3 / 16.0f, 0);
}

ResourceLocation *SkeletonRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<Skeleton> skeleton = dynamic_pointer_cast<Skeleton>(entity);

    if (skeleton->getSkeletonType() == Skeleton::TYPE_WITHER)
	{
        return &WITHER_SKELETON_LOCATION;
    }
    if (skeleton->getSkeletonType() == Skeleton::TYPE_STRAY)
	{
        return &STRAY_LOCATION;
    }
    return &SKELETON_LOCATION;
}
void SkeletonRenderer::renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	HumanoidMobRenderer::renderModel(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale);

	shared_ptr<Skeleton> skeleton = dynamic_pointer_cast<Skeleton>(mob);
	if (skeleton == nullptr || skeleton->getSkeletonType() != Skeleton::TYPE_STRAY || mob->isInvisible()) return;

	HumanoidModel *body = static_cast<HumanoidModel *>(resModel);
	strayClothesModel->attackTime = body->attackTime;
	strayClothesModel->riding = body->riding;
	strayClothesModel->young = body->young;
	strayClothesModel->holdingLeftHand = body->holdingLeftHand;
	strayClothesModel->holdingRightHand = body->holdingRightHand;
	strayClothesModel->sneaking = body->sneaking;
	strayClothesModel->bowAndArrow = body->bowAndArrow;

	bindTexture(&STRAY_CLOTHES_LOCATION);
	strayClothesModel->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
}
