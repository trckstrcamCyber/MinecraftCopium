#pragma once
#include "Model.h"
#include "ModelPart.h"

class ShulkerModel : public Model
{
public:
	ModelPart *lid;
	ModelPart *base;
	ModelPart *head;

	bool mainPass;

	ShulkerModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
