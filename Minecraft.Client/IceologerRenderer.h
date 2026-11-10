#pragma once
#include "MobRenderer.h"

// neoLegacy - Iceologer: its own model and skin, drawn at 15/16 size like the other illagers.
class IceologerRenderer : public MobRenderer
{
private:
	static ResourceLocation ICEOLOGER_LOCATION;
public:
	IceologerRenderer();
protected:
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
