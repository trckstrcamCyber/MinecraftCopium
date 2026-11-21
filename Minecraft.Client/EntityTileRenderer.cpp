#include "stdafx.h"
#include "EntityTileRenderer.h"
#include "TileEntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"

EntityTileRenderer *EntityTileRenderer::instance = new EntityTileRenderer;

EntityTileRenderer::EntityTileRenderer()
{
	chest = std::make_shared<ChestTileEntity>();
	trappedChest = std::make_shared<ChestTileEntity>(ChestTile::TYPE_TRAP);
	enderChest = std::make_shared<EnderChestTileEntity>();
	// neoLegacy - Copper Age (Round 5): held / inventory / dropped
	// rendering for the copper chest and the golem statue. These used to
	// fall through to the vanilla chest below, so the statue (and a copper
	// chest) rendered as a plain oak chest in the hand. The copper chest
	// TE carries TYPE_COPPER so ChestRenderer binds the copper texture;
	// the statue TE dispatches (by GetType) to CopperGolemStatueRenderer.
	copperChest = std::make_shared<ChestTileEntity>(ChestTile::TYPE_COPPER);
	statue = std::make_shared<CopperGolemStatueBlockEntity>();
}

void EntityTileRenderer::render(Tile *tile, int data, float brightness, float alpha, bool setColor, bool useCompiled)
{
	if (tile->id == Tile::ender_chest_Id)
	{
		TileEntityRenderDispatcher::instance->render(enderChest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else if (tile->id == Tile::chest_trap_Id)
	{
		TileEntityRenderDispatcher::instance->render(trappedChest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else if (tile->id == Tile::copper_chest_Id)
	{
		// neoLegacy - Copper Age (Round 5): item aux is the oxidation
		// number (ChestTile::getSpawnResourcesAuxValue returns
		// (data >> 3) & 7), so rebuild block data with facing 0; the
		// renderer reads the cached TileEntity::data when there is no
		// level (see ChestRenderer::render).
		copperChest->data = (data & 7) << 3;
		TileEntityRenderDispatcher::instance->render(copperChest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else if (tile->id == Tile::copper_golem_statue_Id)
	{
		// neoLegacy - Copper Age (Round 5): item aux carries the
		// oxidation bits (0-2); pose/facing are placement-time, so a held
		// statue reads STANDING/NORTH. The renderer reads the cached
		// TileEntity::data when there is no level (see
		// CopperGolemStatueRenderer::render).
		statue->data = data;
		TileEntityRenderDispatcher::instance->render(statue, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
	else
	{
		TileEntityRenderDispatcher::instance->render(chest, 0, 0, 0, 0, setColor, alpha, useCompiled);
	}
}
