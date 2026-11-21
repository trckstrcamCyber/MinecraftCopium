#pragma once

using namespace std;

#include "TileEntity.h"
#include "Container.h"

#include "ListTag.h"

class Player;
class CompoundTag;

// neoLegacy - Copper Age (Phase 6). The copper shelf's storage: 3
// slots (Java shelf). ChestTileEntity pattern, without neighbours,
// open-count or a container UI — ShelfTile::use() swaps items with
// the player directly, and ShelfTile::getAnalogOutputSignal reads
// the fullness. No custom renderer is staged in Phase 6 (see
// ShelfTile.h), so stored items are not yet visible in-world.
//
// Round-5 fix (2026-11-20): the shelf's facing lives HERE, not in
// the block data. Phase 6 stored it in data bits 3-5, but block data
// is a 4-bit nibble (SparseDataStorage::set masks with "& 15"), so a
// Facing value (2-5) shifted left 3 is 16-40 and can never be read
// back — every shelf rendered with the NORTH shape. The TE persists
// the facing in its NBT instead (see load/save, key "Facing").
class ShelfTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_SHELFTILEENTITY; }
	static TileEntity *create() { return new ShelfTileEntity(); }

	static const unsigned int SLOT_COUNT = 3;

	using TileEntity::setChanged;

private:
	ItemInstanceArray *items;
	int facing;

public:
	ShelfTileEntity();
	virtual ~ShelfTileEntity();

	bool isEmpty();

	// Facing the shelf front points at (Facing::NORTH/SOUTH/WEST/EAST).
	int getFacing();
	void setFacing(int facing);

	virtual unsigned int getContainerSize() override;
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot) override;
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count) override;
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot) override;
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item) override;
	virtual wstring getName() override;
	virtual wstring getCustomName() override;
	virtual bool hasCustomName() override;
	virtual void load(CompoundTag *base) override;
	virtual void save(CompoundTag *base) override;
	// Round-6 fix (2026-11-20): live-sync the shelf contents to the client.
	// Without this the client's ShelfTileEntity never receives the stored
	// items (base TileEntity::getUpdatePacket returns nullptr), so the
	// renderer sees an empty shelf. Bed precedent.
	virtual shared_ptr<Packet> getUpdatePacket() override;
	virtual int getMaxStackSize() const override;
	virtual bool stillValid(shared_ptr<Player> player) override;
	virtual void setChanged() override;
	virtual void startOpen() override;
	virtual void stopOpen() override;
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item) override;

	// 4J Added
	virtual shared_ptr<TileEntity> clone() override;
};
