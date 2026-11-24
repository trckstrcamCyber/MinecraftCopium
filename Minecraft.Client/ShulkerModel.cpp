#include "stdafx.h"
#include "ShulkerModel.h"
#include "../Minecraft.World/Shulker.h"
#include "../Minecraft.World/Mth.h"
#include "Minecraft.h"
#include "Common/Colours/ColourTable.h"

ShulkerModel::ShulkerModel() : Model()
{
	texWidth = 64;
	texHeight = 128;
	mainPass = false;

	lid = new ModelPart(this, 0, 0);
	lid->addBox(-8.0f, -16.0f, -8.0f, 16, 12, 16, 0.01f);
	lid->setPos(0.0f, 24.0f, 0.0f);

	base = new ModelPart(this, 0, 28);
	base->addBox(-8.0f, -8.0f, -8.0f, 16, 8, 16);
	base->setPos(0.0f, 24.0f, 0.0f);

	head = new ModelPart(this, 0, 116);
	head->addBox(-3.0f, 0.0f, -3.0f, 6, 6, 6);
	head->setPos(0.0f, 12.0f, 0.0f);

	lid->compile(1.0f / 16.0f);
	base->compile(1.0f / 16.0f);
	head->compile(1.0f / 16.0f);
}

void ShulkerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	shared_ptr<Shulker> shulker = dynamic_pointer_cast<Shulker>(entity);
	if (shulker == nullptr) return;

	float a = bob - static_cast<float>(shulker->tickCount);
	float peek = shulker->getClientPeekAmount(a);
	float peekAngle = (peek + 0.5f) * PI;
	float lidTwist = Mth::sin(peekAngle) - 1.0f;
	float wobble = peekAngle > PI ? Mth::sin(bob * 0.1f) * 0.7f : 0.0f;

	lid->setPos(0.0f, 16.0f + Mth::sin(peekAngle) * 8.0f + wobble, 0.0f);
	lid->yRot = peek > 0.3f ? lidTwist * lidTwist * lidTwist * lidTwist * PI * 0.125f : 0.0f;

	head->xRot = xRot * PI / 180.0f;
	head->yRot = yRot * PI / 180.0f;
}

void ShulkerModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	if (!mainPass)
	{
		base->render(scale, usecompiled);
		lid->render(scale, usecompiled);
		return;
	}

	const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Mob_Shulker_Shell);
	glColor3f(((colour >> 16) & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, (colour & 0xff) / 255.0f);
	base->render(scale, usecompiled);
	lid->render(scale, usecompiled);
	glColor3f(1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
	base->render(scale, usecompiled);
	lid->render(scale, usecompiled);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}
