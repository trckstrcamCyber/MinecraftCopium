#pragma once
#include "ZombieModel.h"

// neoLegacy - the illager body (Vindicator, Evoker; Pillager and friends later) on the modern 64x64 illager skin layout:
// tall head with the big nose, robed body, and two arm sets - a crossed-arms bundle for when it is calm, and ordinary swinging
// arms (raised weapon / casting pose) for when it is hunting or casting.
class IllagerModel : public ZombieModel
{
public:
	IllagerModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);

private:
	ModelPart *nose;
	ModelPart *crossedArms;
	bool showCrossedArms;
	static void rebuild(ModelPart *part, int u, int v, bool mirror, float x, float y, float z, int w, int h, int d, float g);
};
