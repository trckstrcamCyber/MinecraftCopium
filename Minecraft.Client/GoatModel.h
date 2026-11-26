#pragma once
#include "QuadrupedModel.h"

// neoLegacy - Goat model, laid out exactly like the modern Java goat (64x64 texture):
// horizontal body + chest block, angled nose, goatee, two horns, two ears, short hind legs and long
// front legs. While the goat is ramming its head drops to a charging pose.
class GoatModel : public QuadrupedModel
{
public:
	ModelPart *nose;
	ModelPart *leftHorn;	// own parts (children of the head) so a lost horn can be hidden
	ModelPart *rightHorn;	// via DATA_HORNS — see GoatModel::setupAnim
	GoatModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
};
