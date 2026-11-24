#include "stdafx.h"
#include "ShulkerBulletRenderer.h"
#include "Model.h"
#include "ModelPart.h"
#include "../Minecraft.World/ShulkerBullet.h"
#include "../Minecraft.World/Mth.h"

ResourceLocation ShulkerBulletRenderer::SPARK_LOCATION = ResourceLocation(TN_MOB_SHULKER_SPARK);

namespace
{
	class ShulkerBulletModel : public Model
	{
	public:
		ShulkerBulletModel() { texWidth = 64; texHeight = 32; }
	};

	float rotlerp(float from, float to, float a)
	{
		float diff = to - from;
		while (diff < -180.0f) diff += 360.0f;
		while (diff >= 180.0f) diff -= 360.0f;
		return from + a * diff;
	}
}

ShulkerBulletRenderer::ShulkerBulletRenderer() : EntityRenderer()
{
	bulletModel = new ShulkerBulletModel();
	bulletPart = new ModelPart(bulletModel, 0, 0);
	bulletPart->addBox(-4.0f, -4.0f, -1.0f, 8, 8, 2);
	bulletPart->texOffs(0, 10)->addBox(-1.0f, -4.0f, -4.0f, 2, 8, 8);
	bulletPart->texOffs(20, 0)->addBox(-4.0f, -1.0f, -4.0f, 8, 2, 8);
	bulletPart->setPos(0.0f, 0.0f, 0.0f);
	bulletPart->compile(1.0f / 16.0f);
}

void ShulkerBulletRenderer::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a)
{
	float yRot = rotlerp(entity->yRotO, entity->yRot, a);
	float xRot = entity->xRotO + (entity->xRot - entity->xRotO) * a;
	float t = entity->tickCount + a;

	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 0.15f, static_cast<float>(z));
	glRotatef(Mth::sin(t * 0.05f) * 180.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(Mth::cos(t * 0.05f) * 180.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(Mth::sin(t * 0.075f) * 360.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);
	glScalef(-1.0f, -1.0f, 1.0f);

	bulletPart->yRot = yRot * PI / 180.0f;
	bulletPart->xRot = xRot * PI / 180.0f;

	bindTexture(entity);
	bulletPart->render(1.0f / 16.0f, true);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glScalef(1.5f, 1.5f, 1.5f);
	bulletPart->render(1.0f / 16.0f, true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}

ResourceLocation *ShulkerBulletRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &SPARK_LOCATION;
}
