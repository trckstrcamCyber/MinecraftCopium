#include "stdafx.h"
#include "EnderCrystalModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystalRenderer.h"
#include "Tesselator.h"
#include "Lighting.h"

ResourceLocation EnderCrystalRenderer::ENDER_CRYSTAL_LOCATION = ResourceLocation(TN_MOB_ENDERDRAGON_ENDERCRYSTAL);
ResourceLocation EnderCrystalRenderer::BEAM_LOCATION = ResourceLocation(TN_MOB_ENDERDRAGON_BEAM);

EnderCrystalRenderer::EnderCrystalRenderer()
{
	currentModel = -1;
	this->shadowRadius = 0.5f;
}

void EnderCrystalRenderer::render(shared_ptr<Entity> _crystal, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type EnderCrystal rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(_crystal);
	if (currentModel != EnderCrystalModel::MODEL_ID)
	{
		model = new EnderCrystalModel(0);
		currentModel = EnderCrystalModel::MODEL_ID;
	}


	float tt = crystal->time + a;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	bindTexture(&ENDER_CRYSTAL_LOCATION);
	float hh = sin(tt * 0.2f) / 2 + 0.5f;
	hh = hh * hh + hh;
	model->render(crystal, 0, tt * 3, hh * 0.2f, 0, 0, 1 / 16.0f, true);

	glPopMatrix();

	if (crystal->hasBeamTarget()) renderBeam(crystal, x, y, z, a);
}

ResourceLocation *EnderCrystalRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &ENDER_CRYSTAL_LOCATION;
}
void EnderCrystalRenderer::renderBeam(shared_ptr<EnderCrystal> crystal, double x, double y, double z, float a)
{
	float tt = crystal->time + a;
	float hh = sin(tt * 0.2f) / 2 + 0.5f;
	hh = (hh * hh + hh) * 0.2f;

	double cx = crystal->xo + (crystal->x - crystal->xo) * a;
	double cy = crystal->yo + (crystal->y - crystal->yo) * a;
	double cz = crystal->zo + (crystal->z - crystal->zo) * a;
	float xd = static_cast<float>(crystal->getBeamTargetX() + 0.5 - cx);
	float yd = static_cast<float>(crystal->getBeamTargetY() + 0.5 - (cy + hh + 1));
	float zd = static_cast<float>(crystal->getBeamTargetZ() + 0.5 - cz);

	float sdd = sqrt(xd * xd + zd * zd);
	float dd = sqrt(xd * xd + yd * yd + zd * zd);

	glColor4f(1, 1, 1, 1);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + hh + 1, static_cast<float>(z));
	glRotatef(static_cast<float>(-atan2(zd, xd)) * 180.0f / PI - 90.0f, 0, 1, 0);
	glRotatef(static_cast<float>(-atan2(sdd, yd)) * 180.0f / PI - 90.0f, 1, 0, 0);

	Tesselator *t = Tesselator::getInstance();
	Lighting::turnOff();
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	bindTexture(&BEAM_LOCATION);
	glShadeModel(GL_SMOOTH);

	float v0 = 0 - (crystal->tickCount + a) * 0.005f;
	float v1 = dd / 32.0f - (crystal->tickCount + a) * 0.005f;

	t->begin(GL_TRIANGLE_STRIP);
	int steps = 8;
	for (int i = 0; i <= steps; i++)
	{
		float s = sin(i % steps * PI * 2 / steps) * 0.75f;
		float c = cos(i % steps * PI * 2 / steps) * 0.75f;
		float u = i % steps * 1.0f / steps;
		t->vertexUV(s * 0.2f, c * 0.2f, 0, u, v1);
		t->vertexUV(s, c, dd, u, v0);
	}
	t->end();

	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);
	glDisable(GL_BLEND);
	glPopMatrix();
	Lighting::turnOn();
}
