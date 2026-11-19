#include "stdafx.h"
#include "IllagerModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "../Minecraft.World/Illager.h"

void IllagerModel::rebuild(ModelPart *part, int u, int v, bool mirror, float x, float y, float z, int w, int h, int d, float g)
{
	part->cubes.clear();
	part->texOffs(u, v);
	part->bMirror = mirror;
	part->addHumanoidBox(x, y, z, w, h, d, g);
	part->bMirror = false;
}

IllagerModel::IllagerModel() : ZombieModel(0.0f, false)	// 64x64 humanoid, not an armor model
{
	showCrossedArms = true;

	rebuild(head, 0, 0, false, -4, -10, -4, 8, 10, 8, 0.0f);
	rebuild(body, 16, 20, false, -4, 0, -3, 8, 12, 6, 0.0f);
	body->texOffs(0, 38);			// the long robe, drawn as part of the body
	body->addHumanoidBox(-4, 0, -3, 8, 18, 6, 0.5f);
	rebuild(arm0, 40, 46, false, -3, -2, -2, 4, 12, 4, 0.0f);
	rebuild(arm1, 40, 46, true, -1, -2, -2, 4, 12, 4, 0.0f);
	rebuild(leg0, 0, 22, false, -2, 0, -2, 4, 12, 4, 0.0f);
	rebuild(leg1, 0, 22, true, -2, 0, -2, 4, 12, 4, 0.0f);

	// no jacket, sleeve or pants layers on an illager skin
	ModelPart *unused[] = { jacket, sleeve0, sleeve1, pants0, pants1 };
	for (int i = 0; i < 5; i++)
	{
		if (unused[i] != nullptr) unused[i]->cubes.clear();
	}

	// the hat layer is only worn by the Illusioner (shown per entity in setupAnim)
	if (hair != nullptr)
	{
		hair->cubes.clear();
		hair->texOffs(32, 0);
		hair->addHumanoidBox(-4, -10, -4, 8, 12, 8, 0.45f);
	}

	// the big nose hangs off the head
	nose = new ModelPart(this, 24, 0);
	nose->addHumanoidBox(-1, -1, -6, 2, 4, 2, 0.0f);
	nose->setPos(0, -2, 0);
	head->addChild(nose);

	// calm pose: both forearms folded in front of the chest
	crossedArms = new ModelPart(this, 44, 22);
	crossedArms->addHumanoidBox(-8, -2, -2, 4, 8, 4, 0.0f);
	crossedArms->bMirror = true;
	crossedArms->addHumanoidBox(4, -2, -2, 4, 8, 4, 0.0f);
	crossedArms->bMirror = false;
	crossedArms->texOffs(40, 38);
	crossedArms->addHumanoidBox(-4, 2, -2, 8, 4, 4, 0.0f);
	crossedArms->setPos(0, 3, -1);
	crossedArms->xRot = -0.75f;

	ModelPart *all[] = { head, body, arm0, arm1, leg0, leg1, nose, crossedArms };
	for (int i = 0; i < 8; i++) all[i]->compile(1.0f / 16.0f);
	if (hair) hair->compile(1.0f / 16.0f);
	if (jacket) jacket->compile(1.0f / 16.0f);
	if (sleeve0) sleeve0->compile(1.0f / 16.0f);
	if (sleeve1) sleeve1->compile(1.0f / 16.0f);
	if (pants0) pants0->compile(1.0f / 16.0f);
	if (pants1) pants1->compile(1.0f / 16.0f);
}

void IllagerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	shared_ptr<Illager> illager = dynamic_pointer_cast<Illager>(entity);
	Illager::ArmPose pose = (illager != nullptr) ? illager->getArmPose() : Illager::CROSSED;
	// a lost raid's survivors cheer whenever they are not busy (arms folded or hanging)
	if (illager != nullptr && illager->isCelebrating() && (pose == Illager::CROSSED || pose == Illager::NEUTRAL)) pose = Illager::CELEBRATING;

	// arms always return to their resting pivots first (the casting pose moves them)
	arm0->setPos(-5, 2, 0);
	arm1->setPos(5, 2, 0);
	arm0->yRot = arm1->yRot = 0;
	arm0->zRot = arm1->zRot = 0;

	if (pose == Illager::ATTACKING)
	{
		// weapon raised, then swung down over the swing time (Java: AnimationUtils.swingWeaponDown)
		float swing = Mth::sin(attackTime * PI);
		float swing2 = Mth::sin((1.0f - (1.0f - attackTime) * (1.0f - attackTime)) * PI);
		arm0->zRot = 0;
		arm1->zRot = 0;
		arm0->yRot = 0.15707964f;
		arm1->yRot = -0.15707964f;
		arm0->xRot = -1.8849558f + Mth::cos(bob * 0.09f) * 0.15f;
		arm1->xRot = Mth::cos(bob * 0.19f) * 0.5f;
		arm0->xRot += swing * 2.2f - swing2 * 0.4f;
		arm1->xRot += swing * 1.2f - swing2 * 0.4f;
		arm0->zRot += Mth::cos(bob * 0.09f) * 0.05f + 0.05f;
		arm1->zRot -= Mth::cos(bob * 0.09f) * 0.05f + 0.05f;
		arm0->xRot += Mth::sin(bob * 0.067f) * 0.05f;
		arm1->xRot -= Mth::sin(bob * 0.067f) * 0.05f;
	}
	else if (pose == Illager::SPELLCASTING)
	{
		// both arms thrown up and out, trembling
		arm0->setPos(-5, 2, 0);
		arm1->setPos(5, 2, 0);
		arm0->xRot = Mth::cos(bob * 0.6662f) * 0.25f;
		arm1->xRot = Mth::cos(bob * 0.6662f) * 0.25f;
		arm0->zRot = 2.3561945f;
		arm1->zRot = -2.3561945f;
		arm0->yRot = 0;
		arm1->yRot = 0;
	}

	if (pose == Illager::CELEBRATING)
	{
		// both arms thrown up in the air, waving a little (Java: AnimationUtils for the CELEBRATING pose)
		arm0->setPos(-5, 2, 0);
		arm1->setPos(5, 2, 0);
		arm0->xRot = Mth::cos(bob * 0.6662f) * 0.05f;
		arm1->xRot = Mth::cos(bob * 0.6662f) * 0.05f;
		arm0->zRot = 2.670354f;
		arm1->zRot = -2.3561945f;
		arm0->yRot = 0;
		arm1->yRot = 0;
	}

	if (pose == Illager::BOW_AND_ARROW)
	{
		// both arms out in front, holding the bow up at the target
		arm0->yRot = -0.1f + head->yRot;
		arm1->yRot = 0.1f + head->yRot + 0.4f;
		arm0->xRot = -PI / 2.0f + head->xRot;
		arm1->xRot = -PI / 2.0f + head->xRot;
	}

	if (pose == Illager::CROSSBOW_HOLD)
	{
		// both hands out in front holding the loaded crossbow up (Java: AnimationUtils.animateCrossbowHold)
		arm0->yRot = -0.3f + head->yRot;
		arm1->yRot = 0.6f + head->yRot;
		arm0->xRot = -PI / 2.0f + head->xRot + 0.1f;
		arm1->xRot = -1.5f + head->xRot;
	}
	else if (pose == Illager::CROSSBOW_CHARGE)
	{
		// drawing the string: the off hand pulls back while the other holds the stock (Java: animateCrossbowCharge). The client does not
		// know how far through the draw it is, so the pull repeats on a 25 tick loop.
		float t = static_cast<float>(fmod(bob, 25.0f)) / 25.0f;
		arm0->yRot = -0.8f;
		arm0->xRot = -0.97079635f;
		arm1->xRot = -0.97079635f + t * (-PI / 2.0f + 0.97079635f);
		arm1->yRot = 0.4f + t * 0.45f;
	}

	if (hair != nullptr) hair->visible = (entity != nullptr && entity->GetType() == eTYPE_ILLUSIONER);

	showCrossedArms = (pose == Illager::CROSSED);
	arm0->visible = !showCrossedArms;
	arm1->visible = !showCrossedArms;
}

void IllagerModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	HumanoidModel::render(entity, time, r, bob, yRot, xRot, scale, usecompiled);	// runs setupAnim and draws head, body, arms and legs
	if (showCrossedArms) crossedArms->render(scale, usecompiled);
}
