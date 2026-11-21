#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "TileEntity.h"
#include "net.minecraft.world.level.tile.h"
#include "CopperOxidationTileEntity.h"
#include "TileEntityDataPacket.h"

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). See the
// header for the design: the oxidation field (0-7) lives in this tile
// entity's NBT because the 4-bit data nibble cannot hold it alongside
// the door/trapdoor/stairs structural bits.

CopperOxidationTileEntity::CopperOxidationTileEntity() : TileEntity()
{
	oxidation = 0;
}

int CopperOxidationTileEntity::getOxidation()
{
	return oxidation;
}

void CopperOxidationTileEntity::setOxidation(int oxidation)
{
	// Clamp to the 0-7 field. The ab08eeb4 SparseDataStorage clamp stops
	// the neighbour-corruption at the nibble level; this is the same
	// guard at the TE level so a bad write can never store an out-of-
	// range field here either.
	this->oxidation = oxidation & 7;
	setChanged();
}

int CopperOxidationTileEntity::getOxidationStage()
{
	return oxidation & 3;
}

void CopperOxidationTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	// Old saves (and chunks placed before this TE existed) have no
	// "Oxidation" key — default to unaffected (0) rather than failing.
	if (base->contains(L"Oxidation"))
	{
		oxidation = base->getByte(L"Oxidation") & 7;
	}
}

void CopperOxidationTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	base->putByte(L"Oxidation", static_cast<byte>(oxidation));
}

shared_ptr<TileEntity> CopperOxidationTileEntity::clone()
{
	shared_ptr<CopperOxidationTileEntity> result = std::make_shared<CopperOxidationTileEntity>();
	TileEntity::clone(result);
	result->oxidation = oxidation;
	return result;
}

shared_ptr<Packet> CopperOxidationTileEntity::getUpdatePacket()
{
	// Minimal tag: oxidation only. The tile (door/trapdoor/stairs) has
	// no other TE state, so save() == the sync payload.
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_COPPER_OXIDATION, tag);
}

void CopperOxidationTileEntity::applyUpdate(CompoundTag *tag)
{
	// Client-side only: apply the oxidation from a TileEntityDataPacket
	// without touching anything else. Do NOT call setChanged() here —
	// the client must not mark its chunk unsaved or re-read the nibble.
	if (tag != nullptr && tag->contains(L"Oxidation"))
	{
		oxidation = tag->getByte(L"Oxidation") & 7;
	}
}
