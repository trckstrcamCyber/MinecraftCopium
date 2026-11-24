#include "stdafx.h"
#include "EndRodParticle.h"
#include "../Minecraft.World/Level.h"

EndRodParticle::EndRodParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z)
{
	xd = xa;
	yd = ya;
	zd = za;
	size *= 0.75f;
	lifetime = 60 + random->nextInt(12);
	noPhysics = true;

	fadeR = 0xf2 / 255.0f;
	fadeG = 0xde / 255.0f;
	fadeB = 0xc9 / 255.0f;

	setMiscTex(BASE_TEX + FRAME_COUNT - 1);
}

AABB *EndRodParticle::getCollideBox()
{
	return nullptr;
}

bool EndRodParticle::isPushable()
{
	return false;
}

void EndRodParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (age++ >= lifetime) remove();
	if (age > lifetime / 2)
	{
		setAlpha(1.0f - ((static_cast<float>(age) - lifetime / 2) / static_cast<float>(lifetime)));
		rCol = rCol + (fadeR - rCol) * .2f;
		gCol = gCol + (fadeG - gCol) * .2f;
		bCol = bCol + (fadeB - bCol) * .2f;
	}

	setMiscTex(BASE_TEX + (FRAME_COUNT - 1 - age * FRAME_COUNT / lifetime));

	yd -= 0.0005;
	move(xd, yd, zd, true);
	xd *= 0.91f;
	yd *= 0.91f;
	zd *= 0.91f;
}

int EndRodParticle::getLightColor(float a)
{
	return SharedConstants::FULLBRIGHT_LIGHTVALUE;
}

float EndRodParticle::getBrightness(float a)
{
	return 1.0f;
}
