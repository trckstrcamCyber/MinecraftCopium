#pragma once

#include "TileEntity.h"

class CompoundTag;
class Packet;

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Shared tile
// entity that stores a copper block's oxidation state (0-7: stage 0-3,
// +4 when waxed) OUTSIDE the 4-bit data nibble.
//
// The data nibble (SparseDataStorage, clamped to 0-15 by the ab08eeb4 fix)
// cannot hold oxidation + structural bits for the copper door (dir/open/
// hinge), trapdoor (hinge/open/top) and cut-copper stairs (dir/upside-
// down): those fields need 3 bits on top of 4 structural bits. Storing
// the oxidation in the nibble silently truncated it (door/trapdoor never
// weathered; stairs/chest reset to unaffected), and the pre-clamp writes
// spilled into the neighbour block's nibble ("blocks randomly turning
// into other blocks"). The oxidation lives here instead, in NBT, and the
// nibble keeps only the structural bits. The chest and the golem statue
// already have tile entities, so they store their own oxidation/pose/
// facing fields; this class is for the three blocks that had none.
//
// Persistence: "Oxidation" byte in NBT (load defaults to 0 = unaffected
// when the key is absent, so old saves load cleanly as unaffected).
// Sync: getUpdatePacket() + applyUpdate() (TYPE_COPPER_OXIDATION); the
// client applies the tag to its own TE, and Level::broadcastTileEntity-
// Update pushes live changes (weathering / axe scrape / placement).
class CopperOxidationTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_COPPEROXIDATIONTILEENTITY; }
	static TileEntity *create() { return new CopperOxidationTileEntity(); }

private:
	// Oxidation field (0-7): stage 0-3, +4 when waxed. Same encoding as
	// the CopperTile family (data bits 0-2), so getTexture / drops /
	// weathering all read it identically.
	int oxidation;

public:
	CopperOxidationTileEntity();

	// Oxidation state (0-7). setOxidation clamps to 0-7 and marks the TE
	// changed (chunk unsaved + neighbour output signal).
	int getOxidation();
	void setOxidation(int oxidation);
	// Stage only (0-3); waxed reuses the unwaxed texture/sound.
	int getOxidationStage();

	virtual void load(CompoundTag *base) override;
	virtual void save(CompoundTag *base) override;
	virtual shared_ptr<TileEntity> clone() override;

	// Live sync: minimal tag (Oxidation only). The client branch in
	// ClientConnection::handleTileEntityData calls applyUpdate, NOT
	// load(), so no other TE state is touched.
	virtual shared_ptr<Packet> getUpdatePacket() override;
	virtual void applyUpdate(CompoundTag *tag);
};
