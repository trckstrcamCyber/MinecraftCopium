#include "stdafx.h"
#include "IronThinTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

IronThinTile::IronThinTile(int id, const wstring &baseIconName,
	float x0, float y0, float z0, float x1, float y1, float z1)
	: CopperThinTile(id, baseIconName, x0, y0, z0, x1, y1, z1)
{
	ironIconName = baseIconName;
	ironIcon = nullptr;
}

int IronThinTile::getRenderShape()
{
	// Only the two iron ids ever reach this override (the tiles are built
	// as IronThinTile in Tile.cpp), so the fallthrough stays SHAPE_BLOCK.
	if (id == Tile::iron_chain_Id) return Tile::SHAPE_IRON_CHAIN;
	if (id == Tile::iron_lantern_Id || id == Tile::soul_lantern_Id) return Tile::SHAPE_IRON_LANTERN;
	return Tile::SHAPE_BLOCK;
}

Icon *IronThinTile::getTexture(int face, int data)
{
	// No oxidation states: the data bits are meaningless for iron.
	(void)face;
	(void)data;
	return ironIcon;
}

Icon *IronThinTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	// No oxidation states: the data bits are meaningless for iron.
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)face;
	return ironIcon;
}

void IronThinTile::registerIcons(IconRegister *iconRegister)
{
	// Exactly ONE icon -- no oxidation-prefixed variants exist for iron,
	// and registering them would DEBUG_BREAK (see IronThinTile.h).
	ironIcon = iconRegister->registerIcon(ironIconName);
	// Keep the base Tile::icon on a valid iron icon so anything that reads
	// the icon field directly (particles, pick-block) gets a valid icon.
	icon = ironIcon;
}

int IronThinTile::getSpawnResourcesAuxValue(int data)
{
	// Iron has no state bits: a broken iron chain / lantern always drops
	// as aux 0 (unlike copper, which preserves oxidation + wax in data & 7).
	(void)data;
	return 0;
}
