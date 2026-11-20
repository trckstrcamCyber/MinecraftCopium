#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ShelfTileEntity.h"
#include "ItemInstance.h"

ShelfTileEntity::ShelfTileEntity() : TileEntity()
{
	items = new ItemInstanceArray(SLOT_COUNT);
}

ShelfTileEntity::~ShelfTileEntity()
{
	delete[] items->data;
	delete items;
}

bool ShelfTileEntity::isEmpty()
{
	for (unsigned int i = 0; i < SLOT_COUNT; i++)
	{
		if (items->data[i] != nullptr) return false;
	}
	return true;
}

unsigned int ShelfTileEntity::getContainerSize()
{
	return SLOT_COUNT;
}

shared_ptr<ItemInstance> ShelfTileEntity::getItem(unsigned int slot)
{
	if (slot >= SLOT_COUNT) return nullptr;
	return items->data[slot];
}

shared_ptr<ItemInstance> ShelfTileEntity::removeItem(unsigned int slot, int count)
{
	if (slot >= SLOT_COUNT || items->data[slot] == nullptr) return nullptr;
	if (items->data[slot]->count <= count)
	{
		shared_ptr<ItemInstance> item = items->data[slot];
		items->data[slot] = nullptr;
		setChanged();
		if (item->count <= 0) return nullptr;
		return item;
	}
	shared_ptr<ItemInstance> i = items->data[slot]->remove(count);
	if (items->data[slot]->count == 0) items->data[slot] = nullptr;
	setChanged();
	if (i->count <= 0) return nullptr;
	return i;
}

shared_ptr<ItemInstance> ShelfTileEntity::removeItemNoUpdate(int slot)
{
	if (slot >= SLOT_COUNT) return nullptr;
	shared_ptr<ItemInstance> item = items->data[slot];
	items->data[slot] = nullptr;
	return item;
}

void ShelfTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	if (slot >= SLOT_COUNT) return;
	items->data[slot] = item;
	if (item != nullptr && item->count > getMaxStackSize()) item->count = getMaxStackSize();
	this->setChanged();
}

wstring ShelfTileEntity::getName()
{
	return app.GetString(IDS_TILE_SHELF);
}

wstring ShelfTileEntity::getCustomName()
{
	return L"";
}

bool ShelfTileEntity::hasCustomName()
{
	return false;
}

void ShelfTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	if (items)
	{
		delete [] items->data;
		delete items;
	}
	items = new ItemInstanceArray(getContainerSize());
	if (inventoryList == nullptr) return;
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot") & 0xff;
		if (slot < items->length)
		{
			(*items)[slot] = ItemInstance::fromTag(tag);
		}
	}
}

void ShelfTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>;
	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != nullptr)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", static_cast<byte>(i));
			items->data[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
}

int ShelfTileEntity::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

bool ShelfTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this()) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void ShelfTileEntity::setChanged()
{
	TileEntity::setChanged();
}

void ShelfTileEntity::startOpen()
{
}

void ShelfTileEntity::stopOpen()
{
}

bool ShelfTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

// 4J Added
shared_ptr<TileEntity> ShelfTileEntity::clone()
{
	shared_ptr<ShelfTileEntity> result = std::make_shared<ShelfTileEntity>();
	TileEntity::clone(result);
	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != nullptr)
		{
			result->items->data[i] = ItemInstance::clone(items->data[i]);
		}
	}
	return result;
}
