#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "MoveControl.h"

const float MoveControl::MIN_SPEED = 0.0005f;
const float MoveControl::MIN_SPEED_SQR = MIN_SPEED * MIN_SPEED;

MoveControl::MoveControl(Mob *mob)
{
	this->mob = mob;
	wantedX = mob->x;
	wantedY = mob->y;
	wantedZ = mob->z;

	speedModifier = 0.0;

	_hasWanted = false;
	_strafing = false;
	strafeForwards = 0;
	strafeRight = 0;
}

bool MoveControl::hasWanted()
{
	return _hasWanted;
}

double MoveControl::getSpeedModifier()
{
	return speedModifier;
}

void MoveControl::setWantedPosition(double x, double y, double z, double speedModifier)
{
	wantedX = x;
	wantedY = y;
	wantedZ = z;
	this->speedModifier = speedModifier;
	_hasWanted = true;
}

void MoveControl::strafe(float forwards, float right)
{
	strafeForwards = forwards;
	strafeRight = right;
	speedModifier = 0.25;
	_strafing = true;
}

void MoveControl::tick()
{
	mob->setYya(0);
	mob->xxa = 0;

	if (_strafing)
	{
		_strafing = false;
		_hasWanted = false;
		float speed = static_cast<float>(speedModifier * mob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
		float forwards = strafeForwards;
		float right = strafeRight;
		float length = Mth::sqrt(right * right + forwards * forwards);
		if (length < 1.0f) length = 1.0f;
		length = speed / length;
		right *= length;
		forwards *= length;
		float sinYaw = Mth::sin(mob->yRot * PI / 180);
		float cosYaw = Mth::cos(mob->yRot * PI / 180);
		int tx = Mth::floor(mob->x + right * cosYaw - forwards * sinYaw + mob->xd);
		int ty = Mth::floor(mob->y);
		int tz = Mth::floor(mob->z + forwards * cosYaw + right * sinYaw + mob->zd);
		Level *level = mob->level;
		bool blocked = level->getMaterial(tx, ty, tz)->blocksMotion() || !level->getMaterial(tx, ty - 1, tz)->blocksMotion();
		PathNavigation *navigation = mob->getNavigation();
		bool sunlit = navigation != nullptr && navigation->getAvoidSun() && level->isDay() && level->canSeeSky(tx, ty, tz);
		if (blocked || sunlit)
		{
			mob->LivingEntity::setSpeed(0);
			if (!mob->hasImpulse)
			{
				mob->xd = 0;
				mob->zd = 0;
			}
			return;
		}
		mob->LivingEntity::setSpeed(speed);
		mob->setYya(strafeForwards);
		mob->xxa = strafeRight;
		return;
	}

	if (!_hasWanted) return;
	_hasWanted = false;

	int yFloor = floor(mob->bb->y0 + .5f);

	double xd = wantedX - mob->x;
	double zd = wantedZ - mob->z;
	double yd = wantedY - yFloor;
	double dd = xd * xd + yd * yd + zd * zd;
	if (dd < MIN_SPEED_SQR) return;

	float yRotD = static_cast<float>(atan2(zd, xd) * 180 / PI) - 90;

	mob->yRot = rotlerp(mob->yRot, yRotD, MAX_TURN);
	mob->setSpeed(static_cast<float>(speedModifier * mob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));

	if (yd > 0 && xd * xd + zd * zd < 1) mob->getJumpControl()->jump();
}

float MoveControl::rotlerp(float a, float b, float max)
{
	float diff = Mth::wrapDegrees(b - a);
	if (diff > max)
	{
		diff = max;
	}
	if (diff < -max)
	{
		diff = -max;
	}
	return a + diff;
}