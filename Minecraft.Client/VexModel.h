#pragma once
#include "HumanoidModel.h"

// neoLegacy - Vex: a tiny floating humanoid with flapping wings on the 32x32 vex skin. Built on HumanoidModel so the sword in its
// hand is drawn by the normal held-item code; the legs are hidden and the arms swing up when it charges.
class VexModel : public HumanoidModel
{
public:
	VexModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);

private:
	ModelPart *leftWing;
	ModelPart *rightWing;
};
