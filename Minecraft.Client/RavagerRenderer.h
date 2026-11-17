#pragma once

#include "MobRenderer.h"

class RavagerModel;

// neoLegacy - Ravager
class RavagerRenderer : public MobRenderer
{
private:
	static ResourceLocation RAVAGER_LOCATION;

public:
	RavagerRenderer();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
