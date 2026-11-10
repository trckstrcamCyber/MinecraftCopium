#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "DyePowderItem.h"
#include "WoolTileItem.h"

const unsigned int WoolTileItem::COLOR_DESCS[] = 
{ 
	IDS_TILE_CLOTH_BLACK,
	IDS_TILE_CLOTH_RED,
	IDS_TILE_CLOTH_GREEN,
	IDS_TILE_CLOTH_BROWN,
	IDS_TILE_CLOTH_BLUE,
	IDS_TILE_CLOTH_PURPLE,
	IDS_TILE_CLOTH_CYAN,
	IDS_TILE_CLOTH_SILVER,
	IDS_TILE_CLOTH_GRAY,
	IDS_TILE_CLOTH_PINK,
	IDS_TILE_CLOTH_LIME,
	IDS_TILE_CLOTH_YELLOW,
	IDS_TILE_CLOTH_LIGHT_BLUE,
	IDS_TILE_CLOTH_MAGENTA,
	IDS_TILE_CLOTH_ORANGE,
	IDS_TILE_CLOTH_WHITE
};

const unsigned int WoolTileItem::CARPET_COLOR_DESCS[] = 
{ 
	IDS_TILE_CARPET_BLACK,
	IDS_TILE_CARPET_RED,
	IDS_TILE_CARPET_GREEN,
	IDS_TILE_CARPET_BROWN,
	IDS_TILE_CARPET_BLUE,
	IDS_TILE_CARPET_PURPLE,
	IDS_TILE_CARPET_CYAN,
	IDS_TILE_CARPET_SILVER,
	IDS_TILE_CARPET_GRAY,
	IDS_TILE_CARPET_PINK,
	IDS_TILE_CARPET_LIME,
	IDS_TILE_CARPET_YELLOW,
	IDS_TILE_CARPET_LIGHT_BLUE,
	IDS_TILE_CARPET_MAGENTA,
	IDS_TILE_CARPET_ORANGE,
	IDS_TILE_CARPET_WHITE
};

const unsigned int WoolTileItem::CLAY_COLOR_DESCS[] = 
{ 
	IDS_TILE_STAINED_CLAY_BLACK,
	IDS_TILE_STAINED_CLAY_RED,
	IDS_TILE_STAINED_CLAY_GREEN,
	IDS_TILE_STAINED_CLAY_BROWN,
	IDS_TILE_STAINED_CLAY_BLUE,
	IDS_TILE_STAINED_CLAY_PURPLE,
	IDS_TILE_STAINED_CLAY_CYAN,
	IDS_TILE_STAINED_CLAY_SILVER,
	IDS_TILE_STAINED_CLAY_GRAY,
	IDS_TILE_STAINED_CLAY_PINK,
	IDS_TILE_STAINED_CLAY_LIME,
	IDS_TILE_STAINED_CLAY_YELLOW,
	IDS_TILE_STAINED_CLAY_LIGHT_BLUE,
	IDS_TILE_STAINED_CLAY_MAGENTA,
	IDS_TILE_STAINED_CLAY_ORANGE,
	IDS_TILE_STAINED_CLAY_WHITE
};

const unsigned int WoolTileItem::GLASS_COLOR_DESCS[] = 
{ 
	IDS_TILE_STAINED_GLASS_BLACK,
	IDS_TILE_STAINED_GLASS_RED,
	IDS_TILE_STAINED_GLASS_GREEN,
	IDS_TILE_STAINED_GLASS_BROWN,
	IDS_TILE_STAINED_GLASS_BLUE,
	IDS_TILE_STAINED_GLASS_PURPLE,
	IDS_TILE_STAINED_GLASS_CYAN,
	IDS_TILE_STAINED_GLASS_SILVER,
	IDS_TILE_STAINED_GLASS_GRAY,
	IDS_TILE_STAINED_GLASS_PINK,
	IDS_TILE_STAINED_GLASS_LIME,
	IDS_TILE_STAINED_GLASS_YELLOW,
	IDS_TILE_STAINED_GLASS_LIGHT_BLUE,
	IDS_TILE_STAINED_GLASS_MAGENTA,
	IDS_TILE_STAINED_GLASS_ORANGE,
	IDS_TILE_STAINED_GLASS_WHITE
};

const unsigned int WoolTileItem::GLASS_PANE_COLOR_DESCS[] = 
{ 
	IDS_TILE_STAINED_GLASS_PANE_BLACK,
	IDS_TILE_STAINED_GLASS_PANE_RED,
	IDS_TILE_STAINED_GLASS_PANE_GREEN,
	IDS_TILE_STAINED_GLASS_PANE_BROWN,
	IDS_TILE_STAINED_GLASS_PANE_BLUE,
	IDS_TILE_STAINED_GLASS_PANE_PURPLE,
	IDS_TILE_STAINED_GLASS_PANE_CYAN,
	IDS_TILE_STAINED_GLASS_PANE_SILVER,
	IDS_TILE_STAINED_GLASS_PANE_GRAY,
	IDS_TILE_STAINED_GLASS_PANE_PINK,
	IDS_TILE_STAINED_GLASS_PANE_LIME,
	IDS_TILE_STAINED_GLASS_PANE_YELLOW,
	IDS_TILE_STAINED_GLASS_PANE_LIGHT_BLUE,
	IDS_TILE_STAINED_GLASS_PANE_MAGENTA,
	IDS_TILE_STAINED_GLASS_PANE_ORANGE,
	IDS_TILE_STAINED_GLASS_PANE_WHITE
};

const unsigned int WoolTileItem::CONCRETE_POWDER_COLOR_DESCS[] = 
{ 
	IDS_TILE_CONCRETE_POWDER_BLACK,
	IDS_TILE_CONCRETE_POWDER_RED,
	IDS_TILE_CONCRETE_POWDER_GREEN,
	IDS_TILE_CONCRETE_POWDER_BROWN,
	IDS_TILE_CONCRETE_POWDER_BLUE,
	IDS_TILE_CONCRETE_POWDER_PURPLE,
	IDS_TILE_CONCRETE_POWDER_CYAN,
	IDS_TILE_CONCRETE_POWDER_SILVER,
	IDS_TILE_CONCRETE_POWDER_GRAY,
	IDS_TILE_CONCRETE_POWDER_PINK,
	IDS_TILE_CONCRETE_POWDER_LIME,
	IDS_TILE_CONCRETE_POWDER_YELLOW,
	IDS_TILE_CONCRETE_POWDER_LIGHT_BLUE,
	IDS_TILE_CONCRETE_POWDER_MAGENTA,
	IDS_TILE_CONCRETE_POWDER_ORANGE,
	IDS_TILE_CONCRETE_POWDER_WHITE
};

const unsigned int WoolTileItem::CONCRETE_COLOR_DESCS[] = 
{ 
	IDS_TILE_CONCRETE_BLACK,
	IDS_TILE_CONCRETE_RED,
	IDS_TILE_CONCRETE_GREEN,
	IDS_TILE_CONCRETE_BROWN,
	IDS_TILE_CONCRETE_BLUE,
	IDS_TILE_CONCRETE_PURPLE,
	IDS_TILE_CONCRETE_CYAN,
	IDS_TILE_CONCRETE_SILVER,
	IDS_TILE_CONCRETE_GRAY,
	IDS_TILE_CONCRETE_PINK,
	IDS_TILE_CONCRETE_LIME,
	IDS_TILE_CONCRETE_YELLOW,
	IDS_TILE_CONCRETE_LIGHT_BLUE,
	IDS_TILE_CONCRETE_MAGENTA,
	IDS_TILE_CONCRETE_ORANGE,
	IDS_TILE_CONCRETE_WHITE
};

WoolTileItem::WoolTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *WoolTileItem::getIcon(int itemAuxValue) 
{
#ifndef _CONTENT_PACKAGE
	if(Tile::tiles[id])
	{
		if (id == Tile::stained_glass_Id || id == Tile::stained_glass_pane_Id)
		{
			return Tile::tiles[id]->getTexture(2, itemAuxValue);
		}

		if (id == Tile::concrete_powder_Id || id == Tile::concrete_Id)
		{
			// Concrete/ConcretePowderTile build their icons[] straight from
			// DyePowderItem::COLOR_TEXTURES[data] with NO inversion (unlike
			// ColoredTile, which wool/carpet/clay/glass use, and which DOES
			// invert via getTileDataForItemAuxValue). This branch used to
			// apply that inversion anyway, making it silently identical to
			// the generic fallback below -- that's the actual bug behind
			// "the item in my hand doesn't match the block I place": the
			// item's icon/name were computed with the wool-style flip, the
			// world block was not (see getLevelDataForAuxValue below, which
			// is a plain pass-through), so they disagreed for every color
			// except the two that are palindromic under the flip.
			return Tile::tiles[id]->getTexture(2, itemAuxValue % 16);
		}

		return Tile::tiles[id]->getTexture(2, ColoredTile::getTileDataForItemAuxValue(itemAuxValue));
	}
	else
#endif
	{
		return Tile::wool->getTexture(2, ColoredTile::getTileDataForItemAuxValue(itemAuxValue));
	}
}

int WoolTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

unsigned int WoolTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	int tileId = getTileId();
	switch(getTileId())
	{
	case Tile::stained_glass_Id:
		return GLASS_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::stained_glass_pane_Id:
		return GLASS_PANE_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::stained_hardened_clay_Id:
		return CLAY_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::carpet_Id:
		return CARPET_COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	case Tile::concrete_powder_Id:
		// Same reasoning as getIcon() above: raw aux value, not the
		// ColoredTile (wool-style) inversion.
		return CONCRETE_POWDER_COLOR_DESCS[instance->getAuxValue() % 16];
	case Tile::concrete_Id:
		return CONCRETE_COLOR_DESCS[instance->getAuxValue() % 16];
	case Tile::wool_Id:
	default:
		return COLOR_DESCS[ColoredTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	};
}
