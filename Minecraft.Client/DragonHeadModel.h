#pragma once
#include "Model.h"
#include "ModelPart.h"

class DragonHeadModel : public Model
{
public:
	ModelPart *head;
	ModelPart *jaw;

	DragonHeadModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
