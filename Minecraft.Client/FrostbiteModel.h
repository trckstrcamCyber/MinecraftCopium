#pragma once
#include "ZombieModel.h"

// neoLegacy - Frostbite: a zombie body on its own 64x64 skin layout (taken from the Blockbench model: head (0,0), body (32,0),
// right arm (16,16), left arm (16,32), left leg (0,16), right leg (0,32), with inflated overlay layers on the arms and legs).
// The standard humanoid parts are rebuilt with those UVs, the hat/jacket layers are emptied and the sleeves/pants become the overlays.
// Animation: arms hang and swing while it is idle, rise straight out in front when it has a target, and it always shivers a little.
class FrostbiteModel : public ZombieModel
{
public:
	FrostbiteModel();
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);

private:
	static void rebuild(ModelPart *part, int u, int v, bool mirror, float x, float y, float z, int w, int h, int d, float g);
	void syncOverlays();
};
