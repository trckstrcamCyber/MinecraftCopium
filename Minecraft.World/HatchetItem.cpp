#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "HatchetItem.h"
#include "StrippedLogTile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
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
	if (stripped == nullptr) return false;

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
