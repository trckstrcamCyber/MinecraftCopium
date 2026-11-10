#include "stdafx.h"
#include "Random.h"
#include "ItemInstance.h"
#include "Player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.h"
#include "ConcreteColorTileEntity.h"
#include "ConcreteStairTile.h"

ConcreteStairTile::ConcreteStairTile(int id) : StairTile(id, Tile::concrete, 0)
{
	_isEntityTile = true;	// makes LevelChunk::getTileEntity create the color tile entity on demand
	m_dropColor = 0;
}

shared_ptr<TileEntity> ConcreteStairTile::newTileEntity(Level *level)
{
	return std::make_shared<ConcreteColorTileEntity>();
}

Icon *ConcreteStairTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	if (color < 0) color = 0;
	return Tile::concrete->getTexture(face, color);
}

Icon *ConcreteStairTile::getTexture(int face, int data)
{
	return Tile::concrete->getTexture(face, data & 15);
}

void ConcreteStairTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	StairTile::onRemove(level, x, y, z, id, data);
	level->removeTileEntity(x, y, z);
}

void ConcreteStairTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	StairTile::setPlacedBy(level, x, y, z, by, itemInstance);	// orientation
	if (itemInstance != nullptr)
	{
		ConcreteColorTileEntity::setColorAt(level, x, y, z, itemInstance->getAuxValue() & 15);
	}
}

void ConcreteStairTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	m_dropColor = (color >= 0) ? color : 0;
	StairTile::playerWillDestroy(level, x, y, z, data, player);
}

void ConcreteStairTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (level->isClientSide) return;

	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	if (color < 0) color = m_dropColor;

	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(id, 1, color));
	}
}

int ConcreteStairTile::cloneTileData(Level *level, int x, int y, int z)
{
	int color = ConcreteColorTileEntity::getColorAt(level, x, y, z);
	return (color >= 0) ? color : 0;
}
