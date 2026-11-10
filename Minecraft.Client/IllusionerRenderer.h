#pragma once
#include "IllagerRenderer.h"

// neoLegacy - Illusioner: draws the illager body with its own skin and, while it is invisible, four mirror images dancing around it.
class IllusionerRenderer : public IllagerRenderer
{
private:
	static ResourceLocation ILLUSIONER_LOCATION;
public:
	IllusionerRenderer();
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);

protected:
	// the normal renderer hides invisible mobs; here the images ARE the visible part
	virtual void renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
