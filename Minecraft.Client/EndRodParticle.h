#pragma once
#include "Particle.h"

class EndRodParticle : public Particle
{
public:
	virtual eINSTANCEOF GetType() { return eType_ENDRODPARTICLE; }

private:
	static const int BASE_TEX = 11 * 16;
	static const int FRAME_COUNT = 8;

	float fadeR;
	float fadeG;
	float fadeB;

public:
	EndRodParticle(Level *level, double x, double y, double z, double xa, double ya, double za);
	virtual AABB *getCollideBox();
	virtual bool isPushable();
	virtual void tick();
	virtual int getLightColor(float a);
	virtual float getBrightness(float a);
};
