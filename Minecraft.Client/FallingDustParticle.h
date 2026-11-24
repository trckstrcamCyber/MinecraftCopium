#pragma once
#include "Particle.h"

class FallingDustParticle : public Particle
{
public:
	virtual eINSTANCEOF GetType() { return eType_FALLINGDUSTPARTICLE; }

private:
	float baseSize;

public:
	FallingDustParticle(Level *level, double x, double y, double z, float r, float g, float b);
	virtual void tick();
	virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
};
