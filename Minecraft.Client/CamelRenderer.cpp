#include "stdafx.h"
#include "CamelRenderer.h"
#include "CamelModel.h"
#include "../Minecraft.World/SharedConstants.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"

ResourceLocation CamelRenderer::CAMEL_LOCATION = ResourceLocation(TN_MOB_CAMEL);
ResourceLocation CamelRenderer::SADDLE_LOCATION = ResourceLocation(TN_MOB_CAMEL_SADDLE);

CamelRenderer::CamelRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
	setArmor(new CamelModel(0.0f, true));		// the saddle + bridle model, drawn with camel_saddle.png
}

int CamelRenderer::prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a)
{
	shared_ptr<Camel> camel = dynamic_pointer_cast<Camel>(_mob);
	if (layer == 0 && camel != nullptr && camel->isSaddled())
	{
		bindTexture(&SADDLE_LOCATION);
		float brightness = SharedConstants::TEXTURE_LIGHTING ? 1.0f : _mob->getBrightness(a);
		glColor3f(brightness, brightness, brightness);
		return 1;
	}
	return -1;
}

ResourceLocation *CamelRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &CAMEL_LOCATION;
}
