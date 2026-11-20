// neoLegacy - Copper Age (Phase 6): shelf storage + redstone output.
// Phase 3 texture/aux behaviour preserved verbatim (woodIcons, facing
// now in data bits 3-5, getSpawnResourcesAuxValue = data & 7).
#include "stdafx.h"
#include "ShelfTile.h"
#include "IconRegister.h"
#include "ShelfTileEntity.h"
#include "SoundTypes.h"
#include "AbstractContainerMenu.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "ItemInstance.h"

ShelfTile::ShelfTile(int id) : BaseEntityTile(id, Material::wood, isSolidRender())
{
	for (int i = 0; i < 7; i++) woodIcons[i] = nullptr;
}

bool ShelfTile::isSolidRender(bool)
{
	return false;
}

bool ShelfTile::isCubeShaped()
{
	return false;
}

void ShelfTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	// Facing is data bits 3-5; front faces that direction, body sits
	// against the opposite wall. 5/16 deep (model z 11-16), full width/height.
	int facing = (level->getData(x, y, z) >> 3) & 7;
	switch (facing)
	{
	case Facing::SOUTH: setShape(0, 0, 0, 1, 1, 5.0f/16.0f); break;
	case Facing::WEST:  setShape(11.0f/16.0f, 0, 0, 1, 1, 1); break;
	case Facing::EAST:  setShape(0, 0, 0, 5.0f/16.0f, 1, 1); break;
	default:            setShape(0, 0, 11.0f/16.0f, 1, 1, 1); break; // NORTH
	}
}

void ShelfTile::updateDefaultShape()
{
	setShape(0, 0, 11.0f/16.0f, 1, 1, 1);   // NORTH default
}

Icon *ShelfTile::getTexture(int face, int data)
{
	int wood = data & 7;
	if (wood > 6) wood = 0;
	return woodIcons[wood];
}

Icon *ShelfTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return getTexture(face, level->getData(x, y, z));
}

void ShelfTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *names[7] = {
		L"oak_shelf", L"spruce_shelf", L"birch_shelf", L"jungle_shelf",
		L"acacia_shelf", L"dark_oak_shelf", L"bamboo_shelf"
	};
	for (int i = 0; i < 7; i++)
	{
		woodIcons[i] = iconRegister->registerIcon(names[i]);
	}
	icon = woodIcons[0];
}

shared_ptr<TileEntity> ShelfTile::newTileEntity(Level *level)
{
	MemSect(50);
	shared_ptr<TileEntity> retval = std::make_shared<ShelfTileEntity>();
	MemSect(0);
	return retval;
}

void ShelfTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	// ChestTile facing convention (front faces the placer). The wood
	// aux lives in data bits 0-2; the facing goes in bits 3-5.
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;
	int facing = Facing::NORTH;
	if (dir == 1) facing = Facing::EAST;
	if (dir == 2) facing = Facing::SOUTH;
	if (dir == 3) facing = Facing::WEST;
	level->setData(x, y, z, (level->getData(x, y, z) & 7) | (facing << 3), Tile::UPDATE_ALL);
}

void ShelfTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	// ChestTile pattern, without the scatter: drop each stored stack
	// whole at the block centre so nothing is lost with the shelf.
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf != nullptr)
	{
		for (unsigned int i = 0; i < shelf->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = shelf->getItem(i);
			if (item != nullptr)
			{
				shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + 0.5, y + 0.5, z + 0.5, item);
				level->addEntity(itemEntity);
				shelf->setItem(i, nullptr);
			}
		}
		level->updateNeighbourForOutputSignal(x, y, z, id);
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

bool ShelfTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (soundOnly) return true;
	if (level->isClientSide) return true;
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf == nullptr) return false;

	// Slot = thirds along the shelf front face: clickX for a
	// north/south-facing shelf, clickZ for east/west-facing.
	int facing = (level->getData(x, y, z) >> 3) & 7;
	float along = (facing == Facing::EAST || facing == Facing::WEST) ? clickZ : clickX;
	int slot = static_cast<int>(along * 3.0f);
	if (slot < 0) slot = 0;
	if (slot > 2) slot = 2;

	shared_ptr<ItemInstance> hand = player->inventory->getSelected();
	bool wasEmpty = shelf->isEmpty();
	if (hand == nullptr)
	{
		// Empty hand: take the item in the clicked slot.
		shared_ptr<ItemInstance> stored = shelf->getItem(slot);
		if (stored == nullptr) return false;
		shelf->removeItemNoUpdate(slot);
		shelf->setChanged();
		if (!player->inventory->add(stored))
		{
			// Inventory full: put it back rather than lose it.
			shelf->setItem(slot, stored);
			return false;
		}
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_TAKE_ITEM, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		if (shelf->isEmpty()) level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_DEACTIVATE, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		return true;
	}

	shared_ptr<ItemInstance> stored = shelf->getItem(slot);
	if (stored == nullptr)
	{
		// Slot empty: place one item from the hand.
		shared_ptr<ItemInstance> placed = std::make_shared<ItemInstance>(hand->id, 1, hand->getAuxValue());
		if (hand->hasTag()) placed->setTag(static_cast<CompoundTag *>(hand->getTag()->copy()));
		shelf->setItem(slot, placed);
		player->inventory->removeItem(player->inventory->selected, 1);
		shelf->setChanged();
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_PLACE_ITEM, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		if (wasEmpty) level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_ACTIVATE, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		return true;
	}

	// Slot occupied: swap one hand item for the stored item.
	shared_ptr<ItemInstance> placed = std::make_shared<ItemInstance>(hand->id, 1, hand->getAuxValue());
	if (hand->hasTag()) placed->setTag(static_cast<CompoundTag *>(hand->getTag()->copy()));
	shelf->setItem(slot, placed);
	player->inventory->removeItem(player->inventory->selected, 1);
	shelf->setChanged();
	if (!player->inventory->add(stored))
	{
		// Inventory full (hand stack was the only free space): drop the
		// swapped-out item at the player instead of losing it.
		shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, player->x, player->y, player->z, stored);
		level->addEntity(itemEntity);
	}
	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_SINGLE_SWAP, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
	return true;
}

bool ShelfTile::hasAnalogOutputSignal()
{
	return true;
}

int ShelfTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	// ChestTile precedent: container-fullness signal (0-15). The
	// Java shelf emits by filled slots instead; the difference is
	// recorded in the Phase 6 report as a playtest item.
	shared_ptr<Container> container = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (container == nullptr) return 0;
	return AbstractContainerMenu::getRedstoneSignalFromContainer(container);
}

int ShelfTile::getSpawnResourcesAuxValue(int data)
{
	return data & 7;
}
