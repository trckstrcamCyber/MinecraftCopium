#pragma once
#include "QuadrupedModel.h"

// neoLegacy - Camel model, ported from the Java CamelModel (128x128 texture): long body with a hump
// and tail, a tall neck/head with ears, and four long legs. Sitting camels drop to the ground with
// their legs folded.
class CamelModel : public QuadrupedModel
{
public:
	// saddleLayer = true builds the Java "saddle layer" instead of the camel: the saddle pad, girth straps and the bridle.
	// It is a separate set of boxes with their own UV areas (the lower half of camel_saddle.png), so the saddle texture can't
	// simply be drawn over the normal camel model.
	CamelModel(float inflate = 0.0f, bool saddleLayer = false);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
};
