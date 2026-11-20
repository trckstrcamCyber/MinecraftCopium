#include "stdafx.h"
#include "../Minecraft.World/CopperGolem.h"
#include "../Minecraft.World/CopperGolemStatueBlock.h"
#include "ModelPart.h"
#include "CopperGolemModel.h"

CopperGolemModel::CopperGolemModel(float g)
{
	statuePose = -1;

	int xTexSize = 64;
	int yTexSize = 64;

	head = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	head->setPos(0, 13, 0);
	head->texOffs(0, 0)->addBox(-4, -5, -5, 8, 5, 10, g);
	head->texOffs(56, 0)->addBox(-1, -2, -6, 2, 3, 2, g);
	head->texOffs(37, 8)->addBox(-1, -9, -1, 2, 4, 2, g);
	head->texOffs(37, 0)->addBox(-2, -13, -2, 4, 4, 4, g);

	body = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	body->setPos(0, 19, 0);
	body->texOffs(0, 15)->addBox(-4, -6, -3, 8, 6, 6, g);

	arm0 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	arm0->setPos(-4, 13, 0);
	arm0->texOffs(36, 16)->addBox(-3, -1, -2, 3, 10, 4, g);

	arm1 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	arm1->setPos(4, 13, 0);
	arm1->texOffs(50, 16)->addBox(0, -1, -2, 3, 10, 4, g);

	leg0 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	leg0->setPos(0, 19, 0);
	leg0->texOffs(0, 27)->addBox(-4, 0, -2, 4, 5, 4, g);

	leg1 = (new ModelPart(this))->setTexSize(xTexSize, yTexSize);
	leg1->setPos(0, 19, 0);
	leg1->texOffs(16, 27)->addBox(0, 0, -2, 4, 5, 4, g);
}

void CopperGolemModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	head->render(scale, usecompiled);
	body->render(scale, usecompiled);
	leg0->render(scale, usecompiled);
	leg1->render(scale, usecompiled);
	arm0->render(scale, usecompiled);
	arm1->render(scale, usecompiled);
}

void CopperGolemModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	// Statue poses override everything else (procedural approximations —
	// the Java statue model is a separate class; this reuses the golem
	// model with posed limbs).
	if (statuePose >= 0)
	{
		head->yRot = 0;
		head->xRot = 0;
		body->xRot = 0;
		body->zRot = 0;
		switch (statuePose)
		{
		case CopperGolemStatueBlock::POSE_SITTING:
			// Legs forward horizontal, arms resting forward.
			leg0->xRot = -1.5708f;
			leg1->xRot = -1.5708f;
			leg0->zRot = 0;
			leg1->zRot = 0;
			arm0->xRot = -0.5f;
			arm1->xRot = -0.5f;
			arm0->zRot = 0;
			arm1->zRot = 0;
			break;
		case CopperGolemStatueBlock::POSE_RUNNING:
			// Body leaning forward, limbs swung opposite.
			body->xRot = 0.35f;
			leg0->xRot = -0.8f;
			leg1->xRot = 0.8f;
			leg0->zRot = 0;
			leg1->zRot = 0;
			arm0->xRot = 0.8f;
			arm1->xRot = -0.8f;
			arm0->zRot = 0;
			arm1->zRot = 0;
			break;
		case CopperGolemStatueBlock::POSE_STAR:
			// Arms raised out to the sides, legs spread.
			arm0->zRot = 1.9f;
			arm1->zRot = -1.9f;
			arm0->xRot = 0;
			arm1->xRot = 0;
			leg0->zRot = 0.35f;
			leg1->zRot = -0.35f;
			leg0->xRot = 0;
			leg1->xRot = 0;
			break;
		default:
			// POSE_STANDING: neutral.
			leg0->xRot = 0;
			leg1->xRot = 0;
			leg0->zRot = 0;
			leg1->zRot = 0;
			arm0->xRot = 0;
			arm1->xRot = 0;
			arm0->zRot = 0;
			arm1->zRot = 0;
			break;
		}
		return;
	}

	head->yRot = yRot / (float) (180 / PI);
	head->xRot = xRot / (float) (180 / PI);

	// Head-spin idle animation (Java CopperGolemAnimation, 3.5 s): the
	// golem spins its head only while idle (r is the walk speed; a
	// walking golem keeps its head still). Phase comes from tickCount via
	// CopperGolem::getSpinPhase, so client and server agree.
	shared_ptr<CopperGolem> golem = dynamic_pointer_cast<CopperGolem>(entity);
	if (golem != nullptr && r <= 0.05f)
	{
		float phase = golem->getSpinPhase();
		if (phase >= 0.0f)
		{
			head->yRot = spinHeadAngle(phase) / (float) (180 / PI);
			head->xRot = 0;
		}
	}

	leg0->xRot = (-1.5f * triangleWave(time, 13)) * r;
	leg1->xRot = (1.5f * triangleWave(time, 13)) * r;
	leg0->yRot = 0;
	leg1->yRot = 0;
	leg0->zRot = 0;
	leg1->zRot = 0;
	arm0->zRot = 0;
	arm1->zRot = 0;
}

void CopperGolemModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
	// Arms hang and swing with the walk (VillagerGolem pattern, without
	// the attack / offer-flower branches — the copper golem never attacks).
	arm0->xRot = (-0.2f + 1.5f * triangleWave(time, 13)) * r;
	arm1->xRot = (-0.2f - 1.5f * triangleWave(time, 13)) * r;
}

void CopperGolemModel::setStatuePose(int pose)
{
	statuePose = pose;
}

int CopperGolemModel::getStatuePose()
{
	return statuePose;
}

float CopperGolemModel::triangleWave(float bob, float period)
{
	return (abs(fmod(bob, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}

float CopperGolemModel::spinHeadAngle(float phase)
{
	// Java keyframes (degrees): 0 -> -35 -> +35 -> 300 -> 360, piecewise
	// linear over the spin. This is a deterministic approximation of the
	// Java animation curve (fixed 220-tick period; see CopperGolem.h).
	if (phase < 0.25f) return -35.0f * (phase / 0.25f);
	if (phase < 0.5f) return -35.0f + 70.0f * ((phase - 0.25f) / 0.25f);
	if (phase < 0.75f) return 35.0f + 265.0f * ((phase - 0.5f) / 0.25f);
	return 300.0f + 60.0f * ((phase - 0.75f) / 0.25f);
}
