#include "stdafx.h"
#include "IceCubeRenderer.h"
#include "IceCubeModel.h"

ResourceLocation IceCubeRenderer::ICE_CUBE_LOCATION = ResourceLocation(TN_MOB_ICE_CUBE);

IceCubeRenderer::IceCubeRenderer()
{
	model = new IceCubeModel();
	shadowRadius = 0.5f;
}

void IceCubeRenderer::render(shared_ptr<Entity> cube, double x, double y, double z, float rot, float a)
{
	glPushMatrix();
	bindTexture(cube);
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(180.0f, 0, 1, 0);
	glScalef(-1, -1, 1);
	glTranslatef(0.0f, -1.5f, 0.0f);
	model->render(cube, a, 0.0f, 0.0f, cube->yRot, cube->xRot, 1.0f / 16.0f, true);
	glPopMatrix();
}

ResourceLocation *IceCubeRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &ICE_CUBE_LOCATION;
}
