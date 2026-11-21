#pragma once

#include "TileEntity.h"

class CompoundTag;
class CopperGolem;
class Packet;

// neoLegacy - Copper Age (Phase 5). Tile entity for the copper golem
// statue (Tile 2320). Stores the custom name: when an OXIDIZED golem
// turns to statue, createStatue() copies the golem's custom name onto the
// statue (Java CopperGolemStatueBlockEntity.createStatue, copper-code.txt
// :4426). The name round-trips through chunk save/load ("CustomName").
//
// Oxidation workaround (2026-11-20): pose and facing ALSO live here
// ("Pose" / "Facing" NBT bytes, 0-3 each). They used to be packed with
// oxidation into the 4-bit block-data nibble (pose bits 3-4, facing
// bits 5-6), but the SparseDataStorage clamp truncates the 6-bit value
// to 15, so pose was 0/1 only, facing was always NORTH, and neither
// persisted. The nibble now keeps oxidation (+ waxed) ONLY (bits 0-2);
// pose/facing are TE fields, synced live via getUpdatePacket() /
// applyUpdate() (TYPE_COPPER_STATUE) and Level::broadcastTileEntityUpdate.
//
// removeStatue() (Java :4435) builds the golem back from the statue — same
// custom name, standing at the block centre, facing the statue's facing,
// spawn sound played — and returns it. It IS wired to a trigger:
// CopperGolemStatueBlock::use() calls it when an axe is used on a fully
// oxidized, unwaxed statue (audit fix, Java reanimate). Breaking the
// statue drops the statue as an item (aux 0 — it places back as an
// unaffected standing statue); it does NOT spawn a golem on break.
class CopperGolemStatueBlockEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_COPPERGOLEMSTATUETILEENTITY; }
	static TileEntity *create() { return new CopperGolemStatueBlockEntity(); }

private:
	wstring name;
	// Oxidation workaround: pose + facing live here, not in the nibble.
	// pose: 0=STANDING, 1=SITTING, 2=RUNNING, 3=STAR.
	// facing: 0=NORTH, 1=EAST, 2=SOUTH, 3=WEST (ChestTile convention).
	int pose;
	int facing;

public:
	CopperGolemStatueBlockEntity();

	// Pose (0-3). setPose clamps to 0-3 and marks the TE changed (which
	// also notifies comparators via TileEntity::setChanged).
	int getPose();
	void setPose(int pose);
	// Facing (0-3). setFacing clamps to 0-3 and marks the TE changed.
	int getFacing();
	void setFacing(int facing);

	// Copy the golem's custom name onto this statue (Java createStatue).
	virtual void createStatue(shared_ptr<CopperGolem> golem);

	// Build the golem back from this statue (Java removeStatue). The
	// caller is responsible for removing the statue block and adding the
	// returned golem to the level; this only constructs + positions it.
	// Returns nullptr if the golem could not be created.
	virtual shared_ptr<CopperGolem> removeStatue();

	virtual wstring getCustomName();
	virtual bool hasCustomName();
	virtual void setCustomName(const wstring &name);
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual shared_ptr<TileEntity> clone();

	// Live sync: minimal tag (Pose + Facing only). The client branch in
	// ClientConnection::handleTileEntityData calls applyUpdate, NOT
	// load(), so no other TE state is touched.
	virtual shared_ptr<Packet> getUpdatePacket() override;
	virtual void applyUpdate(CompoundTag *tag);
};
