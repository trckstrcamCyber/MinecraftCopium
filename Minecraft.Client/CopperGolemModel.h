#pragma once

#include "Model.h"

class ModelPart;

// neoLegacy - Copper Age (Phase 5). Model for the copper golem, ported
// from Java CopperGolemModel (copper-code.txt :1070). 64x64 texture.
// Geometry is transcribed from the Java model (feet at y=24, matching the
// -24 translate in LivingEntityRenderer):
//   body  (0,19,0)  tex (0,15)  box (-4,-6,-3, 8,6,6)
//   head  (0,13,0)  tex (0,0)   box (-4,-5,-5, 8,5,10)  + nose (56,0)
//                   (-1,-2,-6, 2,3,2) + antenna stem (37,8)
//                   (-1,-9,-1, 2,4,2) + antenna top (37,0)
//                   (-2,-13,-2, 4,4,4)
//   right arm (-4,13,0) tex (36,16) box (-3,-1,-2, 3,10,4)
//   left arm  (4,13,0)  tex (50,16) box (0,-1,-2, 3,10,4)
//   right leg (0,19,0)  tex (0,27)  box (-4,0,-2, 4,5,4)
//   left leg  (0,19,0)  tex (16,27) box (0,0,-2, 4,5,4)
// The head spins through the idle animation (see setupAnim): keyframes
// 0 -> -35 -> +35 -> 300 -> 360 degrees over the 70-tick spin.
// statuePose renders the statue poses (CopperGolemStatueBlock::POSE_*);
// -1 means "live golem" (walk/spin animation). The statue poses are
// procedural approximations on this model — playtest check.
class CopperGolemModel : public Model
{
public:
	ModelPart *head, *body, *arm0, *arm1, *leg0, *leg1;

	CopperGolemModel(float g = 0.0f);

	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
	void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);

	// Statue pose for the statue renderer (-1 = live golem).
	void setStatuePose(int pose);
	int getStatuePose();

private:
	int statuePose;
	float triangleWave(float bob, float period);
	float spinHeadAngle(float phase);
};
