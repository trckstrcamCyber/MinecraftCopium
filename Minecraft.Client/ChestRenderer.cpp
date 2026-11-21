#include "stdafx.h"
#include "ChestRenderer.h"
#include "ChestModel.h"
#include "LargeChestModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Calendar.h"

ResourceLocation ChestRenderer::CHEST_LARGE_TRAP_LOCATION = ResourceLocation(TN_TILE_LARGE_TRAP_CHEST);
//ResourceLocation ChestRenderer::CHEST_LARGE_XMAS_LOCATION = ResourceLocation(TN_TILE_LARGE_XMAS_CHEST);
ResourceLocation ChestRenderer::CHEST_LARGE_LOCATION = ResourceLocation(TN_TILE_LARGE_CHEST);
ResourceLocation ChestRenderer::CHEST_TRAP_LOCATION = ResourceLocation(TN_TILE_TRAP_CHEST);
//ResourceLocation ChestRenderer::CHEST_XMAS_LOCATION = ResourceLocation(TN_TILE_XMAS_CHEST);
ResourceLocation ChestRenderer::CHEST_LOCATION = ResourceLocation(TN_TILE_CHEST);
// neoLegacy - Copper Age (Phase 3). Copper chest entity textures, one per
// oxidation state (waxed reuses the unwaxed texture).
ResourceLocation ChestRenderer::CHEST_COPPER_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST);
ResourceLocation ChestRenderer::CHEST_COPPER_EXPOSED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_EXPOSED);
ResourceLocation ChestRenderer::CHEST_COPPER_WEATHERED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_WEATHERED);
ResourceLocation ChestRenderer::CHEST_COPPER_OXIDIZED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_OXIDIZED);
// neoLegacy - Copper Age (Phase 4). Copper double-chest sheets (128x64),
// combined from the jar1201 left/right textures. The two halves of a
// double chest oxidize independently, so each half binds its own sheet.
ResourceLocation ChestRenderer::CHEST_COPPER_LARGE_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_LARGE);
ResourceLocation ChestRenderer::CHEST_COPPER_LARGE_EXPOSED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_LARGE_EXPOSED);
ResourceLocation ChestRenderer::CHEST_COPPER_LARGE_WEATHERED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_LARGE_WEATHERED);
ResourceLocation ChestRenderer::CHEST_COPPER_LARGE_OXIDIZED_LOCATION = ResourceLocation(TN_TILE_COPPER_CHEST_LARGE_OXIDIZED);

ChestRenderer::ChestRenderer() : TileEntityRenderer()
{
	chestModel = new ChestModel();
	largeChestModel = new LargeChestModel();

	xmasTextures = false;

	// 4J Stu - Disable this
#if 0
	if (Calendar::GetMonth() + 1 == 12 && Calendar::GetDayOfMonth() >= 24 && Calendar::GetDayOfMonth() <= 26)
	{
		xmasTextures = true;
	}
#endif
}

ChestRenderer::~ChestRenderer()
{
	delete chestModel;
	delete largeChestModel;
}

void ChestRenderer::render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J Convert as we aren't using a templated class
	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(_chest);

	int data;

	if (!chest->hasLevel())
	{
		// neoLegacy - Copper Age (Round 5): a level-less chest is a held /
		// inventory / dropped item (EntityTileRenderer). Its cached
		// TileEntity::data was set from the item aux by the caller
		// (oxidation for a copper chest); -1 means "not set", i.e. a plain
		// chest. TileEntity::getData() would dereference the null level
		// when data is -1, so read the public member directly.
		data = chest->data >= 0 ? chest->data : 0;
	}
	else
	{
		Tile *tile = chest->getTile();
		data = chest->getData();

		// neoLegacy - Copper Age (Phase 3): the facing is bits 0-2 of the
		// data; bits 3-5 hold the copper chest's oxidation. Test the facing
		// only here (a freshly placed chest has facing 0).
		if (dynamic_cast<ChestTile*>(tile) != nullptr && (data & 7) == 0)
		{
			static_cast<ChestTile *>(tile)->recalcLockDir(chest->getLevel(), chest->x, chest->y, chest->z);
			data = chest->getData();
		}

		chest->checkNeighbors();
	}
	if (chest->n.lock() != nullptr || chest->w.lock() != nullptr) return;

	// neoLegacy - Copper Age (Phase 3). Copper chest texture by oxidation
	// state (bits 3-5 of the data; waxed reuses the unwaxed texture).
	// neoLegacy - Copper Age (Phase 4). The double-chest form uses the
	// combined 128x64 sheet (one per oxidation state); each half of a
	// double chest binds its own sheet, since the halves oxidize
	// independently. The single-chest texture is unchanged.
	ResourceLocation *copperLocation = &CHEST_COPPER_LOCATION;
	ResourceLocation *copperLargeLocation = &CHEST_COPPER_LARGE_LOCATION;
	switch ((data >> 3) & 3)
	{
	case 1: copperLocation = &CHEST_COPPER_EXPOSED_LOCATION; copperLargeLocation = &CHEST_COPPER_LARGE_EXPOSED_LOCATION; break;
	case 2: copperLocation = &CHEST_COPPER_WEATHERED_LOCATION; copperLargeLocation = &CHEST_COPPER_LARGE_WEATHERED_LOCATION; break;
	case 3: copperLocation = &CHEST_COPPER_OXIDIZED_LOCATION; copperLargeLocation = &CHEST_COPPER_LARGE_OXIDIZED_LOCATION; break;
	default: break;
	}

	ChestModel *model;
	if (chest->e.lock() != nullptr || chest->s.lock() != nullptr)
	{
		model = largeChestModel;

		if (chest->getType() == ChestTile::TYPE_COPPER)
		{
			bindTexture(copperLargeLocation);
		}
		else if (chest->getType() == ChestTile::TYPE_TRAP)
		{
			bindTexture(&CHEST_LARGE_TRAP_LOCATION);
		}
		//else if (xmasTextures)
		//{
		//	bindTexture(&CHEST_LARGE_XMAS_LOCATION);
		//}
		else
		{
			bindTexture(&CHEST_LARGE_LOCATION);
		}
	}
	else
	{
		model = chestModel;
		if (chest->getType() == ChestTile::TYPE_COPPER)
		{
			bindTexture(copperLocation);
		}
		else if (chest->getType() == ChestTile::TYPE_TRAP)
		{
			bindTexture(&CHEST_TRAP_LOCATION);
		}
		//else if (xmasTextures)
		//{
		//	bindTexture(&CHEST_XMAS_LOCATION);
		//}
		else
		{
			bindTexture(&CHEST_LOCATION);
		}
	}

	glPushMatrix();
	glEnable(GL_RESCALE_NORMAL);
	//if( setColor ) glColor4f(1, 1, 1, 1);
	if( setColor ) glColor4f(1, 1, 1, alpha);
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 1, static_cast<float>(z) + 1);
	glScalef(1, -1, -1);

	glTranslatef(0.5f, 0.5f, 0.5f);
	// neoLegacy - Copper Age (Phase 3): the facing is bits 0-2 of the data;
	// bits 3-5 hold the copper chest's oxidation.
	int facing = data & 7;
	int rot = 0;
	if (facing == 2) rot = 180;
	if (facing == 3) rot = 0;
	if (facing == 4) rot = 90;
	if (facing == 5) rot = -90;

	if (facing == 2 && chest->e.lock() != nullptr)
	{
		glTranslatef(1, 0, 0);
	}
	if (facing == 5 && chest->s.lock() != nullptr)
	{
		glTranslatef(0, 0, -1);
	}
	glRotatef(rot, 0, 1, 0);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	float open = chest->oOpenness + (chest->openness - chest->oOpenness) * a;
	if (chest->n.lock() != nullptr)
	{
		float open2 = chest->n.lock()->oOpenness + (chest->n.lock()->openness - chest->n.lock()->oOpenness) * a;
		if (open2 > open) open = open2;
	}
	if (chest->w.lock() != nullptr)
	{
		float open2 = chest->w.lock()->oOpenness + (chest->w.lock()->openness - chest->w.lock()->oOpenness) * a;
		if (open2 > open) open = open2;
	}

	open = 1 - open;
	open = 1 - open * open * open;

	model->lid->xRot = -(open * PI / 2);
	model->render(useCompiled);
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if( setColor ) glColor4f(1, 1, 1, 1);
}
