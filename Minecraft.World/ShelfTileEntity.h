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
class ShelfTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_SHELFTILEENTITY; }
	static TileEntity *create() { return new ShelfTileEntity(); }

	static const unsigned int SLOT_COUNT = 3;

	using TileEntity::setChanged;

private:
	ItemInstanceArray *items;

public:
	ShelfTileEntity();
	virtual ~ShelfTileEntity();

	bool isEmpty();

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
	virtual int getMaxStackSize() const override;
	virtual bool stillValid(shared_ptr<Player> player) override;
	virtual void setChanged() override;
	virtual void startOpen() override;
	virtual void stopOpen() override;
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item) override;

	// 4J Added
	virtual shared_ptr<TileEntity> clone() override;
};
