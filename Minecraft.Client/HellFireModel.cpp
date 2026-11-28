#include "stdafx.h"
#include "HellFireModel.h"
#include "../Minecraft.World/HellFireBoss.h"
#include "ModelPart.h"

HellFireModel::HellFireModel() : Model()
{
    texWidth = 64;
    texHeight = 64;

    // ---------------------------------------------------------------------
    // Stage 1: Wildfire. This follows wildfire_boss.java exactly, including
    // the nested underbite, shield pivots, CubeDeformation and UV offsets.
    // The Blockbench root is positioned at (0, 24, 0).
    // ---------------------------------------------------------------------
    wildfireRoot = new ModelPart(this, 0, 0);
    wildfireRoot->setPos(0.0f, 24.0f, 0.0f);

    wildfireTorso = new ModelPart(this, 32, 12);
    wildfireTorso->addBox(-2.0f, -10.5f, -2.0f, 4, 21, 4, 0.0f);
    wildfireTorso->setPos(0.0f, -11.5f, 0.0f);
    wildfireRoot->addChild(wildfireTorso);

    wildfireHead = new ModelPart(this, 0, 0);
    wildfireHead->addBox(-4.0f, -6.625f, -4.0f, 8, 8, 8, 0.0f);
    wildfireHead->texOffs(0, 16)->addBox(-4.0f, -6.625f, -4.0f, 8, 8, 8, 0.5f);
    wildfireHead->texOffs(32, 0)->addBox(-8.0f, -10.625f, 0.0f, 16, 12, 0, 0.0f);
    wildfireHead->setPos(0.0f, -25.375f, 0.0f);
    wildfireRoot->addChild(wildfireHead);

    wildfireUnderbite = new ModelPart(this, 0, 32);
    wildfireUnderbite->addBox(-4.0f, -7.0f, -7.0f, 8, 9, 8, 0.25f);
    wildfireUnderbite->setPos(0.0f, -0.625f, 3.0f);
    wildfireHead->addChild(wildfireUnderbite);

    wildfireShieldRoot = new ModelPart(this, 0, 0);
    wildfireShieldRoot->setPos(0.0f, -20.0f, 0.0f);
    wildfireRoot->addChild(wildfireShieldRoot);

    wildfireShields = ModelPartArray(4);

    // Shield A: front (-Z)
    wildfireShields[0] = new ModelPart(this, 34, 41);
    wildfireShields[0]->addBox(-6.0f, -10.0f, -1.5f, 12, 20, 3, 0.0f);
    wildfireShields[0]->setPos(0.0f, 7.0f, -10.5f);
    wildfireShields[0]->xRot = -0.1309f;
    wildfireShieldRoot->addChild(wildfireShields[0]);

    // Shield B: right (+X)
    wildfireShields[1] = new ModelPart(this, 34, 41);
    wildfireShields[1]->addBox(-6.0f, -10.0f, -1.5f, 12, 20, 3, 0.0f);
    wildfireShields[1]->setPos(10.5f, 7.0f, 0.0f);
    wildfireShields[1]->yRot = -1.5708f;
    wildfireShields[1]->zRot = -0.1309f;
    wildfireShieldRoot->addChild(wildfireShields[1]);

    // Shield C: left (-X)
    wildfireShields[2] = new ModelPart(this, 34, 41);
    wildfireShields[2]->addBox(-6.0f, -10.0f, -1.5f, 12, 20, 3, 0.0f);
    wildfireShields[2]->setPos(-10.5f, 7.0f, 0.0f);
    wildfireShields[2]->yRot = 1.5708f;
    wildfireShields[2]->zRot = 0.1309f;
    wildfireShieldRoot->addChild(wildfireShields[2]);

    // Shield D: rear (+Z)
    wildfireShields[3] = new ModelPart(this, 34, 41);
    wildfireShields[3]->addBox(-6.0f, -10.0f, -1.5f, 12, 20, 3, 0.0f);
    wildfireShields[3]->setPos(0.0f, 7.0f, 10.5f);
    wildfireShields[3]->xRot = 3.0107f;
    wildfireShields[3]->zRot = 3.1416f;
    wildfireShieldRoot->addChild(wildfireShields[3]);

    // ---------------------------------------------------------------------
    // Stage 2: Soulfire. This follows soulfire_boss.java exactly, including
    // the floating shield array, three-part shield surfaces and UV layout.
    // ---------------------------------------------------------------------
    soulfireRoot = new ModelPart(this, 0, 0);
    soulfireRoot->setPos(0.0f, 24.0f, 0.0f);

    soulfireTorso = new ModelPart(this, 48, 39);
    soulfireTorso->addBox(-2.0f, -11.5f, -2.0f, 4, 21, 4, 0.0f);
    soulfireTorso->setPos(0.0f, -11.5f, 0.0f);
    soulfireRoot->addChild(soulfireTorso);

    soulfireHead = new ModelPart(this, 0, 0);
    soulfireHead->addBox(-4.0f, -7.625f, -4.0f, 8, 8, 8, 0.0f);
    soulfireHead->texOffs(32, 0)->addBox(-4.0f, 0.375f, -4.0f, 8, 4, 8, 0.0f);
    soulfireHead->texOffs(0, 16)->addBox(-4.0f, -7.625f, -4.0f, 8, 8, 8, 0.5f);
    soulfireHead->texOffs(0, 52)->addBox(-8.0f, -11.625f, 0.0f, 16, 12, 0, 0.0f);
    soulfireHead->setPos(0.0f, -25.375f, 0.0f);
    soulfireRoot->addChild(soulfireHead);

    soulfireShieldRoot = new ModelPart(this, 0, 0);
    soulfireShieldRoot->setPos(0.0f, -15.5f, 0.0f);
    soulfireRoot->addChild(soulfireShieldRoot);

    soulfireShields = ModelPartArray(4);

    for (int i = 0; i < 4; ++i)
    {
        soulfireShields[i] = new ModelPart(this, 0, 32);
        soulfireShields[i]->addBox(-5.0f, -10.5f, -1.0f, 10, 4, 2, 0.0f);
        soulfireShields[i]->texOffs(20, 40)->addBox(-6.0f, -6.5f, -1.0f, 12, 10, 2, 0.0f);
        soulfireShields[i]->texOffs(0, 40)->addBox(-4.0f, 3.5f, -1.0f, 8, 10, 2, 0.0f);
        soulfireShieldRoot->addChild(soulfireShields[i]);
    }

    // Soulfire shield A: front (-Z)
    soulfireShields[0]->setPos(0.0f, -1.0f, -10.0f);
    soulfireShields[0]->xRot = 0.1309f;

    // Soulfire shield B: right (+X)
    soulfireShields[1]->setPos(10.0f, -1.0f, 0.0f);
    soulfireShields[1]->yRot = -1.5708f;
    soulfireShields[1]->zRot = 0.1309f;

    // Soulfire shield C: rear (+Z)
    soulfireShields[2]->setPos(0.0f, -1.0f, 10.0f);
    soulfireShields[2]->xRot = -3.0107f;
    soulfireShields[2]->zRot = 3.1416f;

    // Soulfire shield D: left (-X)
    soulfireShields[3]->setPos(-10.0f, -1.0f, 0.0f);
    soulfireShields[3]->yRot = 1.5708f;
    soulfireShields[3]->zRot = -0.1309f;

    // Compile every part in advance. ModelPart compiles only its own cubes;
    // parent-child transforms remain live and are applied by render().
    wildfireRoot->compile(1.0f / 16.0f);
    wildfireTorso->compile(1.0f / 16.0f);
    wildfireHead->compile(1.0f / 16.0f);
    wildfireUnderbite->compile(1.0f / 16.0f);
    wildfireShieldRoot->compile(1.0f / 16.0f);
    for (int i = 0; i < 4; ++i)
        wildfireShields[i]->compile(1.0f / 16.0f);

    soulfireRoot->compile(1.0f / 16.0f);
    soulfireTorso->compile(1.0f / 16.0f);
    soulfireHead->compile(1.0f / 16.0f);
    soulfireShieldRoot->compile(1.0f / 16.0f);
    for (int i = 0; i < 4; ++i)
        soulfireShields[i]->compile(1.0f / 16.0f);
}

int HellFireModel::modelVersion()
{
    // Bump this whenever the runtime geometry or UV layout changes.
    return 2;
}

void HellFireModel::render(shared_ptr<Entity> entity, float time, float r, float bob,
                           float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    shared_ptr<HellFireBoss> boss = dynamic_pointer_cast<HellFireBoss>(entity);
    if (boss != nullptr && boss->getPhase() == HellFireBoss::SOULFIRE)
        soulfireRoot->render(scale, usecompiled);
    else
        wildfireRoot->render(scale, usecompiled);
}

void HellFireModel::setupAnim(float time, float r, float bob, float yRot, float xRot,
                              float scale, shared_ptr<Entity> entity,
                              unsigned int uiBitmaskOverrideAnim)
{
    shared_ptr<HellFireBoss> boss = dynamic_pointer_cast<HellFireBoss>(entity);
    bool soul = boss != nullptr && boss->getPhase() == HellFireBoss::SOULFIRE;

    const float angleY = yRot / (180.0f / PI);
    const float angleX = xRot / (180.0f / PI);

    if (!soul)
    {
        // Keep the heavy Wildfire pose grounded and let the head track its target.
        wildfireRoot->y = 24.0f;
        wildfireHead->yRot = angleY;
        wildfireHead->xRot = angleX;
    }
    else
    {
        // Soulfire floats independently of the grounded first phase. The shields
        // gently orbit while retaining the exact Blockbench base orientations.
        const float floatOffset = cos(bob * 0.16f) * 1.15f;
        soulfireRoot->y = 24.0f + floatOffset;
        soulfireRoot->yRot = sin(time * 0.035f) * 0.035f;
        soulfireHead->yRot = angleY;
        soulfireHead->xRot = angleX;

        soulfireShields[0]->xRot = 0.1309f + sin(time * 0.08f) * 0.035f;
        soulfireShields[1]->yRot = -1.5708f + sin(time * 0.08f + 1.0f) * 0.035f;
        soulfireShields[2]->xRot = -3.0107f + sin(time * 0.08f + 2.0f) * 0.035f;
        soulfireShields[3]->yRot = 1.5708f + sin(time * 0.08f + 3.0f) * 0.035f;
    }
}
