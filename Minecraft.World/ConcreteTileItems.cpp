#include "stdafx.h"
#include "ItemInstance.h"
#include "Player.h"
#include "Facing.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "ConcreteColorTileEntity.h"
#include "ConcreteSlabTile.h"
#include "ConcreteTileItems.h"

// ---------------------------------------------------------------------------------
// Stairs
// ---------------------------------------------------------------------------------

// Global 0-15 order, identical to ConcreteSlabTile::SLAB_NAMES / Tile::concrete
const unsigned int ConcreteStairTileItem::STAIRS_NAMES[16] =
{
	IDS_TILE_CONCRETE_STAIRS_BLACK,
	IDS_TILE_CONCRETE_STAIRS_RED,
	IDS_TILE_CONCRETE_STAIRS_GREEN,
	IDS_TILE_CONCRETE_STAIRS_BROWN,
	IDS_TILE_CONCRETE_STAIRS_BLUE,
	IDS_TILE_CONCRETE_STAIRS_PURPLE,
	IDS_TILE_CONCRETE_STAIRS_CYAN,
	IDS_TILE_CONCRETE_STAIRS_SILVER,
	IDS_TILE_CONCRETE_STAIRS_GRAY,
	IDS_TILE_CONCRETE_STAIRS_PINK,
	IDS_TILE_CONCRETE_STAIRS_LIME,
	IDS_TILE_CONCRETE_STAIRS_YELLOW,
	IDS_TILE_CONCRETE_STAIRS_LIGHT_BLUE,
	IDS_TILE_CONCRETE_STAIRS_MAGENTA,
	IDS_TILE_CONCRETE_STAIRS_ORANGE,
	IDS_TILE_CONCRETE_STAIRS_WHITE,
};

ConcreteStairTileItem::ConcreteStairTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *ConcreteStairTileItem::getIcon(int itemAuxValue)
{
	return Tile::concrete->getTexture(2, itemAuxValue & 15);
}

int ConcreteStairTileItem::getLevelDataForAuxValue(int auxValue)
{
	// Block data is orientation only; the color is applied by ConcreteStairTile::setPlacedBy.
	return 0;
}

unsigned int ConcreteStairTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return STAIRS_NAMES[instance->getAuxValue() & 15];
}

// ---------------------------------------------------------------------------------
// Slabs (port of StoneSlabTileItem, comparing the tile-entity color instead of the
// block data "type" bits)
// ---------------------------------------------------------------------------------

ConcreteSlabTileItem::ConcreteSlabTileItem(int id, HalfSlabTile *halfTile, HalfSlabTile *fullTile, bool full) : TileItem(id)
{
	this->halfTile = halfTile;
	this->fullTile = fullTile;

	isFull = full;
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *ConcreteSlabTileItem::getIcon(int itemAuxValue)
{
	return Tile::concrete->getTexture(2, itemAuxValue & 15);
}

int ConcreteSlabTileItem::getLevelDataForAuxValue(int auxValue)
{
	// Block data is top/bottom only; the color is applied by ConcreteSlabTile::setPlacedBy.
	return 0;
}

unsigned int ConcreteSlabTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return ConcreteSlabTile::SLAB_NAMES[instance->getAuxValue() & 15];
}

void ConcreteSlabTileItem::convertToFull(shared_ptr<ItemInstance> instance, Level *level, int x, int y, int z, int color)
{
	if (level->isUnobstructed(fullTile->getAABB(level, x, y, z)) && level->setTileAndData(x, y, z, fullTile->id, 0, Tile::UPDATE_ALL))
	{
		// Swapping the half slab for the full slab replaced the tile entity, so give the
		// new one the same color.
		ConcreteColorTileEntity::setColorAt(level, x, y, z, color);

		level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, fullTile->soundType->getPlaceSound(), (fullTile->soundType->getVolume() + 1) / 2, fullTile->soundType->getPitch() * 0.8f);
		instance->count--;
	}
}

bool ConcreteSlabTileItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (isFull)
	{
		return TileItem::useOn(instance, player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
	}

	if (instance->count == 0) return false;
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	int color = instance->getAuxValue() & 15;
	int currentTile = level->getTile(x, y, z);
	int currentData = level->getData(x, y, z);
	bool isUpper = (currentData & HalfSlabTile::TOP_SLOT_BIT) != 0;

	if (((face == Facing::UP && !isUpper) || (face == Facing::DOWN && isUpper)) && currentTile == halfTile->id && ConcreteColorTileEntity::getColorAt(level, x, y, z) == color)
	{
		if (bTestUseOnOnly)
		{
			return true;
		}

		convertToFull(instance, level, x, y, z, color);
		return true;
	}
	else if (tryConvertTargetTile(instance, player, level, x, y, z, face, bTestUseOnOnly))
	{
		return true;
	}
	else
	{
		return TileItem::useOn(instance, player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
	}
}

bool ConcreteSlabTileItem::mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item)
{
	int ox = x, oy = y, oz = z;
	int color = item->getAuxValue() & 15;

	int currentTile = level->getTile(x, y, z);
	int currentData = level->getData(x, y, z);
	bool isUpper = (currentData & HalfSlabTile::TOP_SLOT_BIT) != 0;

	if (((face == Facing::UP && !isUpper) || (face == Facing::DOWN && isUpper)) && currentTile == halfTile->id && ConcreteColorTileEntity::getColorAt(level, x, y, z) == color)
	{
		return true;
	}

	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	currentTile = level->getTile(x, y, z);

	if (currentTile == halfTile->id && ConcreteColorTileEntity::getColorAt(level, x, y, z) == color)
	{
		return true;
	}

	return TileItem::mayPlace(level, ox, oy, oz, face, player, item);
}

bool ConcreteSlabTileItem::tryConvertTargetTile(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, bool bTestUseOnOnly)
{
	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	int color = instance->getAuxValue() & 15;
	int currentTile = level->getTile(x, y, z);

	if (currentTile == halfTile->id && ConcreteColorTileEntity::getColorAt(level, x, y, z) == color)
	{
		if (bTestUseOnOnly)
		{
			return true;
		}

		convertToFull(instance, level, x, y, z, color);
		return true;
	}

	return false;
}
