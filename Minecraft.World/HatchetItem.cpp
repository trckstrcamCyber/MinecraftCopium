#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "HatchetItem.h"
#include "StrippedLogTile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"

TileArray *HatchetItem::diggables = nullptr;

void HatchetItem::staticCtor()
{
	HatchetItem::diggables = new TileArray( HATCHET_DIGGABLES);
	diggables->data[0] = Tile::wood;
	diggables->data[1] = Tile::bookshelf;
	diggables->data[2] = Tile::treeTrunk;
	diggables->data[3] = Tile::chest;
	// 4J - brought forward from 1.2.3
	diggables->data[4] = Tile::stoneSlab;
	diggables->data[5] = Tile::stoneSlabHalf;
	diggables->data[6] = Tile::pumpkin;
	diggables->data[7] = Tile::litPumpkin;
}

HatchetItem::HatchetItem(int id, const Tier *tier) : DiggerItem (id, 3, tier, diggables)
{
}

// 4J - brought forward from 1.2.3
float HatchetItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
    if (tile != nullptr && (tile->material == Material::wood  || tile->material == Material::plant || tile->material == Material::replaceable_plant))
	{
        return speed;
    }
    return DiggerItem::getDestroySpeed(itemInstance, tile);
}

// neoLegacy - Copper Age (Phase 4). Axe used on a copper block: if waxed
// (field 4-7) remove the wax (field - 4); else if oxidized (field 1-3)
// de-oxidize one stage (field - 1). Follows the log-strip pattern (same
// sound, same 1-durability cost); there is no separate scrape sound in
// this tree (jar1201 has no scrape ogg), so the copper step sound is
// used. For the nibble-stored families the field lives in data bits 0-2;
// for the chest, door, trapdoor and stairs it lives in a tile entity
// instead (ChestTileEntity / CopperOxidationTileEntity, on the door's
// LOWER half), so those cases read/write the TE and broadcast the change
// instead of calling setTileAndData. Returns true if a scrape
// happened (or would happen, when bTestUseOnOnly), false if the block is
// not a scrapable copper block.
static bool scrapeCopper(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int tileId, int data, bool bTestUseOnOnly)
{
	int field;
	int newData = 0;
	// Oxidation stored in a tile entity (chest, door, trapdoor, stairs):
	// the TE holding it, its position, and the scraped field value. Null
	// for the nibble-stored families, which use newData instead.
	shared_ptr<ChestTileEntity> chestTe;
	shared_ptr<CopperOxidationTileEntity> oxTe;
	int oxX = x, oxY = y, oxZ = z;
	int newField = 0;
	switch (tileId)
	{
	// Field = data bits 0-2. CopperTile family (block, cut, chiseled,
	// grate, lantern, rod, statue), CopperFenceTile (chain, bars), bulb
	// (unlit and lit) and slab (half and full; bit 3 is TOP_SLOT_BIT and
	// is preserved by the ~7 mask).
	case Tile::copper_block_Id:
	case Tile::cut_copper_Id:
	case Tile::chiseled_copper_Id:
	case Tile::copper_grate_Id:
	case Tile::copper_lantern_Id:
	case Tile::lightning_rod_Id:
	case Tile::copper_golem_statue_Id:
	case Tile::copper_chain_Id:
	case Tile::copper_bars_Id:
	case Tile::copper_bulb_Id:
	case Tile::copper_bulb_lit_Id:
	case Tile::cut_copper_slab_Id:
	case Tile::cut_copper_slab_full_Id:
		field = data & 7;
		if (field == 0) return false; // unaffected and unwaxed: nothing to scrape
		newData = (data & ~7) | (field >= 4 ? field - 4 : field - 1);
		break;
	// Chest: the field lives in the ChestTileEntity, not the nibble
	// (facing is bits 0-2, so the nibble has no room for it).
	case Tile::copper_chest_Id:
		chestTe = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z));
		if (chestTe == nullptr) return false;
		field = chestTe->getOxidation();
		if (field == 0) return false;
		newField = field >= 4 ? field - 4 : field - 1;
		break;
	// Door: the field lives in the CopperOxidationTileEntity on the LOWER
	// half. Clicking either half scrapes the TE below the upper half.
	// The other half must still be a copper door.
	case Tile::copper_door_Id:
	{
		bool isUpper = (data & 8) != 0;
		int ux = x, uy = isUpper ? y - 1 : y + 1, uz = z;
		if (level->getTile(ux, uy, uz) != Tile::copper_door_Id) return false;
		oxX = x; oxY = isUpper ? y - 1 : y; oxZ = z;
		oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(level->getTileEntity(oxX, oxY, oxZ));
		if (oxTe == nullptr) return false;
		field = oxTe->getOxidation();
		if (field == 0) return false;
		newField = field >= 4 ? field - 4 : field - 1;
		break;
	}
	// Trapdoor: the field lives in the CopperOxidationTileEntity, not the
	// nibble (hinge/open/top leave no room for it).
	case Tile::copper_trapdoor_Id:
		oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(level->getTileEntity(x, y, z));
		if (oxTe == nullptr) return false;
		field = oxTe->getOxidation();
		if (field == 0) return false;
		newField = field >= 4 ? field - 4 : field - 1;
		break;
	// Stairs: the field lives in the CopperOxidationTileEntity, not the
	// nibble (facing is bits 0-1, upside-down bit 2, so no room for it).
	case Tile::cut_copper_stairs_Id:
		oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(level->getTileEntity(x, y, z));
		if (oxTe == nullptr) return false;
		field = oxTe->getOxidation();
		if (field == 0) return false;
		newField = field >= 4 ? field - 4 : field - 1;
		break;
	default:
		// Not a scrapable copper block. (The copper torch and shelf do not
		// oxidize, the copper button has no oxidation states, and rose-gold
		// tiles never oxidize.)
		return false;
	}

	if (!bTestUseOnOnly)
	{
		Tile *tile = Tile::tiles[tileId];
		level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
		if (level->isClientSide) return true;
		if (chestTe != nullptr)
		{
			// Chest oxidation lives in the TE; broadcast so clients see
			// it live (the chest renderer reads the TE every frame).
			chestTe->setOxidation(newField);
			level->broadcastTileEntityUpdate(chestTe);
			// Double chest: both halves share one oxidation stage
			// (Cameron's call, 2026-11-20), so the paired half takes
			// the sampled half's new STAGE, keeping its own waxed bit.
			// (An unwax scrape leaves the stage unchanged, so the
			// partner's stage does not move either — only the sampled
			// half loses its wax.)
			chestTe->checkNeighbors();
			shared_ptr<ChestTileEntity> pairTe = chestTe->e.lock();
			if (pairTe == nullptr) pairTe = chestTe->w.lock();
			if (pairTe == nullptr) pairTe = chestTe->n.lock();
			if (pairTe == nullptr) pairTe = chestTe->s.lock();
			if (pairTe != nullptr)
			{
				pairTe->setOxidation((newField & 3) | (pairTe->getOxidation() & 4));
				level->broadcastTileEntityUpdate(pairTe);
				level->setTilesDirty(pairTe->x, pairTe->y, pairTe->z, pairTe->x, pairTe->y, pairTe->z);
			}
			level->setTilesDirty(x, y, z, x, y, z);
		}
		else if (oxTe != nullptr)
		{
			// Door/trapdoor/stairs oxidation lives in the TE; the texture
			// is baked into the chunk mesh, so rebuild it, and broadcast
			// so clients see it live. A door's oxidation is read from
			// the LOWER half's TE by BOTH halves, so rebuild both halves
			// (the upper half would otherwise keep the stale texture).
			oxTe->setOxidation(newField);
			level->broadcastTileEntityUpdate(oxTe);
			if (tileId == Tile::copper_door_Id) level->setTilesDirty(oxX, oxY - 1, oxZ, oxX, oxY + 1, oxZ);
			else level->setTilesDirty(oxX, oxY, oxZ, oxX, oxY, oxZ);
		}
		else
		{
			// Nibble-stored family. The statue is an entity tile too
			// (its pose/facing live in the TE), and a same-id
			// setTileAndData rewrite runs onRemove on the server,
			// deleting the TE and recreating it with default
			// pose/facing — so write its data with setData instead
			// (only the data changes; the statue emits no light). The
			// other nibble blocks keep setTileAndData (the bulb needs
			// its light re-check).
			if (tileId == Tile::copper_golem_statue_Id) level->setData(x, y, z, newData, Tile::UPDATE_ALL);
			else level->setTileAndData(x, y, z, tileId, newData, Tile::UPDATE_ALL);
		}
		instance->hurtAndBreak(1, player);
	}
	return true;
}

// neoLegacy - right-click a log or a Block of Bamboo with any axe to strip it.
// The stripped version keeps the axis the block was placed on (up/down, east/west, north/south).
//     log   (oak, spruce, birch, jungle)  -> stripped_log   (same wood index 0-3)
//     log2  (acacia, dark oak)            -> stripped_log2  (wood index 0-1)
//     Block of Bamboo                     -> stripped_log2  (wood index 2)
bool HatchetItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	int tileId = level->getTile(x, y, z);
	Tile *stripped = nullptr;
	int woodIndex = 0;
	int data = level->getData(x, y, z);

	if (tileId == Tile::log_Id)
	{
		stripped = Tile::strippedLog;
		woodIndex = data & RotatedPillarTile::MASK_TYPE;
	}
	else if (tileId == Tile::log2_Id)
	{
		stripped = Tile::strippedLog2;
		woodIndex = data & RotatedPillarTile::MASK_TYPE;
	}
	else if (tileId == Tile::bamboo_block_Id)
	{
		stripped = Tile::strippedLog2;
		woodIndex = StrippedLogTile::BAMBOO - 4;
	}
	if (stripped == nullptr)
	{
		// neoLegacy - Copper Age (Phase 4). Not a log: try a copper scrape
		// (remove wax / de-oxidize one stage). Returns false if the block
		// is not a scrapable copper block, so the normal axe use proceeds.
		return scrapeCopper(instance, player, level, x, y, z, tileId, data, bTestUseOnOnly);
	}

	if (!bTestUseOnOnly)
	{
		level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, stripped->soundType->getStepSound(), (stripped->soundType->getVolume() + 1) / 2, stripped->soundType->getPitch() * 0.8f);
		if (level->isClientSide) return true;
		int facing = data & RotatedPillarTile::MASK_FACING;
		level->setTileAndData(x, y, z, stripped->id, woodIndex | facing, Tile::UPDATE_ALL);
		instance->hurtAndBreak(1, player);
	}
	return true;
}
