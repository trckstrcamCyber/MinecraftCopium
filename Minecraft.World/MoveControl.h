#pragma once

#include "Control.h"

class Mob;

class MoveControl : public Control
{
public:
	static const float MIN_SPEED;
	static const float MIN_SPEED_SQR;

private:
	static const int MAX_TURN = 30;

	Mob *mob;
	double wantedX;
	double wantedY;
	double wantedZ;
	double speedModifier;
	bool _hasWanted;
	bool _strafing;
	float strafeForwards;
	float strafeRight;

public:
	MoveControl(Mob *mob);

	bool hasWanted();
	double getSpeedModifier();
	void setWantedPosition(double x, double y, double z, double speedModifier);
	void setSpeed(float speed);
	void strafe(float forwards, float right);
	virtual void tick();

private:
	float rotlerp(float a, float b, float max);
};