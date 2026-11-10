#pragma once
using namespace std;

#include "TileEntity.h"
#include "WorldlyContainer.h"

// neoLegacy - shared brain of the Crusher and the Refiner (they are the same machine with a
// different recipe table; which one this is comes from the tile at its position, so there is
// only one tile entity class and nothing extra to save).
//
// Two slots:  0 = input, 1 = output.
//   * A hopper pointing DOWN into the TOP of the block fills the input slot. Nothing else can
//     insert: hoppers from the sides are refused, and only items the machine can actually
//     process are accepted.
//   * Every tick it works on the input slot. Each item takes a fixed number of ticks, then one
//     item is consumed and the result (which may be nothing) lands in the output slot.
//   * The output comes out of the BOTTOM: the machine pushes it into a hopper / chest / another
//     machine directly below, and a hopper below can also pull it. If there is no container
//     below and the space is open, it simply drops the item there.
// The machine only starts its next item once the output slot is empty.
class ProcessorTileEntity : public TileEntity, public WorldlyContainer
{
public:
	eINSTANCEOF GetType() { return eTYPE_PROCESSORTILEENTITY; }
	static TileEntity *create() { return new ProcessorTileEntity(); }

	static const int SLOT_INPUT = 0;
	static const int SLOT_OUTPUT = 1;
	static const int SLOT_COUNT = 2;

private:
	ItemInstanceArray items;
	int progress;		// ticks spent on the current input item

	static const intArray SLOTS_INPUT;
	static const intArray SLOTS_OUTPUT;
	static const intArray SLOTS_NONE;

	bool isRefiner();
	int getProcessTime();
	bool tryOutput();
	shared_ptr<ItemInstance> processOne(int inputId, int inputAux);

public:
	ProcessorTileEntity();
	virtual ~ProcessorTileEntity();

	// What each machine accepts (used by the tile entity and by nothing else).
	static bool accepts(bool refiner, shared_ptr<ItemInstance> item);

	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual void tick();
	virtual shared_ptr<TileEntity> clone();

	// Container
	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);

	// WorldlyContainer
	virtual intArray getSlotsForFace(int face);
	virtual bool canPlaceItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);
	virtual bool canTakeItemThroughFace(int slot, shared_ptr<ItemInstance> item, int face);
};
