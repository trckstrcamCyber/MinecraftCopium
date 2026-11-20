#pragma once

#include "TileEntity.h"

class CompoundTag;
class CopperGolem;

// neoLegacy - Copper Age (Phase 5). Tile entity for the copper golem
// statue (Tile 2320). Stores only the custom name: when an OXIDIZED golem
// turns to statue, createStatue() copies the golem's custom name onto the
// statue (Java CopperGolemStatueBlockEntity.createStatue, copper-code.txt
// :4426). The name round-trips through chunk save/load ("CustomName").
//
// removeStatue() (Java :4435) builds the golem back from the statue — same
// custom name, standing at the block centre, facing the statue's facing,
// spawn sound played — and returns it. It is implemented but NOT wired to
// any trigger: the extracted Java text contains exactly one occurrence of
// removeStatue (the definition) and no caller, so nothing in this tree
// calls it yet. Breaking the statue drops nothing and spawns no golem
// until Cameron settles the trigger.
class CopperGolemStatueBlockEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_COPPERGOLEMSTATUETILEENTITY; }
	static TileEntity *create() { return new CopperGolemStatueBlockEntity(); }

private:
	wstring name;

public:
	CopperGolemStatueBlockEntity();

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
};
