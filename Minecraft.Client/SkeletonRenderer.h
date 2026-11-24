#pragma once
#include "HumanoidMobRenderer.h"

class SkeletonModel;

class SkeletonRenderer : public HumanoidMobRenderer
{
private:
    static ResourceLocation SKELETON_LOCATION;
    static ResourceLocation WITHER_SKELETON_LOCATION;
    static ResourceLocation STRAY_LOCATION;
    static ResourceLocation STRAY_CLOTHES_LOCATION;

	SkeletonModel *strayClothesModel;

public:
	SkeletonRenderer();

protected:
    virtual void scale(shared_ptr<LivingEntity> mob, float a);
    void translateWeaponItem();
    virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	virtual void renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
};
