#include "stdafx.h"
#include "../Minecraft.World/CopperGolem.h"
#include "../Minecraft.World/CopperGolemStatueBlock.h"
#include "ModelPart.h"
#include "CopperGolemModel.h"

CopperGolemModel::CopperGolemModel(float g)
{
	statuePose = -1;
	lastState = -1;
	stateStartTick = 0;
	wasCarrying = false;
	carryStartTick = 0;

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
	// The head-spin is an idle animation: while the golem is interacting
	// with a chest it plays the reach-in pose instead (below), so the
	// spin must not fight the head-down.
	if (golem != nullptr && r <= 0.05f && golem->getState() == COPPER_GOLEM_STATE_IDLE)
	{
		float phase = golem->getSpinPhase();
		if (phase >= 0.0f)
		{
			head->yRot = spinHeadAngle(phase) / (float) (180 / PI);
			head->xRot = 0;
		}
	}

	// Transport reach-in pose (Phase 5b, Java CopperGolemAnimation
	// CHEST_INTERACTION_*): while the golem is taking from / depositing
	// into a chest, both arms reach forward into it — arm pitches around
	// -90° to -120° in the Java keyframes — body slightly forward, head
	// down. Strongest in the first ~10 ticks (take/deposit at tick 1,
	// sound at tick 9), easing back to the normal pose by tick 60, when
	// the goal closes the chest and resets the state to IDLE.
	// While carrying a stack (Java WALK_ITEM), the arms are held slightly
	// forward/down instead of the full walk swing.
	// The state is the synched byte (CopperGolem::DATA_STATE_ID): the
	// goal sets it server-side and the client reads it here. Runs after
	// prepareMobModel, so these overrides win over the walk swing.
	if (golem != nullptr)
	{
		int state = static_cast<int>(golem->getState());
		if (state != lastState)
		{
			lastState = state;
			stateStartTick = golem->tickCount;
		}
		int elapsed = golem->tickCount - stateStartTick;
		if (elapsed < 0) elapsed = 0;

		bool interacting =
			state == COPPER_GOLEM_STATE_GETTING_ITEM ||
			state == COPPER_GOLEM_STATE_GETTING_NO_ITEM ||
			state == COPPER_GOLEM_STATE_DROPPING_ITEM ||
			state == COPPER_GOLEM_STATE_DROPPING_NO_ITEM;

		if (interacting)
		{
			// Reach strength: ramps up over the first 10 ticks, holds,
			// then eases back to 0 at tick 60.
			float reach = 1.0f;
			if (elapsed < 10) reach = static_cast<float>(elapsed) / 10.0f;
			else if (elapsed < 60) reach = 1.0f - static_cast<float>(elapsed - 10) / 50.0f;
			else reach = 0.0f;
			if (reach < 0.0f) reach = 0.0f;
			if (reach > 1.0f) reach = 1.0f;

			// DROP variants reach marginally lower than GET (Java
			// DROP/NODROP keyframes sit at ≈ -113°/-117° vs ≈ -93°/-98°
			// for GET).
			bool dropping = state == COPPER_GOLEM_STATE_DROPPING_ITEM ||
				state == COPPER_GOLEM_STATE_DROPPING_NO_ITEM;
			float armPitch = dropping ? -1.98f : -1.66f; // -113° / -95°
			float bodyPitch = 0.25f;
			float headPitch = 0.35f;

			arm0->xRot = arm0->xRot * (1.0f - reach) + armPitch * reach;
			arm1->xRot = arm1->xRot * (1.0f - reach) + armPitch * reach;
			body->xRot = body->xRot * (1.0f - reach) + bodyPitch * reach;
			head->xRot = head->xRot * (1.0f - reach) + headPitch * reach;
			arm0->zRot = 0;
			arm1->zRot = 0;
		}
		else if (golem->getCarriedItem() != nullptr)
		{
			// WALK_ITEM: carrying a stack — hold both arms slightly
			// forward/down instead of the full walk swing. Ramps in over
			// the first 10 ticks of carrying so the arms do not snap
			// from the reach pose to the carry pose.
			if (!wasCarrying)
			{
				wasCarrying = true;
				carryStartTick = golem->tickCount;
			}
			int carryElapsed = golem->tickCount - carryStartTick;
			if (carryElapsed < 0) carryElapsed = 0;
			float carry = static_cast<float>(carryElapsed) / 10.0f;
			if (carry > 1.0f) carry = 1.0f;
			float armPitch = -0.6f; // ≈ -34°, arms forward/down
			arm0->xRot = arm0->xRot * (1.0f - carry) + armPitch * carry;
			arm1->xRot = arm1->xRot * (1.0f - carry) + armPitch * carry;
		}
		else
		{
			wasCarrying = false;
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
