#include "stdafx.h"
#include "VexRenderer.h"
#include "VexModel.h"
#include "../Minecraft.World/Vex.h"

ResourceLocation VexRenderer::VEX_LOCATION = ResourceLocation(TN_MOB_VEX);
ResourceLocation VexRenderer::VEX_CHARGING_LOCATION = ResourceLocation(TN_MOB_VEX_CHARGING);

VexRenderer::VexRenderer() : HumanoidMobRenderer(new VexModel(), 0.3f, 1.0f)
{
	createArmorParts();
}

void VexRenderer::createArmorParts()
{
	// a vex wears no armor: keep the (unused) armor models tiny and shared with the default humanoid
	delete armorParts1;
	delete armorParts2;
	armorParts1 = new HumanoidModel(1.0f, true);
	armorParts2 = new HumanoidModel(0.5f, true);
}

ResourceLocation *VexRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	shared_ptr<Vex> vex = dynamic_pointer_cast<Vex>(mob);
	return (vex != nullptr && vex->isCharging()) ? &VEX_CHARGING_LOCATION : &VEX_LOCATION;
}
