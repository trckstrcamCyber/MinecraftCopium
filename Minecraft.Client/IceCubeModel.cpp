#include "stdafx.h"
#include "IceCubeModel.h"
#include "ModelPart.h"

IceCubeModel::IceCubeModel()
{
	texWidth = 128;
	texHeight = 128;

	everything = new ModelPart(this);
	everything->setPos(-2.0f, 9.0f, 1.0f);
	everything->bMirror = true;		// every box of this model is mirrored
	everything->texOffs(64, 0);   everything->addBox(-14.0f, -1.0f, -17.0f, 16, 16, 16, 0.0f);
	everything->texOffs(0, 40);   everything->addBox(2.0f, -1.0f, -17.0f, 16, 8, 16, 0.0f);
	everything->texOffs(0, 0);    everything->addBox(-14.0f, -9.0f, -1.0f, 16, 24, 16, 0.0f);
	everything->texOffs(64, 32);  everything->addBox(2.0f, -9.0f, -1.0f, 16, 16, 16, 0.0f);
	everything->texOffs(0, 64);   everything->addBox(-14.0f, -9.0f, -9.0f, 16, 8, 8, 0.0f);
	everything->bMirror = false;
	everything->compile(1.0f / 16.0f);
}

void IceCubeModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	everything->render(scale, usecompiled);
}
