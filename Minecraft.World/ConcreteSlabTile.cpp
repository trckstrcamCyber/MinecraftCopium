#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "ConcreteColorTileEntity.h"
#include "ConcreteSlabTile.h"

// Global 0-15 order -- identical to DyePowderItem::BLACK..WHITE and to the
// concrete_/concrete_powder_ ADD_ICON rows already in PreStitchedTextureMap.
// See strings.h for the matching IDS_TILE_CONCRETE_SLAB_* block.
const unsigned int ConcreteSlabTile::SLAB_NAMES[16] = {
	IDS_TILE_CONCRETE_SLAB_BLACK,
	IDS_TILE_CONCRETE_SLAB_RED,
	IDS_TILE_CONCRETE_SLAB_GREEN,
	IDS_TILE_CONCRETE_SLAB_BROWN,
	IDS_TILE_CONCRETE_SLAB_BLUE,
	IDS_TILE_CONCRETE_SLAB_PURPLE,
	IDS_TILE_CONCRETE_SLAB_CYAN,
	IDS_TILE_CONCRETE_SLAB_SILVER,
	IDS_TILE_CONCRETE_SLAB_GRAY,
	IDS_TILE_CONCRETE_SLAB_PINK,
	IDS_TILE_CONCRETE_SLAB_LIME,
	IDS_TILE_CONCRETE_SLAB_YELLOW,
	IDS_TILE_CONCRETE_SLAB_LIGHT_BLUE,
	IDS_TILE_CONCRETE_SLAB_MAGENTA,
	IDS_TILE_CONCRETE_SLAB_ORANGE,
	IDS_TILE_CONCRETE_SLAB_WHITE,
};

ConcreteSlabTile::ConcreteSlabTile(int id, int resourceTileId)
	: HalfSlabTile(id, Material::stone)
{
	this->resourceTileId = resourceTileId;
	this->m_dropColor = 0;
	_isEntityTile = true;	// makes LevelChunk::getTileEntity create the color tile entity on demand
}

shared_ptr<TileEntity> ConcreteSlabTile::newTileEntity(Level *level)
{
	return std::make_shared<ConcreteColorTileEntity>();
}

Icon *ConcreteSlabTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	// ConcreteTile returns the same single icon for every face of a given color, so
	// there's no top/side split to worry about -- just look up the color.
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	if (color < 0) color = 0;
	return Tile::concrete->getTexture(face, color);
}

Icon *ConcreteSlabTile::getTexture(int face, int data)
{
	return Tile::concrete->getTexture(face, data & 15);
}

void ConcreteSlabTile::registerIcons(IconRegister *iconRegister)
{
	// None -- entirely delegated to Tile::concrete's already-registered icons.
}

int ConcreteSlabTile::getResource(int data, Random *random, int playerBonusLevel)
{
	// Breaking either the half or the full slab drops the half-slab item.
	return resourceTileId;
}

unsigned int ConcreteSlabTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	return ConcreteSlabTile::SLAB_NAMES[iData & 15];
}

int ConcreteSlabTile::getAuxName(int auxValue)
{
	return SLAB_NAMES[auxValue & 15];
}

void ConcreteSlabTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	HalfSlabTile::onRemove(level, x, y, z, id, data);
	level->removeTileEntity(x, y, z);
}

void ConcreteSlabTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	HalfSlabTile::setPlacedBy(level, x, y, z, by, itemInstance);
	if (itemInstance != nullptr)
	{
		ConcreteColorTileEntity::setColorAt(level, x, y, z, itemInstance->getAuxValue() & 15);
	}
}

void ConcreteSlabTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	m_dropColor = (color >= 0) ? color : 0;
	HalfSlabTile::playerWillDestroy(level, x, y, z, data, player);
}

void ConcreteSlabTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (level->isClientSide) return;

	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	if (color < 0) color = m_dropColor;

	// getResourceCount() is 1 for a half slab and 2 for a full slab (HalfSlabTile)
	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(resourceTileId, 1, color));
	}
}

int ConcreteSlabTile::cloneTileData(Level *level, int x, int y, int z)
{
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	return (color >= 0) ? color : 0;
}
