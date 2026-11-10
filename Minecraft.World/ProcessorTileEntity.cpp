#include "stdafx.h"
#include "Random.h"
#include "ItemInstance.h"
#include "Player.h"
#include "ItemEntity.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "com.mojang.nbt.h"
#include "Facing.h"
#include "DyePowderItem.h"
#include "HopperTileEntity.h"
#include "ProcessorTileEntity.h"

// ---------------------------------------------------------------------------------------------
// Tuning -- everything a designer might want to tweak lives here.
// Chances are out of 100; a roll that lands past every listed outcome gives NOTHING.
// ---------------------------------------------------------------------------------------------
namespace
{
	// How long one item takes, in game ticks (20 ticks = 1 second).
	const int CRUSHER_TICKS = 40;	// 2 seconds per item
	const int REFINER_TICKS = 60;	// 3 seconds per item

	// Crusher: stone -> gravel is guaranteed; gravel -> sand can fail.
	const int CRUSHER_GRAVEL_TO_SAND = 70;			// 70% sand, 30% nothing

	// Refiner, gravel:
	const int REFINER_GRAVEL_GOLD_NUGGET = 50;		// 50% gold nugget
	const int REFINER_GRAVEL_IRON_INGOT  = 5;		//  5% iron ingot (rare)
															// remaining 45% = nothing
	// Refiner, sand:
	const int REFINER_SAND_REDSTONE = 40;			// 40% redstone dust
	const int REFINER_SAND_LAPIS    = 15;			// 15% lapis lazuli
															// remaining 45% = nothing
}

static int processorSlotsInput[]  = { ProcessorTileEntity::SLOT_INPUT };
static int processorSlotsOutput[] = { ProcessorTileEntity::SLOT_OUTPUT };
static int processorSlotsNone[]   = { 0 };

const intArray ProcessorTileEntity::SLOTS_INPUT  = intArray(processorSlotsInput, 1);
const intArray ProcessorTileEntity::SLOTS_OUTPUT = intArray(processorSlotsOutput, 1);
const intArray ProcessorTileEntity::SLOTS_NONE   = intArray(processorSlotsNone, 0);

ProcessorTileEntity::ProcessorTileEntity() : TileEntity()
{
	items = ItemInstanceArray(SLOT_COUNT);
	progress = 0;
}

ProcessorTileEntity::~ProcessorTileEntity()
{
	delete[] items.data;
}

bool ProcessorTileEntity::isRefiner()
{
	return level != nullptr && level->getTile(x, y, z) == Tile::refiner_Id;
}

int ProcessorTileEntity::getProcessTime()
{
	return isRefiner() ? REFINER_TICKS : CRUSHER_TICKS;
}

bool ProcessorTileEntity::accepts(bool refiner, shared_ptr<ItemInstance> item)
{
	if (item == nullptr) return false;

	if (refiner)
	{
		return item->id == Tile::gravel_Id || item->id == Tile::sand_Id;
	}

	// Crusher. Only plain stone (aux 0), not cobblestone -- change here to accept more.
	if (item->id == Tile::stone_Id) return item->getAuxValue() == 0;
	return item->id == Tile::gravel_Id;
}

// Rolls the result of processing ONE input item. Returns nullptr for "nothing".
shared_ptr<ItemInstance> ProcessorTileEntity::processOne(int inputId, int inputAux)
{
	int roll = level->random->nextInt(100);

	if (isRefiner())
	{
		if (inputId == Tile::gravel_Id)
		{
			if (roll < REFINER_GRAVEL_GOLD_NUGGET) return std::make_shared<ItemInstance>(Item::gold_nugget, 1);
			if (roll < REFINER_GRAVEL_GOLD_NUGGET + REFINER_GRAVEL_IRON_INGOT) return std::make_shared<ItemInstance>(Item::iron_ingot, 1);
			return nullptr;
		}
		if (inputId == Tile::sand_Id)
		{
			if (roll < REFINER_SAND_REDSTONE) return std::make_shared<ItemInstance>(Item::redstone, 1);
			if (roll < REFINER_SAND_REDSTONE + REFINER_SAND_LAPIS) return std::make_shared<ItemInstance>(Item::dye, 1, DyePowderItem::BLUE);	// lapis lazuli
			return nullptr;
		}
		return nullptr;
	}

	// Crusher
	if (inputId == Tile::stone_Id) return std::make_shared<ItemInstance>(Tile::gravel, 1);
	if (inputId == Tile::gravel_Id)
	{
		if (roll < CRUSHER_GRAVEL_TO_SAND) return std::make_shared<ItemInstance>(Tile::sand, 1);
		return nullptr;
	}
	return nullptr;
}

void ProcessorTileEntity::tick()
{
	TileEntity::tick();

	if (level == nullptr || level->isClientSide) return;

	// Get finished product out of the bottom first.
	if (items[SLOT_OUTPUT] != nullptr)
	{
		tryOutput();
	}

	shared_ptr<ItemInstance> input = items[SLOT_INPUT];

	// Nothing (valid) to work on, or the last product hasn't left yet: wait, and don't
	// carry progress over to a different item.
	if (input == nullptr || !accepts(isRefiner(), input) || items[SLOT_OUTPUT] != nullptr)
	{
		progress = 0;
		return;
	}

	progress++;
	if (progress < getProcessTime()) return;

	progress = 0;

	int inputId = input->id;
	int inputAux = input->getAuxValue();
	shared_ptr<ItemInstance> result = processOne(inputId, inputAux);

	// One input item is used up whether or not anything comes out.
	input->count--;
	if (input->count <= 0) items[SLOT_INPUT] = nullptr;

	if (result != nullptr)
	{
		items[SLOT_OUTPUT] = result;
		tryOutput();
	}

	setChanged();
}

// Pushes the output slot out of the bottom of the block. Returns true if anything moved.
bool ProcessorTileEntity::tryOutput()
{
	shared_ptr<ItemInstance> out = items[SLOT_OUTPUT];
	if (out == nullptr) return false;

	// 1) A container directly below (hopper, chest, another Crusher/Refiner...). Items enter it
	//    through ITS top face.
	shared_ptr<Container> below = HopperTileEntity::getContainerAt(level, x, y - 1, z);
	if (below != nullptr)
	{
		shared_ptr<ItemInstance> remainder = HopperTileEntity::addItem(below.get(), out->copy(), Facing::UP);
		if (remainder == nullptr || remainder->count == 0)
		{
			items[SLOT_OUTPUT] = nullptr;
			below->setChanged();
			setChanged();
			return true;
		}
		if (remainder->count != out->count)
		{
			items[SLOT_OUTPUT] = remainder;
			below->setChanged();
			setChanged();
			return true;
		}
		return false;
	}

	// 2) Open space below: just let it fall out. (A solid block below with no container means the
	//    output stays put and the machine waits -- same as a furnace with a full output.)
	if (!level->isSolidBlockingTile(x, y - 1, z))
	{
		shared_ptr<ItemEntity> drop = std::make_shared<ItemEntity>(level, x + 0.5, y - 0.3, z + 0.5, out->copy());
		drop->xd = 0;
		drop->yd = 0;
		drop->zd = 0;
		drop->throwTime = 10;
		level->addEntity(drop);

		items[SLOT_OUTPUT] = nullptr;
		setChanged();
		return true;
	}

	return false;
}

void ProcessorTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);

	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	delete[] items.data;
	items = ItemInstanceArray(SLOT_COUNT);
	for (int i = 0; i < inventoryList->size(); i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot");
		if (slot >= 0 && slot < items.length) items[slot] = ItemInstance::fromTag(tag);
	}

	progress = base->getShort(L"Progress");
}

void ProcessorTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);

	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != nullptr)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", static_cast<byte>(i));
			items[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
	base->putShort(L"Progress", static_cast<short>(progress));
}

shared_ptr<TileEntity> ProcessorTileEntity::clone()
{
	shared_ptr<ProcessorTileEntity> result = std::make_shared<ProcessorTileEntity>();
	TileEntity::clone(result);
	for (unsigned int i = 0; i < items.length; i++)
	{
		if (items[i] != nullptr) result->items[i] = items[i]->copy();
	}
	result->progress = progress;
	return result;
}

// ---- Container -------------------------------------------------------------------------------

unsigned int ProcessorTileEntity::getContainerSize()
{
	return items.length;
}

shared_ptr<ItemInstance> ProcessorTileEntity::getItem(unsigned int slot)
{
	return items[slot];
}

shared_ptr<ItemInstance> ProcessorTileEntity::removeItem(unsigned int slot, int count)
{
	if (items[slot] != nullptr)
	{
		if (items[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = items[slot];
			items[slot] = nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = items[slot]->remove(count);
			if (items[slot]->count == 0) items[slot] = nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> ProcessorTileEntity::removeItemNoUpdate(int slot)
{
	if (items[slot] != nullptr)
	{
		shared_ptr<ItemInstance> item = items[slot];
		items[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void ProcessorTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items[slot] = item;
	if (item != nullptr && item->count > getMaxStackSize()) item->count = getMaxStackSize();
}

wstring ProcessorTileEntity::getName()
{
	return L"container.processor";
}

wstring ProcessorTileEntity::getCustomName()
{
	return L"";
}

bool ProcessorTileEntity::hasCustomName()
{
	return false;
}

int ProcessorTileEntity::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void ProcessorTileEntity::setChanged()
{
	TileEntity::setChanged();
}

bool ProcessorTileEntity::stillValid(shared_ptr<Player> player)
{
	// No GUI -- players never open this block.
	return false;
}

void ProcessorTileEntity::startOpen()
{
}

void ProcessorTileEntity::stopOpen()
{
}

bool ProcessorTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return slot == SLOT_INPUT && accepts(isRefiner(), item);
}

// ---- WorldlyContainer ------------------------------------------------------------------------

intArray ProcessorTileEntity::getSlotsForFace(int face)
{
	if (face == Facing::UP) return SLOTS_INPUT;
	if (face == Facing::DOWN) return SLOTS_OUTPUT;
	return SLOTS_NONE;
}

bool ProcessorTileEntity::canPlaceItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face)
{
	// Only through the top, only into the input slot.
	return slot == SLOT_INPUT && face == Facing::UP;
}

bool ProcessorTileEntity::canTakeItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face)
{
	// Only the output, only out of the bottom.
	return slot == SLOT_OUTPUT && face == Facing::DOWN;
}
