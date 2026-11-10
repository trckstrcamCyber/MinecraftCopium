#include "stdafx.h"
#include "VexModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "../Minecraft.World/Vex.h"

VexModel::VexModel() : HumanoidModel(0.0f, 0.0f, 32, 32)	// 32x32 texture
{
	texWidth = 32;
	texHeight = 32;

	// every part is rebuilt for the vex skin; all coordinates are absolute (the model's root sat 2.5 units above the origin)
	head->cubes.clear();
	head->setTexSize(32, 32);
	head->texOffs(0, 0);
	head->addHumanoidBox(-2.5f, -5.0f, -2.5f, 5, 5, 5, 0.0f);
	head->setPos(0, 17.5f, 0);

	body->cubes.clear();
	body->setTexSize(32, 32);
	body->texOffs(0, 10);
	body->addHumanoidBox(-1.5f, 0.0f, -1.0f, 3, 4, 2, 0.0f);
	body->texOffs(0, 16);
	body->addHumanoidBox(-1.5f, 1.0f, -1.0f, 3, 5, 2, -0.2f);
	body->setPos(0, 17.5f, 0);

	arm0->cubes.clear();
	arm0->setTexSize(32, 32);
	arm0->texOffs(23, 0);
	arm0->addHumanoidBox(-1.25f, -0.5f, -1.0f, 2, 4, 2, -0.1f);
	arm0->setPos(-1.75f, 17.75f, 0);

	arm1->cubes.clear();
	arm1->setTexSize(32, 32);
	arm1->texOffs(23, 6);
	arm1->addHumanoidBox(-0.75f, -0.5f, -1.0f, 2, 4, 2, -0.1f);
	arm1->setPos(1.75f, 17.75f, 0);

	// a vex has no legs and none of the extra skin layers
	ModelPart *unused[] = { leg0, leg1, hair, jacket, sleeve0, sleeve1, pants0, pants1 };
	for (int i = 0; i < 8; i++)
	{
		if (unused[i] != nullptr)
		{
			unused[i]->cubes.clear();
			unused[i]->visible = false;
		}
	}

	// thin sheet wings hang off the back of the body
	rightWing = new ModelPart(this, 16, 14);
	rightWing->setTexSize(32, 32);
	rightWing->bMirror = true;
	rightWing->addBox(0, 0, 0, 0, 5, 8);
	rightWing->bMirror = false;
	rightWing->setPos(-0.5f, 1.0f, 1.0f);
	body->addChild(rightWing);

	leftWing = new ModelPart(this, 16, 14);
	leftWing->setTexSize(32, 32);
	leftWing->addBox(0, 0, 0, 0, 5, 8);
	leftWing->setPos(0.5f, 1.0f, 1.0f);
	body->addChild(leftWing);

	head->compile(1.0f / 16.0f);
	body->compile(1.0f / 16.0f);
	arm0->compile(1.0f / 16.0f);
	arm1->compile(1.0f / 16.0f);
	rightWing->compile(1.0f / 16.0f);
	leftWing->compile(1.0f / 16.0f);
	for (int i = 0; i < 8; i++) if (unused[i] != nullptr) unused[i]->compile(1.0f / 16.0f);
}

void VexModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot * (PI / 180.0f);
	head->xRot = xRot * (PI / 180.0f);

	shared_ptr<Vex> vex = dynamic_pointer_cast<Vex>(entity);
	bool charging = vex != nullptr && vex->isCharging();

	body->xRot = 0.15f;
	float flap = Mth::cos(bob * 5.5f * (PI / 180.0f));
	if (charging)
	{
		arm0->xRot = 3.7699115f;		// sword arm thrown up and forward
		arm1->xRot = 0.27925268f;
	}
	else
	{
		arm0->xRot = 0.15f + flap * 0.1f;
		arm1->xRot = 0.15f + flap * 0.1f;
	}
	arm0->yRot = arm1->yRot = 0;
	arm0->zRot = -0.05f;
	arm1->zRot = 0.05f;

	// wings flap about 7.6 times a second
	float wingBeat = bob * 45.836624f * (PI / 180.0f);
	rightWing->yRot = 1.0995574f + Mth::cos(wingBeat) * PI * 0.05f;
	leftWing->yRot = -rightWing->yRot;
	rightWing->xRot = leftWing->xRot = 0.47123888f;
	rightWing->zRot = -0.47123888f;
	leftWing->zRot = 0.47123888f;
}
