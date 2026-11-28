#pragma once
#include "MobRenderer.h"
#include "ResourceLocation.h"
class HellFireRenderer : public MobRenderer
{
private:
    static ResourceLocation WILDFIRE_LOCATION;
    static ResourceLocation SOULFIRE_LOCATION;
public:
    HellFireRenderer();
    virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
    virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
protected:
    virtual void scale(shared_ptr<LivingEntity> entity, float a);
};
