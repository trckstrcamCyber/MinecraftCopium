#include "stdafx.h"
#include "FallingDustParticle.h"
#include "../Minecraft.World/Level.h"
#include "../Minecraft.World/Mth.h"

FallingDustParticle::FallingDustParticle(Level *level, double x, double y, double z, float r, float g, float b) : Particle(level, x, y, z)
{
	xd = 0;
	yd = 0;
	zd = 0;
	rCol = r;
	gCol = g;
	bCol = b;
	size *= 0.75f;
	size *= 0.9f;
	baseSize = size;
	lifetime = static_cast<int>(floor(32.0f / (random->nextFloat() * 0.8f + 0.2f) * 0.9f));
	setMiscTex(7);
}

void FallingDustParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	size = baseSize * Mth::clamp((age + a) / lifetime * 32.0f, 0.0f, 1.0f);
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void FallingDustParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime) remove();

	setMiscTex(7 - age * 8 / lifetime);
	move(xd, yd, zd);
	yd -= 0.003;
	if (yd < -0.14) yd = -0.14;
}
