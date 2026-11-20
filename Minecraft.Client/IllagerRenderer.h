#pragma once
#include "HumanoidMobRenderer.h"

// neoLegacy - draws the illagers that share IllagerModel (Vindicator, Evoker). Each type has its own skin.
class IllagerRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation VINDICATOR_LOCATION;
	static ResourceLocation EVOKER_LOCATION;
	static ResourceLocation PILLAGER_LOCATION;
	static ResourceLocation PROVOKER_LOCATION;
	static ResourceLocation SORCERER_LOCATION;
	static ResourceLocation ALCHEMIST_LOCATION;
	static ResourceLocation ARCHIVIST_LOCATION;
	static ResourceLocation NECROMANCER_LOCATION;

public:
	// texW / texH: the skin's size; hatKind: an IllagerModel::Hat (0 none, 1 the Sorcerer's wizard hat, 2 the Alchemist's brim)
	IllagerRenderer(int texW = 64, int texH = 64, int hatKind = 0);

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void createArmorParts();
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
