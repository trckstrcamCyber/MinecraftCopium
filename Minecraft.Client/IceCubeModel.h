#pragma once
#include "Model.h"

class ModelPart;

// neoLegacy - the Ice Cube: a lumpy 2x1.5 block of ice (128x128 skin) from the FrozenFiend mod's Blockbench model.
class IceCubeModel : public Model
{
private:
	ModelPart *everything;
public:
	IceCubeModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
