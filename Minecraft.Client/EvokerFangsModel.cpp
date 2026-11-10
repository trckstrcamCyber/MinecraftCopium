#include "stdafx.h"
#include "EvokerFangsModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/Mth.h"

EvokerFangsModel::EvokerFangsModel()
{
	texWidth = 64;
	texHeight = 32;

	base = new ModelPart(this, 0, 0);
	base->setPos(-5.0f, 22.0f, -5.0f);
	base->addBox(0.0f, 0.0f, 0.0f, 10, 12, 10);

	upperJaw = new ModelPart(this, 40, 0);
	upperJaw->setPos(1.5f, 22.0f, -4.0f);
	upperJaw->addBox(0.0f, 0.0f, 0.0f, 4, 14, 8);

	lowerJaw = new ModelPart(this, 40, 0);
	lowerJaw->setPos(-1.5f, 22.0f, 4.0f);
	lowerJaw->yRot = PI;
	lowerJaw->addBox(0.0f, 0.0f, 0.0f, 4, 14, 8);
}

// "time" carries the bite progress (0 = hidden .. 1 = fully bitten, see EvokerFangs::getAnimationProgress)
void EvokerFangsModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	float f = time * 2.0f;
	if (f > 1.0f) f = 1.0f;
	f = 1.0f - f * f * f;

	upperJaw->zRot = PI - f * 0.35f * PI;
	lowerJaw->zRot = PI + f * 0.35f * PI;
	lowerJaw->yRot = PI;

	float rise = (time + Mth::sin(time * 2.7f)) * 0.6f * 12.0f;
	upperJaw->y = 24.0f - rise;
	lowerJaw->y = upperJaw->y;
	base->y = upperJaw->y;
}

void EvokerFangsModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);
	base->render(scale, usecompiled);
	upperJaw->render(scale, usecompiled);
	lowerJaw->render(scale, usecompiled);
}
