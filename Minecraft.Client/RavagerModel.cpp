#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"
#include "../Minecraft.World/Mth.h"
#include "ModelPart.h"
#include "RavagerModel.h"

const float RavagerModel::NECK_REST_Z = -20.0f;

RavagerModel::RavagerModel()
{
	const int tw = 128;
	const int th = 128;

	body = (new ModelPart(this))->setTexSize(tw, th);
	body->setPos(0, 5, 2);
	body->texOffs(0, 55)->addBox(-7, -7, -4, 14, 16, 20, 0.0f);
	body->texOffs(0, 91)->addBox(-6, 9, -4, 12, 13, 18, 0.0f);
	body->xRot = 1.5708f;

	neck = (new ModelPart(this))->setTexSize(tw, th);
	neck->setPos(0, 4, NECK_REST_Z);
	neck->texOffs(68, 73)->addBox(-5, -11, 10, 10, 10, 18, 0.0f);

	head = (new ModelPart(this))->setTexSize(tw, th);
	head->setPos(0, -8, 10);
	head->texOffs(0, 0)->addBox(-8, -6, -14, 16, 20, 16, 0.0f);
	head->texOffs(0, 0)->addBox(-2, 8, -18, 4, 8, 4, 0.0f);

	mouth = (new ModelPart(this))->setTexSize(tw, th);
	mouth->setPos(0, 13, 0);
	mouth->texOffs(0, 36)->addBox(-8, -1, -14, 16, 3, 16, 0.0f);

	horns = (new ModelPart(this))->setTexSize(tw, th);
	horns->setPos(-5, 1, -9);
	horns->texOffs(74, 55)->addBox(-5, -14, -1, 2, 14, 4, 0.0f);
	horns->texOffs(74, 55)->addBox(13, -14, -1, 2, 14, 4, 0.0f);
	horns->xRot = 1.0472f;

	head->addChild(mouth);
	head->addChild(horns);
	neck->addChild(head);

	// two hind legs, then two front legs
	leg0 = (new ModelPart(this))->setTexSize(tw, th);
	leg0->setPos(-12, -6, 22);
	leg0->texOffs(96, 0)->addBox(0, -7, -5, 8, 37, 8, 0.0f);

	leg1 = (new ModelPart(this))->setTexSize(tw, th);
	leg1->setPos(4, -6, 22);
	leg1->texOffs(96, 0)->addBox(0, -7, -5, 8, 37, 8, 0.0f);

	leg2 = (new ModelPart(this))->setTexSize(tw, th);
	leg2->setPos(-4, -2, -4);
	leg2->texOffs(64, 0)->addBox(-8, -11, -4, 8, 37, 8, 0.0f);

	leg3 = (new ModelPart(this))->setTexSize(tw, th);
	leg3->setPos(-4, -2, -4);
	leg3->texOffs(64, 0)->addBox(8, -11, -4, 8, 37, 8, 0.0f);
}

void RavagerModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	body->render(scale, usecompiled);
	neck->render(scale, usecompiled);	// draws the head, mouth and horns with it
	leg0->render(scale, usecompiled);
	leg1->render(scale, usecompiled);
	leg2->render(scale, usecompiled);
	leg3->render(scale, usecompiled);
}

void RavagerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot / (float) (180 / PI);
	head->xRot = xRot / (float) (180 / PI);

	// right hind and left front swing together, the other pair against them
	float swing = 0.4f * r;
	leg0->xRot = cos(time * 0.6662f) * swing;
	leg1->xRot = cos(time * 0.6662f + PI) * swing;
	leg2->xRot = cos(time * 0.6662f + PI) * swing;
	leg3->xRot = cos(time * 0.6662f) * swing;
}

// the bite: the neck shoots forward and back over 10 ticks while the mouth snaps open and shut
void RavagerModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
	shared_ptr<Ravager> ravager = dynamic_pointer_cast<Ravager>(mob);
	int attackTick = (ravager != nullptr) ? ravager->getAttackTick() : 0;

	if (attackTick > 0)
	{
		float wave = triangleWave(attackTick - a, 10.0f);
		float f1 = (1.0f + wave) * 0.5f;
		float lunge = f1 * f1 * f1 * 12.0f;						// 12 at the start and end of the bite, 0 at the furthest point
		neck->z = NECK_REST_Z - 12.0f + lunge;

		if (attackTick > 5) mouth->xRot = sin((-4.0f + attackTick - a) / 4.0f) * PI * 0.4f;
		else mouth->xRot = 0.15707964f * sin(PI * (attackTick - a) / 10.0f);
	}
	else
	{
		neck->z = NECK_REST_Z;
		mouth->xRot = PI * 0.01f;
	}
}

float RavagerModel::triangleWave(float bob, float period)
{
	return (abs(fmod(bob, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}
