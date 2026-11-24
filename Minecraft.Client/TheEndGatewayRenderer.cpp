#include "stdafx.h"
#include "../Minecraft.World/Random.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/FloatBuffer.h"
#include "../Minecraft.World/Facing.h"
#include "Tesselator.h"
#include "TileEntityRenderDispatcher.h"
#include "TheEndGatewayRenderer.h"

ResourceLocation TheEndGatewayRenderer::END_SKY_LOCATION = ResourceLocation(TN_MISC_TUNNEL);
ResourceLocation TheEndGatewayRenderer::END_PORTAL_LOCATION = ResourceLocation(TN_MISC_PARTICLEFIELD);
ResourceLocation TheEndGatewayRenderer::BEAM_LOCATION = ResourceLocation(TN_MISC_BEACON_BEAM);
int TheEndGatewayRenderer::RANDOM_SEED = 31100;
Random TheEndGatewayRenderer::RANDOM = Random(RANDOM_SEED);

TheEndGatewayRenderer::TheEndGatewayRenderer()
{
	lb = MemoryTracker::createFloatBuffer(16);
	modelView = MemoryTracker::createFloatBuffer(16);
	projection = MemoryTracker::createFloatBuffer(16);
}

FloatBuffer *TheEndGatewayRenderer::getBuffer(float a, float b, float c, float d)
{
	lb->clear();
	lb->put(a)->put(b)->put(c)->put(d);
	lb->flip();
	return lb;
}

int TheEndGatewayRenderer::getPasses(double distanceSqr)
{
	int passes;
	if (distanceSqr > 36864.0) passes = 1;
	else if (distanceSqr > 25600.0) passes = 3;
	else if (distanceSqr > 16384.0) passes = 5;
	else if (distanceSqr > 9216.0) passes = 7;
	else if (distanceSqr > 4096.0) passes = 9;
	else if (distanceSqr > 1024.0) passes = 11;
	else if (distanceSqr > 576.0) passes = 13;
	else if (distanceSqr > 256.0) passes = 14;
	else passes = 15;
	return passes + 1;
}

void TheEndGatewayRenderer::renderBeam(double x, double y, double z, float a, double height, float r, float g, float b)
{
	if (height == 0) return;

	Level *level = tileEntityRenderDispatcher->level;
	float tt = (level != nullptr ? static_cast<float>(level->getGameTime()) : 0.0f) + a;
	float texVOff = -tt * 0.2f - floor(-tt * 0.1f);

	bindTexture(&BEAM_LOCATION);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(false);

	double y0 = y + 0.5;
	double y1 = y + 0.5 + height;
	double v0 = texVOff;
	double v1 = fabs(height) * (0.5 / 0.15) + texVOff;
	double rot = tt * 0.025 * -1.5;

	double ax = 0.5 + cos(rot + PI * 0.75) * 0.15, az = 0.5 + sin(rot + PI * 0.75) * 0.15;
	double bx = 0.5 + cos(rot + PI * 0.25) * 0.15, bz = 0.5 + sin(rot + PI * 0.25) * 0.15;
	double cx = 0.5 + cos(rot + PI * 1.25) * 0.15, cz = 0.5 + sin(rot + PI * 1.25) * 0.15;
	double dx = 0.5 + cos(rot + PI * 1.75) * 0.15, dz = 0.5 + sin(rot + PI * 1.75) * 0.15;

	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->color(r, g, b, 1.0f);
	t->vertexUV(x + ax, y1, z + az, 1.0, v1); t->vertexUV(x + ax, y0, z + az, 1.0, v0); t->vertexUV(x + bx, y0, z + bz, 0.0, v0); t->vertexUV(x + bx, y1, z + bz, 0.0, v1);
	t->vertexUV(x + dx, y1, z + dz, 1.0, v1); t->vertexUV(x + dx, y0, z + dz, 1.0, v0); t->vertexUV(x + cx, y0, z + cz, 0.0, v0); t->vertexUV(x + cx, y1, z + cz, 0.0, v1);
	t->vertexUV(x + bx, y1, z + bz, 1.0, v1); t->vertexUV(x + bx, y0, z + bz, 1.0, v0); t->vertexUV(x + dx, y0, z + dz, 0.0, v0); t->vertexUV(x + dx, y1, z + dz, 0.0, v1);
	t->vertexUV(x + cx, y1, z + cz, 1.0, v1); t->vertexUV(x + cx, y0, z + cz, 1.0, v0); t->vertexUV(x + ax, y0, z + az, 0.0, v0); t->vertexUV(x + ax, y1, z + az, 0.0, v1);
	t->end();

	glDepthMask(true);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

void TheEndGatewayRenderer::render(shared_ptr<TileEntity> _gateway, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<TheEndGatewayTileEntity> gateway = dynamic_pointer_cast<TheEndGatewayTileEntity>(_gateway);
	if (gateway == nullptr) return;

	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);

	if (gateway->isSpawning() || gateway->isCoolingDown())
	{
		float percent = gateway->isSpawning() ? gateway->getSpawnPercent(a) : gateway->getCooldownPercent(a);
		double beamLength = gateway->isSpawning() ? 256.0 - y : 25.0;
		percent = Mth::sin(percent * PI);
		double beamHeight = floor(percent * beamLength);
		float r = gateway->isSpawning() ? 0.70f : 0.48f;
		float g = gateway->isSpawning() ? 0.33f : 0.20f;
		float b = gateway->isSpawning() ? 0.82f : 0.66f;
		renderBeam(x, y, z, a, beamHeight, r, g, b);
		renderBeam(x, y, z, a, -beamHeight, r, g, b);
	}

	RANDOM.setSeed(RANDOM_SEED);

	glGetFloat(GL_MODELVIEW_MATRIX, modelView);
	glGetFloat(GL_PROJECTION_MATRIX, projection);

	int passes = getPasses(x * x + y * y + z * z);
	for (int i = 0; i < passes; i++)
	{
		float brightness = 2.0f / static_cast<float>(18 - i);
		if (i == 0)
		{
			bindTexture(&END_SKY_LOCATION);
			brightness = 0.15f;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			bindTexture(&END_PORTAL_LOCATION);
			if (i == 1)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
			}
		}

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGen(GL_S, GL_EYE_PLANE, getBuffer(1, 0, 0, 0));
		glTexGen(GL_T, GL_EYE_PLANE, getBuffer(0, 1, 0, 0));
		glTexGen(GL_R, GL_EYE_PLANE, getBuffer(0, 0, 1, 0));
		glTexGen(GL_Q, GL_OBJECT_PLANE, getBuffer(0, 0, 0, 1));
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.5f, 0.5f, 0.0f);
		glScalef(0.5f, 0.5f, 1.0f);
		float layer = static_cast<float>(i + 1);
		float timeFraction = static_cast<float>(System::currentTimeMillis() % 800000) / 800000.0f;
		glTranslatef(17.0f / layer, (2.0f + layer / 1.5f) * timeFraction, 0.0f);
		glRotatef((layer * layer * 4321.0f + layer * 9.0f) * 2.0f, 0.0f, 0.0f, 1.0f);
		glScalef(4.5f - layer / 4.0f, 4.5f - layer / 4.0f, 1.0f);
		glMultMatrixf(projection->_getDataPointer());
		glMultMatrixf(modelView->_getDataPointer());

		Tesselator *t = Tesselator::getInstance();
		t->useProjectedTexture(true);
		t->begin();

		float r = RANDOM.nextFloat() * 0.5f + 0.1f;
		float g = RANDOM.nextFloat() * 0.5f + 0.4f;
		float b = RANDOM.nextFloat() * 0.5f + 0.5f;
		if (i == 0) r = g = b = 1.0f;
		t->color(r * brightness, g * brightness, b * brightness, 1.0f);

		if (gateway->shouldRenderFace(Facing::SOUTH))
		{
			t->vertex(x, y, z + 1);
			t->vertex(x + 1, y, z + 1);
			t->vertex(x + 1, y + 1, z + 1);
			t->vertex(x, y + 1, z + 1);
		}
		if (gateway->shouldRenderFace(Facing::NORTH))
		{
			t->vertex(x, y + 1, z);
			t->vertex(x + 1, y + 1, z);
			t->vertex(x + 1, y, z);
			t->vertex(x, y, z);
		}
		if (gateway->shouldRenderFace(Facing::EAST))
		{
			t->vertex(x + 1, y + 1, z);
			t->vertex(x + 1, y + 1, z + 1);
			t->vertex(x + 1, y, z + 1);
			t->vertex(x + 1, y, z);
		}
		if (gateway->shouldRenderFace(Facing::WEST))
		{
			t->vertex(x, y, z);
			t->vertex(x, y, z + 1);
			t->vertex(x, y + 1, z + 1);
			t->vertex(x, y + 1, z);
		}
		if (gateway->shouldRenderFace(Facing::DOWN))
		{
			t->vertex(x, y, z);
			t->vertex(x + 1, y, z);
			t->vertex(x + 1, y, z + 1);
			t->vertex(x, y, z + 1);
		}
		if (gateway->shouldRenderFace(Facing::UP))
		{
			t->vertex(x, y + 1, z + 1);
			t->vertex(x + 1, y + 1, z + 1);
			t->vertex(x + 1, y + 1, z);
			t->vertex(x, y + 1, z);
		}

		t->end();
		t->useProjectedTexture(false);

		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	glEnable(GL_LIGHTING);
	glEnable(GL_FOG);
}
