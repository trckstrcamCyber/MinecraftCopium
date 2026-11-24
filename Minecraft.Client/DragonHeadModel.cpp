#include "stdafx.h"
#include "DragonHeadModel.h"
#include "../Minecraft.World/Mth.h"

DragonHeadModel::DragonHeadModel() : Model()
{
	texWidth = 256;
	texHeight = 256;

	setMapTex(L"head.upperhead", 112, 30);
	setMapTex(L"head.upperlip", 176, 44);
	setMapTex(L"jaw.jaw", 176, 65);
	setMapTex(L"head.scale", 0, 0);
	setMapTex(L"head.nostril", 112, 0);

	head = new ModelPart(this, L"head");
	head->addBox(L"upperlip", -6, -1, -24, 12, 5, 16);
	head->addBox(L"upperhead", -8, -8, -10, 16, 16, 16);
	head->bMirror = true;
	head->addBox(L"scale", -5, -12, -4, 2, 4, 6);
	head->addBox(L"nostril", -5, -3, -22, 2, 2, 4);
	head->bMirror = false;
	head->addBox(L"scale", 3, -12, -4, 2, 4, 6);
	head->addBox(L"nostril", 3, -3, -22, 2, 2, 4);

	jaw = new ModelPart(this, L"jaw");
	jaw->setPos(0, 4, -8);
	jaw->addBox(L"jaw", -6, 0, -16, 12, 4, 16);
	head->addChild(jaw);

	head->compile(1.0f / 16.0f);
	jaw->compile(1.0f / 16.0f);
}

void DragonHeadModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	jaw->xRot = (Mth::sin(time * PI * 0.2f) + 1.0f) * 0.2f;
	head->yRot = yRot * PI / 180.0f;
	head->xRot = xRot * PI / 180.0f;

	glTranslatef(0.0f, -0.374375f, 0.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	head->render(scale, usecompiled);
}
