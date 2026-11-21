#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ShelfTileEntity.h"
#include "Facing.h"
#include "ItemInstance.h"
#include "net.minecraft.network.packet.h"
#include "TileEntityDataPacket.h"

ShelfTileEntity::ShelfTileEntity() : TileEntity()
{
	items = new ItemInstanceArray(SLOT_COUNT);
	facing = Facing::NORTH;
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

int ShelfTileEntity::getFacing()
{
	return facing;
}

void ShelfTileEntity::setFacing(int facing)
{
	// Only the four horizontal facings are valid; anything else would
	// leave updateShape() on its default branch, so reject it here.
	if (facing == Facing::NORTH || facing == Facing::SOUTH || facing == Facing::WEST || facing == Facing::EAST)
	{
		this->facing = facing;
	}
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
	// Round-5 fix: the facing lives in the TE (see the header). A
	// missing key (shelves placed before this fix) reads back as 0
	// via CompoundTag::getByte, which is not a horizontal facing, so
	// keep the NORTH default in that case.
	int loadedFacing = base->getByte(L"Facing");
	if (loadedFacing == Facing::NORTH || loadedFacing == Facing::SOUTH || loadedFacing == Facing::WEST || loadedFacing == Facing::EAST)
	{
		facing = loadedFacing;
	}
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
	base->putByte(L"Facing", static_cast<byte>(facing));
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

shared_ptr<Packet> ShelfTileEntity::getUpdatePacket()
{
	// Round-6 fix (2026-11-20): live-sync the shelf contents to the client.
	// Bed precedent — save() writes Facing + Items, so the client's load()
	// restores both and the renderer (which reads the TE every frame) shows
	// the stored items without a chunk rebuild.
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_SHELF, tag);
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
	result->facing = facing;
	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != nullptr)
		{
			result->items->data[i] = ItemInstance::clone(items->data[i]);
		}
	}
	return result;
}
