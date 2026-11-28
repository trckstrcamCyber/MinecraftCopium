#include "stdafx.h"
#include "HellFireRenderer.h"
#include "HellFireModel.h"
#include "../Minecraft.World/HellFireBoss.h"
#include "BossMobGuiInfo.h"
ResourceLocation HellFireRenderer::WILDFIRE_LOCATION = ResourceLocation(TN_MOB_HELLFIRE_WILDFIRE);
ResourceLocation HellFireRenderer::SOULFIRE_LOCATION = ResourceLocation(TN_MOB_HELLFIRE_SOULFIRE);
HellFireRenderer::HellFireRenderer() : MobRenderer(new HellFireModel(), 1.2f) {}
ResourceLocation *HellFireRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
    shared_ptr<HellFireBoss> boss = dynamic_pointer_cast<HellFireBoss>(entity);
    return boss != nullptr && boss->getPhase() == HellFireBoss::SOULFIRE ? &SOULFIRE_LOCATION : &WILDFIRE_LOCATION;
}
void HellFireRenderer::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a)
{
    shared_ptr<HellFireBoss> boss = dynamic_pointer_cast<HellFireBoss>(entity);
    if (boss != nullptr) BossMobGuiInfo::setBossHealth(boss, true);
    MobRenderer::render(entity, x, y, z, rot, a);
}
void HellFireRenderer::scale(shared_ptr<LivingEntity> entity, float a)
{
    shared_ptr<HellFireBoss> boss = dynamic_pointer_cast<HellFireBoss>(entity);
    float s = boss != nullptr && boss->getPhase() == HellFireBoss::SOULFIRE ? 1.20f : 1.12f;
    glScalef(s, s, s);
}
