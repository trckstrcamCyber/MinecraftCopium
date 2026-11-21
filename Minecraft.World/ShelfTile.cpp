// neoLegacy - Copper Age (Phase 6): shelf storage + redstone output.
// Phase 3 texture/aux behaviour preserved verbatim (woodIcons,
// getSpawnResourcesAuxValue = data & 7).
// Round-5 fix (2026-11-20): the facing lives in the ShelfTileEntity,
// NOT in data bits 3-5 — block data is a 4-bit nibble, so the Phase 6
// "(facing << 3)" write could never be read back and every shelf
// rendered with the NORTH shape. See updateShape()/setPlacedBy().
#include "stdafx.h"
#include "ShelfTile.h"
#include "IconRegister.h"
#include "ShelfTileEntity.h"
#include "SoundTypes.h"
#include "AbstractContainerMenu.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
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
	// Round-5 fix: the facing lives in the ShelfTileEntity, NOT in
	// the block data. Block data is a 4-bit nibble
	// (SparseDataStorage::set masks every write with "& 15", and
	// get() masks every read the same way), so the Phase 6 encoding
	// "(data >> 3) & 7" could never yield Facing::SOUTH (3),
	// WEST (4) or EAST (5): setPlacedBy wrote (facing << 3), i.e.
	// 16/24/32/40, which reads back as 0 or 8, so (data >> 3) & 7
	// was 0 or 1 and the switch below always took the NORTH default
	// — every shelf sat against the wrong wall except a
	// north-facing one. The TE is the only store with room for it:
	// wood already takes data bits 0-2, and a facing needs 2 more.
	// Prefer the TE the caller already has (forceEntity); otherwise
	// look it up (LevelChunk::getTileEntity creates it lazily, and
	// returns nullptr while tile entities are disabled). No TE yet
	// means a freshly placed shelf whose setPlacedBy has not run, or
	// a pre-fix shelf — NORTH is the safe default either way.
	int facing = Facing::NORTH;
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(forceEntity);
	if (shelf == nullptr && level != nullptr)
	{
		shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	}
	if (shelf != nullptr)
	{
		facing = shelf->getFacing();
	}
	// Front faces that direction, body sits against the opposite
	// wall. 5/16 deep (model z 11-16), full width/height.
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

// ChestTile pattern: the base getTileAABB/getAABB use the thread-local
// shape, which is only the NORTH default unless updateShape() has run
// for this block — so refresh the facing-dependent shape first, or
// collision/selection would use the wrong side's slab.
AABB *ShelfTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getTileAABB(level, x, y, z);
}

AABB *ShelfTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getAABB(level, x, y, z);
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
	// ChestTile facing convention (front faces the placer).
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;
	int facing = Facing::NORTH;
	if (dir == 1) facing = Facing::EAST;
	if (dir == 2) facing = Facing::SOUTH;
	if (dir == 3) facing = Facing::WEST;
	// Round-5 fix: the facing goes in the TileEntity, NOT in the
	// block data. The wood aux stays in data bits 0-2 (the placement
	// data is already just the item aux); ORing (facing << 3) into
	// the 4-bit data nibble could never be read back — and on a
	// pre-clamp build it spilled bit 4 into the neighbour block's
	// nibble. The TE already exists here: Level::setTileAndData
	// (TileItem::useOn) creates it before setPlacedBy is called.
	level->setData(x, y, z, level->getData(x, y, z) & 7, Tile::UPDATE_ALL);
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf != nullptr)
	{
		shelf->setFacing(facing);
		shelf->setChanged();
	}
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
	// Round-5 fix: the facing comes from the ShelfTileEntity, not
	// from the block data (see updateShape() — the data nibble
	// cannot hold it, so the old "(data >> 3) & 7" read was 0 or 1
	// and an east/west-facing shelf picked its slot from clickX).
	int facing = shelf->getFacing();
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
