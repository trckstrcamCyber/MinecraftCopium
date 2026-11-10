#pragma once
#include "Model.h"

class ModelPart;

// neoLegacy - the evoker's snapping jaws (64x32 texture): a base block with an upper and a lower jaw that bite shut.
class EvokerFangsModel : public Model
{
private:
	ModelPart *base;
	ModelPart *upperJaw;
	ModelPart *lowerJaw;
public:
	EvokerFangsModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
};
