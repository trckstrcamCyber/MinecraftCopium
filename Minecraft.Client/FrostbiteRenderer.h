#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - Frostbite: zombie-shaped, drawn with FrostbiteModel and its own icy skin.
class FrostbiteRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation FROSTBITE_LOCATION;

public:
	FrostbiteRenderer();

protected:
	virtual void createArmorParts();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
