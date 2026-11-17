#pragma once
#include "Model.h"
#include "ModelPart.h"

class DragonModel : public Model
{
public:
	static const int MODEL_ID = 4;

private:
	ModelPart *head;
	ModelPart *neck;
	ModelPart *jaw;
	ModelPart *body;

	ModelPart* rearLegLeft;
	ModelPart* frontLegLeft;
	ModelPart* rearLegTipLeft;
	ModelPart* frontLegTipLeft;
	ModelPart* rearFootLeft;
	ModelPart* frontFootLeft;
	ModelPart* wingLeft;
	ModelPart* wingTipLeft;

	ModelPart *rearLegRight;
	ModelPart *frontLegRight;
	ModelPart *rearLegTipRight;
	ModelPart *frontLegTipRight;
	ModelPart *rearFootRight;
	ModelPart *frontFootRight;
	ModelPart *wingRight;
	ModelPart *wingTipRight;
	float a;

public:

    ModelPart *cubes[5];
    DragonModel(float g);
	void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);

private:
	float rotWrap(double d);

};