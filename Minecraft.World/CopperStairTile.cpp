#include "stdafx.h"
#include "CopperStairTile.h"
#include "CopperOxidationTileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include <tuple>

// neoLegacy - Copper Age (review fix, 2026-11-20). Oxidation snapshot
// for blocks about to be removed, keyed by block position (see
// DoorTile.cpp — same design). The player-break path removes the tile
// (and its TE) before spawnResources runs, so onRemoving records the
// oxidation here first and spawnResources falls back to it when the TE
// is already gone. Entries are consumed on read; a stale entry is
// harmless — the next removal at that position overwrites it.
static std::map<std::tuple<int, int, int>, int> s_removedOxidation;

CopperStairTile::CopperStairTile(int id) : StairTile(id, Tile::cutCopper, 0)
{
	// StairTile is not an entity tile: mark this one as one by hand (the
	// BaseEntityTile ctor does the same for its subclasses), so
	// LevelChunk::setTileAndData creates a CopperOxidationTileEntity on
	// placement and getTileEntity finds it afterwards.
	_isEntityTile = true;
}

shared_ptr<TileEntity> CopperStairTile::newTileEntity(Level *level)
{
	return std::make_shared<CopperOxidationTileEntity>();
}

void CopperStairTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	// Drop the oxidation TE with the block (BaseEntityTile::onRemove does
	// the same for its subclasses); then run the normal stair removal,
	// which delegates to the base block (cut copper).
	level->removeTileEntity(x, y, z);
	StairTile::onRemove(level, x, y, z, id, data);
}

Icon *CopperStairTile::getTexture(int face, int data)
{
	// Item form: the item aux IS the oxidation field (0-7); cutCopper's
	// getTexture masks it to the stage internally (data & 3).
	return Tile::cutCopper->getTexture(face, data & 7);
}

Icon *CopperStairTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	// In-world form: the oxidation field lives in the TE, not the nibble.
	int stage = 0;
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (ox != nullptr) stage = ox->getOxidationStage();
	return Tile::cutCopper->getTexture(face, stage);
}

void CopperStairTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	if (by == nullptr) return;

	// Base StairTile::setPlacedBy rewrites the data as DIR | UPSIDEDOWN
	// (structural bits only — no oxidation bits to preserve any more);
	// the item aux is the oxidation field and goes into the TE instead.
	int usd = level->getData(x, y, z) & UPSIDEDOWN_BIT;
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;

	if (dir == 0) level->setData(x, y, z, DIR_SOUTH | usd, Tile::UPDATE_CLIENTS);
	if (dir == 1) level->setData(x, y, z, DIR_WEST | usd, Tile::UPDATE_CLIENTS);
	if (dir == 2) level->setData(x, y, z, DIR_NORTH | usd, Tile::UPDATE_CLIENTS);
	if (dir == 3) level->setData(x, y, z, DIR_EAST | usd, Tile::UPDATE_CLIENTS);

	if (itemInstance != nullptr)
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
		if (ox != nullptr)
		{
			ox->setOxidation(itemInstance->getAuxValue() & 7);
			level->broadcastTileEntityUpdate(ox);
		}
	}
}

int CopperStairTile::getSpawnResourcesAuxValue(int data)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): the
	// oxidation no longer lives in the nibble, so there is nothing to
	// read here — the value below only ever reflects legacy nibble bits
	// (0 for live blocks). Drops go through the spawnResources override,
	// which reads the CopperOxidationTileEntity, and pick-block goes
	// through the cloneTileData override below, which reads the TE too.
	return (data >> 3) & 7;
}

// neoLegacy - Copper Age (review fix, 2026-11-20). Snapshot the
// oxidation just before the block is removed. LevelChunk::setTileAndData
// calls onRemoving on the server before the tile is replaced and before
// onRemove drops the TE — but the player-break path
// (ServerPlayerGameMode::destroyBlock) calls spawnResources only AFTER
// that removal, so a TE read there would always miss. Recording the
// oxidation here lets spawnResources fall back to it.
void CopperStairTile::onRemoving(Level *level, int x, int y, int z, int data)
{
	(void)data;
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (ox != nullptr) s_removedOxidation[std::make_tuple(x, y, z)] = ox->getOxidation();
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The base
// spawnResources drops the item with getSpawnResourcesAuxValue(data),
// which cannot see the TE — so read the TE here and drop with its
// oxidation aux, mirroring DoorTile/TrapDoorTile. The TE is usually
// already gone by the time this runs (the player-break path removes
// the tile before dropping), so fall back to the oxidation onRemoving
// snapshotted just before removal.
void CopperStairTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (level->isClientSide) return;
	int ox = 0;
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (oxTe != nullptr) ox = oxTe->getOxidation();
	else
	{
		auto it = s_removedOxidation.find(std::make_tuple(x, y, z));
		if (it != s_removedOxidation.end())
		{
			ox = it->second;
			s_removedOxidation.erase(it);
		}
	}
	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		int type = getResource(data, level->random, playerBonusLevel);
		if (type <= 0) continue;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(type, 1, ox));
	}
}

int CopperStairTile::cloneTileData(Level *level, int x, int y, int z)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) on a copper stair gives the stair item with the TE
	// oxidation aux.
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (oxTe != nullptr) return oxTe->getOxidation();
	return Tile::cloneTileData(level, x, y, z);
}
