#include "stdafx.h"
#include "IllusionerRenderer.h"
#include "../Minecraft.World/Illusioner.h"

ResourceLocation IllusionerRenderer::ILLUSIONER_LOCATION = ResourceLocation(TN_MOB_ILLUSIONER);

IllusionerRenderer::IllusionerRenderer() : IllagerRenderer()
{
}

ResourceLocation *IllusionerRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &ILLUSIONER_LOCATION;
}

void IllusionerRenderer::renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	bindTexture(mob);
	resModel->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
}

void IllusionerRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	shared_ptr<Illusioner> illusioner = dynamic_pointer_cast<Illusioner>(_mob);
	if (illusioner != nullptr && illusioner->isInvisible())
	{
		// invisible: no real body, just four shimmering copies around where it stands
		double offsets[4][3];
		illusioner->getRenderLocations(a, offsets);
		float t = illusioner->tickCount + a;
		for (int i = 0; i < 4; i++)
		{
			IllagerRenderer::render(_mob,
				x + offsets[i][0] + cos(i + t * 0.5f) * 0.025,
				y + offsets[i][1] + cos(i + t * 0.75f) * 0.0125,
				z + offsets[i][2] + cos(i + t * 0.7f) * 0.025,
				rot, a);
		}
	}
	else
	{
		IllagerRenderer::render(_mob, x, y, z, rot, a);
	}
}
