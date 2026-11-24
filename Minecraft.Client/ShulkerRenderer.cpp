#include "stdafx.h"
#include "ShulkerRenderer.h"
#include "ShulkerModel.h"
#include "../Minecraft.World/Shulker.h"
#include "../Minecraft.World/Facing.h"

ResourceLocation ShulkerRenderer::SHULKER_LOCATION = ResourceLocation(TN_MOB_SHULKER);

ShulkerRenderer::ShulkerRenderer() : MobRenderer(new ShulkerModel(), 0.0f)
{
	shulkerModel = static_cast<ShulkerModel *>(resModel);
}

void ShulkerRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	shared_ptr<Shulker> shulker = dynamic_pointer_cast<Shulker>(_mob);
	int interpolation = shulker->getClientSideTeleportInterpolation();
	if (interpolation > 0 && shulker->hasOldAttachPosition())
	{
		double progress = (interpolation - a) / 6.0;
		progress *= progress;
		x -= (shulker->getAttachX() - shulker->getOldAttachX()) * progress;
		y -= (shulker->getAttachY() - shulker->getOldAttachY()) * progress;
		z -= (shulker->getAttachZ() - shulker->getOldAttachZ()) * progress;
	}
	MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation *ShulkerRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &SHULKER_LOCATION;
}

void ShulkerRenderer::setupRotations(shared_ptr<LivingEntity> mob, float bob, float bodyRot, float a)
{
	MobRenderer::setupRotations(mob, bob, bodyRot, a);

	switch (dynamic_pointer_cast<Shulker>(mob)->getAttachFace())
	{
	case Facing::EAST:
		glTranslatef(0.5f, 0.5f, 0.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		break;
	case Facing::WEST:
		glTranslatef(-0.5f, 0.5f, 0.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		break;
	case Facing::NORTH:
		glTranslatef(0.0f, 0.5f, -0.5f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		break;
	case Facing::SOUTH:
		glTranslatef(0.0f, 0.5f, 0.5f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		break;
	case Facing::UP:
		glTranslatef(0.0f, 1.0f, 0.0f);
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		break;
	default:
		break;
	}
}

void ShulkerRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glScalef(0.999f, 0.999f, 0.999f);
}

void ShulkerRenderer::additionalRendering(shared_ptr<LivingEntity> mob, float a)
{
	MobRenderer::additionalRendering(mob, a);

	shared_ptr<Shulker> shulker = dynamic_pointer_cast<Shulker>(mob);
	if (shulker == nullptr || shulker->isInvisible()) return;

	glPushMatrix();
	switch (shulker->getAttachFace())
	{
	case Facing::EAST:
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(1.0f, -1.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		break;
	case Facing::WEST:
		glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(-1.0f, -1.0f, 0.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		break;
	case Facing::NORTH:
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, -1.0f, -1.0f);
		break;
	case Facing::SOUTH:
		glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, -1.0f, 1.0f);
		break;
	case Facing::UP:
		glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, -2.0f, 0.0f);
		break;
	default:
		break;
	}

	float headRot = mob->yHeadRotO + (mob->yHeadRot - mob->yHeadRotO) * a;
	float bodyRot = mob->yBodyRotO + (mob->yBodyRot - mob->yBodyRotO) * a;
	float headPitch = mob->xRotO + (mob->xRot - mob->xRotO) * a;
	shulkerModel->head->yRot = (headRot - bodyRot) * PI / 180.0f;
	shulkerModel->head->xRot = headPitch * PI / 180.0f;
	bindTexture(&SHULKER_LOCATION);
	shulkerModel->head->render(1.0f / 16.0f, true);
	glPopMatrix();
}

void ShulkerRenderer::renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	shulkerModel->mainPass = !EntityRenderer::solidColourPass;
	MobRenderer::renderModel(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale);
	shulkerModel->mainPass = false;
}
