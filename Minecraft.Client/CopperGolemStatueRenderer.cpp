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
	// Oxidation workaround: pose/facing live in the TE, not the nibble
	// — read them from the cast TE. Oxidation stays in the nibble
	// (data & 3). A level-less TE (held/inventory item) still carries
	// its pose/facing fields, so read them from the TE in both branches;
	// only the oxidation source differs.
	int pose = CopperGolemStatueBlock::POSE_STANDING;
	int facing = CopperGolemStatueBlock::FACING_NORTH;
	if (statue == nullptr || !statue->hasLevel())
	{
		// neoLegacy - Copper Age (Round 5): a level-less statue is a held /
		// inventory / dropped item (EntityTileRenderer). Its cached
		// TileEntity::data was set from the item aux by the caller
		// (oxidation bits 0-2; pose/facing are placement-time, so they
		// read 0 = STANDING/NORTH here). TileEntity::getData() would
		// dereference the null level when data is -1, so read the public
		// member directly; -1 means "not set".
		data = (statue != nullptr && statue->data >= 0) ? statue->data : 0;
	}
	else
	{
		data = statue->getData();
	}
	if (statue != nullptr)
	{
		pose = statue->getPose();
		facing = statue->getFacing();
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
	glRotatef(180.0f - CopperGolemStatueBlock::facingToYaw(facing), 0, 1, 0);
	glScalef(-1, -1, 1);
	glTranslatef(0, -24.0f / 16.0f - 0.125f / 16.0f, 0);

	statueModel->setStatuePose(pose);
	statueModel->render(shared_ptr<Entity>(), 0, 0, 0, 0, 0, 1 / 16.0f, useCompiled);
	statueModel->setStatuePose(-1);

	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if (setColor) glColor4f(1, 1, 1, 1);
}
