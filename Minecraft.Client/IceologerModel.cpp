#include "stdafx.h"
#include "IceologerModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "../Minecraft.World/Iceologer.h"

IceologerModel::IceologerModel()
{
	texWidth = 128;
	texHeight = 64;

	head = new ModelPart(this);
	head->setPos(0.0f, 0.0f, 0.0f);
	head->texOffs(0, 0);
	head->addBox(-4.0f, -10.0f, -4.0f, 8, 10, 8, 0.0f);
	head->texOffs(70, 33);
	head->addBox(-4.5f, -10.5f, -4.5f, 9, 11, 9, 0.0f);
	head->texOffs(24, 0);
	head->addBox(-1.0f, -3.0f, -6.0f, 2, 4, 2, 0.0f);

	body = new ModelPart(this);
	body->setPos(0.0f, 0.0f, 0.0f);
	body->texOffs(16, 20);
	body->addBox(-4.0f, 0.0f, -3.0f, 8, 12, 6, 0.0f);
	body->texOffs(0, 38);
	body->addBox(-4.0f, 0.0f, -3.0f, 8, 18, 6, 0.25f);

	cape = new ModelPart(this);
	cape->setPos(0.0f, -1.0f, 3.0f);
	cape->texOffs(99, 16);
	cape->addBox(-4.5f, 1.0f, -1.0f, 9, 20, 1, 0.0f);
	body->addChild(cape);

	leg1 = new ModelPart(this);
	leg1->setPos(2.0f, 12.0f, 0.0f);
	leg1->texOffs(0, 22);
	leg1->bMirror = true;
	leg1->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f);
	leg1->bMirror = false;

	leg0 = new ModelPart(this);
	leg0->setPos(-2.0f, 12.0f, 0.0f);
	leg0->texOffs(0, 22);
	leg0->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, 0.0f);

	arm0 = new ModelPart(this);
	arm0->setPos(-5.0f, 2.0f, 0.0f);
	arm0->texOffs(40, 46);
	arm0->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, 0.0f);

	arm1 = new ModelPart(this);
	arm1->setPos(5.0f, 2.0f, 0.0f);
	arm1->texOffs(40, 46);
	arm1->bMirror = true;
	arm1->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, 0.0f);
	arm1->bMirror = false;

	head->compile(1.0f / 16.0f);
	body->compile(1.0f / 16.0f);
	cape->compile(1.0f / 16.0f);
	leg0->compile(1.0f / 16.0f);
	leg1->compile(1.0f / 16.0f);
	arm0->compile(1.0f / 16.0f);
	arm1->compile(1.0f / 16.0f);
}

void IceologerModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	head->render(scale, usecompiled);
	body->render(scale, usecompiled);		// the cape is a child of the body
	leg0->render(scale, usecompiled);
	leg1->render(scale, usecompiled);
	arm0->render(scale, usecompiled);
	arm1->render(scale, usecompiled);
}

void IceologerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	head->yRot = yRot * (PI / 180.0f);
	head->xRot = xRot * (PI / 180.0f);
	leg1->xRot = Mth::cos(time * 0.6662f) * 1.4f * r * 0.5f;
	leg0->xRot = Mth::cos(time * 0.6662f + PI) * 1.4f * r * 0.5f;
	leg0->yRot = 0.0f;
	leg1->yRot = 0.0f;

	shared_ptr<Illager> illager = dynamic_pointer_cast<Illager>(entity);
	Illager::ArmPose pose = (illager != nullptr) ? illager->getArmPose() : Illager::CROSSED;

	arm0->setPos(-5.0f, 2.0f, 0.0f);
	arm1->setPos(5.0f, 2.0f, 0.0f);
	arm0->yRot = arm1->yRot = 0.0f;
	if (pose == Illager::SPELLCASTING)
	{
		// both arms thrown up and out, trembling
		arm0->xRot = Mth::cos(bob * 0.6662f) * 0.25f;
		arm1->xRot = Mth::cos(bob * 0.6662f) * 0.25f;
		arm0->zRot = 2.3561945f;
		arm1->zRot = -2.3561945f;
	}
	else
	{
		// arms swing with the walk
		arm1->xRot = Mth::cos(time * 0.6662f) * 1.4f * r * 0.5f;
		arm0->xRot = Mth::cos(time * 0.6662f + PI) * 1.4f * r * 0.5f;
		arm0->zRot = 0.0f;
		arm1->zRot = 0.0f;
	}
}

// the cape trails the body: its tilt follows how far the "chasing" point lags the Iceologer (formula from the original mod)
void IceologerModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a)
{
	shared_ptr<Iceologer> ice = dynamic_pointer_cast<Iceologer>(mob);
	if (ice == nullptr) return;

	double dx = ice->prevChasingPosX + (ice->chasingPosX - ice->prevChasingPosX) * a - (ice->xo + (ice->x - ice->xo) * a);
	double dy = ice->prevChasingPosY + (ice->chasingPosY - ice->prevChasingPosY) * a - (ice->yo + (ice->y - ice->yo) * a);
	double dz = ice->prevChasingPosZ + (ice->chasingPosZ - ice->prevChasingPosZ) * a - (ice->zo + (ice->z - ice->zo) * a);

	float bodyYaw = ice->yBodyRotO + (ice->yBodyRot - ice->yBodyRotO) * a;
	double sinYaw = Mth::sin(bodyYaw * PI / 180.0f);
	double cosYaw = -Mth::cos(bodyYaw * PI / 180.0f);

	float lift = static_cast<float>(dy) * 10.0f;
	if (lift < -6.0f) lift = -6.0f;
	if (lift > 32.0f) lift = 32.0f;

	float back = static_cast<float>(dx * sinYaw + dz * cosYaw) * 100.0f;
	if (back < 0.0f) back = 0.0f;
	if (back > 150.0f) back = 150.0f;

	float side = static_cast<float>(dx * cosYaw - dz * sinYaw) * 100.0f;
	if (side < -20.0f) side = -20.0f;
	if (side > 20.0f) side = 20.0f;

	// a little extra flutter while walking
	float walkBob = (ice->walkDistO + (ice->walkDist - ice->walkDistO) * a);
	lift += Mth::sin(walkBob * 6.0f) * 32.0f * (r * 0.1f);

	cape->xRot = (180.0f - (6.0f + back / 2.0f + lift)) * (PI / 180.0f);
	cape->yRot = (side / 2.0f) * (PI / 180.0f);
	cape->zRot = (180.0f - side / 2.0f) * (PI / 180.0f);
}
