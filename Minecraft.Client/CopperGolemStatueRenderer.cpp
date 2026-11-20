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
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 1, static_cast<float>(z) + 1);
	glScalef(1, -1, -1);

	glTranslatef(0.5f, 0.5f, 0.5f);
	// Facing is bits 5-6 of the data (0 NORTH, 1 EAST, 2 SOUTH, 3 WEST);
	// rotation matches the ChestRenderer facing map (NORTH 180, SOUTH 0,
	// WEST 90, EAST -90).
	int rot = 0;
	switch (CopperGolemStatueBlock::getFacing(data))
	{
	case CopperGolemStatueBlock::FACING_NORTH: rot = 180; break;
	case CopperGolemStatueBlock::FACING_EAST: rot = -90; break;
	case CopperGolemStatueBlock::FACING_WEST: rot = 90; break;
	default: rot = 0; break;
	}
	glRotatef(static_cast<float>(rot), 0, 1, 0);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	// Pose is bits 3-4 of the data. The model applies the pose in
	// setupAnim; reset to -1 afterwards so the model never keeps a stale
	// pose if it is ever rendered without one.
	statueModel->setStatuePose(CopperGolemStatueBlock::getPose(data));
	statueModel->render(shared_ptr<Entity>(), 0, 0, 0, 0, 0, 1 / 16.0f, useCompiled);
	statueModel->setStatuePose(-1);

	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if (setColor) glColor4f(1, 1, 1, 1);
}
