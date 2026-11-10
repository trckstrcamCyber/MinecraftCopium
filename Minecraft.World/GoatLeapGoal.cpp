#include "stdafx.h"
#include "Random.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.level.h"
#include "Mth.h"
#include "Goat.h"
#include "GoatLeapGoal.h"

// Upward launch speed: ~0.9 gives an apex of roughly 4.8 blocks (a normal jump, 0.42, is 1.25).
static const float LEAP_SPEED = 0.9f;
static const float LEAP_FORWARD = 0.3f;

GoatLeapGoal::GoatLeapGoal(Goat *goat)
{
	this->goat = goat;
	setRequiredControlFlags(Control::JumpControlFlag);
}

bool GoatLeapGoal::canUse()
{
	if (goat->isBaby() || goat->isRamming()) return false;
	if (!goat->onGround || goat->isInWater() || goat->riding != nullptr) return false;
	// Roughly once every 15-20 seconds on average per goat (the selector asks every few ticks).
	return goat->level->random->nextInt(120) == 0;
}

bool GoatLeapGoal::canContinueToUse()
{
	return false;	// a single launch
}

void GoatLeapGoal::start()
{
	float yRotRad = goat->yRot * PI / 180.0f;
	goat->xd += -Mth::sin(yRotRad) * LEAP_FORWARD;
	goat->zd +=  Mth::cos(yRotRad) * LEAP_FORWARD;
	goat->yd = LEAP_SPEED;
	goat->playSound(eSoundType_MOB_GOAT_JUMP, 1.0f, 1.0f);
}
