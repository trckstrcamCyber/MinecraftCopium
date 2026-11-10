#pragma once
#include "Model.h"

class ModelPart;

// neoLegacy - Iceologer (from the FrozenFiend mod's Blockbench model, 128x64 skin): a hooded illager with a flowing cape.
class IceologerModel : public Model
{
private:
	ModelPart *head;
	ModelPart *body;
	ModelPart *cape;
	ModelPart *leg0;
	ModelPart *leg1;
	ModelPart *arm0;
	ModelPart *arm1;
public:
	IceologerModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
	virtual void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);
};
