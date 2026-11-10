#include "stdafx.h"
#include "EvokerFangsRenderer.h"
#include "EvokerFangsModel.h"
#include "../Minecraft.World/EvokerFangs.h"

ResourceLocation EvokerFangsRenderer::FANGS_LOCATION = ResourceLocation(TN_MOB_EVOKER_FANGS);

EvokerFangsRenderer::EvokerFangsRenderer()
{
	model = new EvokerFangsModel();
	shadowRadius = 0.0f;
}

void EvokerFangsRenderer::render(shared_ptr<Entity> _fangs, double x, double y, double z, float rot, float a)
{
	shared_ptr<EvokerFangs> fangs = dynamic_pointer_cast<EvokerFangs>(_fangs);
	if (fangs == nullptr) return;

	float progress = fangs->getAnimationProgress(a);
	if (progress == 0.0f) return;		// still hidden underground

	// the jaws grow out of the ground, then shrink away again at the end of their life
	float size = 2.0f;
	if (progress > 0.9f) size *= (1.0f - progress) / 0.1f;

	glPushMatrix();
	glDisable(GL_CULL_FACE);
	bindTexture(fangs);
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(90.0f - fangs->yRot, 0, 1, 0);
	glScalef(-size, -size, size);
	glTranslatef(0.0f, -0.626f, 0.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	model->render(fangs, progress, 0.0f, 0.0f, fangs->yRot, fangs->xRot, 0.03125f, true);
	glPopMatrix();
	glEnable(GL_CULL_FACE);
}

ResourceLocation *EvokerFangsRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &FANGS_LOCATION;
}
