#pragma once

#include "Model.h"

class ModelPart;

// neoLegacy - Ravager model (from the Blockbench export Ravanger.java, 128x128 texture): body, neck > head > (mouth, horns), four legs.
class RavagerModel : public Model
{
public:
	ModelPart *body, *neck, *head, *mouth, *horns, *leg0, *leg1, *leg2, *leg3;

	RavagerModel();

	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
	void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);

private:
	static const float NECK_REST_Z;
	float triangleWave(float bob, float period);
};
