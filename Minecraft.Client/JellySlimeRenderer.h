#pragma once
#include "SlimeRenderer.h"

// neoLegacy - the Jelly Slime: a normal slime model with the pink Jelly Slime skin (mob/jelly_slime.png).
class JellySlimeRenderer : public SlimeRenderer
{
private:
	static ResourceLocation JELLY_SLIME_LOCATION;

public:
	JellySlimeRenderer(Model *model, Model *armor, float shadow);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
