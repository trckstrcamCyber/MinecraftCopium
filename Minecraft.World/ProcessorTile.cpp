#include "stdafx.h"
#include "Random.h"
#include "ItemInstance.h"
#include "Material.h"
#include "ItemEntity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "ProcessorTile.h"
#include "ProcessorTileEntity.h"
#include "Facing.h"
#include "IconRegister.h"

ProcessorTile::ProcessorTile(int id, bool isRefiner)
	: BaseEntityTile(id, Material::stone, true)
{
	refiner = isRefiner;
	topIcon = nullptr;
	bottomIcon = nullptr;
	sideIcon = nullptr;

	setDestroyTime(3.5f);
	setExplodeable(6.0f);
	setSoundType(Tile::SOUND_STONE);
}

Icon *ProcessorTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return topIcon;
	if (face == Facing::DOWN) return bottomIcon;
	return sideIcon;
}

void ProcessorTile::registerIcons(IconRegister *iconRegister)
{
	topIcon = iconRegister->registerIcon(L"crusher_top");
	bottomIcon = iconRegister->registerIcon(L"crusher_bottom");
	sideIcon = iconRegister->registerIcon(L"observer_side");

	icon = sideIcon;
}

int ProcessorTile::getColor(LevelSource *level, int x, int y, int z)
{
	return refiner ? REFINER_TINT : 0xFFFFFF;
}

int ProcessorTile::getColor(int auxData)
{
	return refiner ? REFINER_TINT : 0xFFFFFF;
}

void ProcessorTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	shared_ptr<Container> container = dynamic_pointer_cast<ProcessorTileEntity>(level->getTileEntity(x, y, z));
	if (container != nullptr)
	{
		for (unsigned int i = 0; i < container->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = container->getItem(i);
			if (item == nullptr) continue;

			float xo = level->random->nextFloat() * 0.8f + 0.1f;
			float yo = level->random->nextFloat() * 0.8f + 0.1f;
			float zo = level->random->nextFloat() * 0.8f + 0.1f;

			shared_ptr<ItemInstance> newItem = std::make_shared<ItemInstance>(item->id, item->count, item->getAuxValue());
			shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + xo, y + yo, z + zo, newItem);
			float pow = 0.05f;
			itemEntity->xd = static_cast<float>(level->random->nextGaussian()) * pow;
			itemEntity->yd = static_cast<float>(level->random->nextGaussian()) * pow + 0.2f;
			itemEntity->zd = static_cast<float>(level->random->nextGaussian()) * pow;
			level->addEntity(itemEntity);

			// Fix for duplication: empty the slot so nothing is dropped twice.
			container->setItem(i, nullptr);
		}
	}

	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

shared_ptr<TileEntity> ProcessorTile::newTileEntity(Level *level)
{
	return std::make_shared<ProcessorTileEntity>();
}
