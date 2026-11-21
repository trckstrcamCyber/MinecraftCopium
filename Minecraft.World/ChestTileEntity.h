#pragma once

using namespace std;

#include "TileEntity.h"
#include "Container.h"

#include "ListTag.h"

class Player;
class CompoundTag;

class ChestTileEntity : public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_CHESTTILEENTITY; }
	static TileEntity *create() { return new ChestTileEntity(); }

	int getContainerType(); // 4J-Added;

	using TileEntity::setChanged;

private:
	void _init(bool isBonusChest);

public:
	ChestTileEntity(bool isBonusChest = false);		// 4J added param
	ChestTileEntity(int type, bool isBonusChest = false);		// 4J added param
	virtual ~ChestTileEntity();

private:
	ItemInstanceArray *items;

public:
	bool isBonusChest;					// 4J added
	bool hasCheckedNeighbors;
	weak_ptr<ChestTileEntity> n;
	weak_ptr<ChestTileEntity> e;
	weak_ptr<ChestTileEntity> w;
	weak_ptr<ChestTileEntity> s;

	float openness, oOpenness;
	int openCount;
private:
	int tickInterval;

	int type;
	wstring name;

	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20).
	// Oxidation field (0-7: stage 0-3, +4 waxed). The block data nibble
	// is 4 bits and could not hold facing (bits 0-2) + oxidation (bits
	// 3-5) — the clamp truncated it and an exposed chest reset to
	// unaffected on the next weathering roll — so the oxidation lives
	// here, in NBT ("Oxidation"), and the nibble keeps facing only.
	// Initialised in _init().
	int oxidation;

public:
	virtual unsigned int getContainerSize();
	virtual shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	virtual wstring getName();
	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);

	// Oxidation state (0-7: stage 0-3, +4 waxed). setOxidation clamps
	// to 0-7 and marks the TE changed. getOxidationStage is stage only
	// (0-3); waxed reuses the unwaxed texture/sound.
	int getOxidation();
	void setOxidation(int oxidation);
	int getOxidationStage();

	// Live sync: minimal tag (Oxidation only — NOT save(), which would
	// send the whole inventory). TYPE_COPPER_CHEST; the client applies
	// it via applyUpdate() (ClientConnection::handleTileEntityData
	// needs a TYPE_COPPER_CHEST branch for this to arrive).
	virtual shared_ptr<Packet> getUpdatePacket() override;
	virtual void applyUpdate(CompoundTag *tag);
	virtual int getMaxStackSize() const;
	virtual bool stillValid(shared_ptr<Player> player);
	virtual void setChanged();
	virtual void clearCache();

private:
	virtual void heyImYourNeighbor(shared_ptr<ChestTileEntity> neighbor, int from);
	// neoLegacy - Copper Age (Phase 6). Per-oxidation open/close sound
	// for a copper chest (see ChestTileEntity.cpp).
	int copperChestSound(bool opening);

public:
	virtual void checkNeighbors();

private:
	bool isSameChest(int x, int y, int z);

public:
	virtual void tick();
	virtual bool triggerEvent(int b0, int b1);
	virtual void startOpen();
	virtual void stopOpen();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
	virtual void setRemoved();
	virtual int getType();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};