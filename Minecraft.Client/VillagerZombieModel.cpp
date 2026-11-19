#include "stdafx.h"
#include "ModelPart.h"
#include "VillagerZombieModel.h"
#include "../Minecraft.World/Mth.h"

void VillagerZombieModel::_init(float g, float yOffset, bool isArmor)
{
	delete head;
	if (isArmor)
	{
		head = new ModelPart(this, 0, 0);
		head->addBox(-4, -10, -4, 8, 6, 8, g); // Head
		head->setPos(0, 0 + yOffset, 0);
	}
	else
	{
		head = new ModelPart(this);
		head->setPos(0, 0 + yOffset, 0);
		head->texOffs(0, 0)->addBox(-4, -10, -4, 8, 10, 8, g);
		head->texOffs(24, 0)->addBox(-1, -3, -6, 2, 4, 2, g);

		delete body;
		body = new ModelPart(this, 16, 20);
		body->addBox(-4, 0, -3, 8, 12, 6, g);
		body->setPos(0, 0 + yOffset, 0);

		delete arm0;
		arm0 = new ModelPart(this, 44, 38);
		arm0->addBox(-3, -2, -2, 4, 12, 4, g);
		arm0->setPos(-5, 2 + yOffset, 0);

		delete arm1;
		arm1 = new ModelPart(this, 44, 38);
		arm1->bMirror = true;
		arm1->addBox(-1, -2, -2, 4, 12, 4, g);
		arm1->setPos(5, 2 + yOffset, 0);

		delete leg0;
		leg0 = new ModelPart(this, 0, 22);
		leg0->addBox(-2, 0, -2, 4, 12, 4, g);
		leg0->setPos(-1.9f, 12 + yOffset, 0);

		delete leg1;
		leg1 = new ModelPart(this, 0, 22);
		leg1->bMirror = true;
		leg1->addBox(-2, 0, -2, 4, 12, 4, g);
		leg1->setPos(1.9f, 12 + yOffset, 0);

		delete jacket;
		jacket = new ModelPart(this, 0, 38);
		jacket->addBox(-4, 0, -3, 8, 18, 6, g + 0.5f);
		jacket->setPos(0, 0 + yOffset, 0);

		delete sleeve0;
		sleeve0 = new ModelPart(this, 44, 22);
		sleeve0->addBox(-3, -2, -2, 4, 12, 4, g + 0.25f);
		sleeve0->setPos(-5, 2 + yOffset, 0);

		delete sleeve1;
		sleeve1 = new ModelPart(this, 44, 22);
		sleeve1->addBox(-1, -2, -2, 4, 12, 4, g + 0.25f);
		sleeve1->setPos(5, 2 + yOffset, 0);

		delete pants0;
		pants0 = new ModelPart(this, 0, 22);
		pants0->addBox(-2, 0, -2, 4, 12, 4, g + 0.25f);
		pants0->setPos(-1.9f, 12 + yOffset, 0);

		delete pants1;
		pants1 = new ModelPart(this, 0, 22);
		pants1->bMirror = true;
		pants1->addBox(-2, 0, -2, 4, 12, 4, g + 0.25f);
		pants1->setPos(1.9f, 12 + yOffset, 0);
	}

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
	if (!isArmor)
	{
		body->compile(1.0f/16.0f);
		arm0->compile(1.0f/16.0f);
		arm1->compile(1.0f/16.0f);
		leg0->compile(1.0f/16.0f);
		leg1->compile(1.0f/16.0f);
		jacket->compile(1.0f/16.0f);
		sleeve0->compile(1.0f/16.0f);
		sleeve1->compile(1.0f/16.0f);
		pants0->compile(1.0f/16.0f);
		pants1->compile(1.0f/16.0f);
	}
}

VillagerZombieModel::VillagerZombieModel() : HumanoidModel(0, 0, 64, 64)
{
	_init(0, 0, false);
}

VillagerZombieModel::VillagerZombieModel(float g, float yOffset, bool isArmor) : HumanoidModel(g, 0, 64, isArmor ? 32 : 64)
{
	_init(g, yOffset, isArmor);
}

int VillagerZombieModel::version()
{
	return 11;
}

void VillagerZombieModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	float attack2 = Mth::sin(attackTime * PI);
	float attack = Mth::sin((1 - (1 - attackTime) * (1 - attackTime)) * PI);
	arm0->zRot = 0;
	arm1->zRot = 0;
	arm0->yRot = -(0.1f - attack2 * 0.6f);
	arm1->yRot = +(0.1f - attack2 * 0.6f);
	arm0->xRot = -PI / 2.0f;
	arm1->xRot = -PI / 2.0f;
	arm0->xRot -= attack2 * 1.2f - attack * 0.4f;
	arm1->xRot -= attack2 * 1.2f - attack * 0.4f;

	arm0->zRot += (Mth::cos(bob * 0.09f) * 0.05f + 0.05f);
	arm1->zRot -= (Mth::cos(bob * 0.09f) * 0.05f + 0.05f);
	arm0->xRot += (Mth::sin(bob * 0.067f) * 0.05f);
	arm1->xRot -= (Mth::sin(bob * 0.067f) * 0.05f);
}