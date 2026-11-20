#pragma once
#include "ZombieModel.h"

// neoLegacy - the illager body (Vindicator, Evoker; Pillager and friends later) on the modern 64x64 illager skin layout:
// tall head with the big nose, robed body, and two arm sets - a crossed-arms bundle for when it is calm, and ordinary swinging
// arms (raised weapon / casting pose) for when it is hunting or casting.
class IllagerModel : public ZombieModel
{
public:
	// Hats from the Illager Invasion mod: the Sorcerer's tall wizard hat and the Alchemist's big brim are extra parts hanging off the hat layer.
	// Their skins are bigger than the standard 64x64 (Sorcerer 64x128, Alchemist 128x64), so the model takes the skin size.
	enum Hat { HAT_NONE = 0, HAT_WIZARD = 1, HAT_ALCHEMIST = 2 };
	IllagerModel(int texW = 64, int texH = 64, Hat hatKind = HAT_NONE);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);

private:
	ModelPart *nose;
	ModelPart *crossedArms;
	bool showCrossedArms;
	static void rebuild(ModelPart *part, int u, int v, bool mirror, float x, float y, float z, int w, int h, int d, float g);
};
