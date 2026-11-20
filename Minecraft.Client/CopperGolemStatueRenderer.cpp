#include "stdafx.h"
#include "CopperGolemStatueRenderer.h"
#include "CopperGolemModel.h"
#include "../Minecraft.World/CopperGolemStatueBlock.h"
#include "../Minecraft.World/CopperGolemStatueBlockEntity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"

// The statue reuses the golem's body textures (one per oxidation state).
ResourceLocation CopperGolemStatueRenderer::STATUE_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM);
ResourceLocation CopperGolemStatueRenderer::STATUE_EXPOSED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_EXPOSED);
ResourceLocation CopperGolemStatueRenderer::STATUE_WEATHERED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_WEATHERED);
ResourceLocation CopperGolemStatueRenderer::STATUE_OXIDIZED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_OXIDIZED);

CopperGolemStatueRenderer::CopperGolemStatueRenderer() : TileEntityRenderer()
{
	statueModel = new CopperGolemModel();
}

CopperGolemStatueRenderer::~CopperGolemStatueRenderer()
{
	delete statueModel;
}

void CopperGolemStatueRenderer::render(shared_ptr<TileEntity> _statue, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<CopperGolemStatueBlockEntity> statue = dynamic_pointer_cast<CopperGolemStatueBlockEntity>(_statue);

	int data;
	if (statue == nullptr || !statue->hasLevel())
	{
		data = 0;
	}
	else
	{
		data = statue->getData();
	}

	// Body texture by oxidation state (bits 0-1; waxed reuses the unwaxed
	// texture, same as the golem).
	ResourceLocation *location = &STATUE_LOCATION;
	switch (CopperGolemStatueBlock::getOxidation(data))
	{
	case 1: location = &STATUE_EXPOSED_LOCATION; break;
	case 2: location = &STATUE_WEATHERED_LOCATION; break;
	case 3: location = &STATUE_OXIDIZED_LOCATION; break;
	default: break;
	}
	bindTexture(location);

	glPushMatrix();
	glEnable(GL_RESCALE_NORMAL);
	if (setColor) glColor4f(1, 1, 1, alpha);
	// LivingEntityRenderer pattern, not the ChestRenderer pattern: the
	// golem model is an entity model (feet at model y=24), so stand it on
	// the centre of the block's base like a living entity.
	glTranslatef(static_cast<float>(x) + 0.5f, static_cast<float>(y), static_cast<float>(z) + 0.5f);
	glRotatef(180.0f - CopperGolemStatueBlock::facingToYaw(CopperGolemStatueBlock::getFacing(data)), 0, 1, 0);
	glScalef(-1, -1, 1);
	glTranslatef(0, -24.0f / 16.0f - 0.125f / 16.0f, 0);

	statueModel->setStatuePose(CopperGolemStatueBlock::getPose(data));
	statueModel->render(shared_ptr<Entity>(), 0, 0, 0, 0, 0, 1 / 16.0f, useCompiled);
	statueModel->setStatuePose(-1);

	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if (setColor) glColor4f(1, 1, 1, 1);
}
