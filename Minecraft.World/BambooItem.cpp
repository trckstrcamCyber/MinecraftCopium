#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "BambooItem.h"
#include "BambooTile.h"
#include "Facing.h"

BambooItem::BambooItem(int id) : Item(id)
{
}

bool BambooItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// Planting on the side of a bamboo plant should still work like Java: it just uses the usual neighbouring position
	int clickedTile = level->getTile(x, y, z);
	if (clickedTile == Tile::snow_Id)
	{
		face = Facing::UP;		// plants replace snow layers
	}
	else if (clickedTile != Tile::vine_Id && clickedTile != Tile::tallgrass_Id && clickedTile != Tile::deadBush->id)
	{
		if (face == 0) y--;
		if (face == 1) y++;
		if (face == 2) z--;
		if (face == 3) z++;
		if (face == 4) x--;
		if (face == 5) x++;
	}

	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;
	if (instance->count == 0) return false;
	if (!level->isEmptyTile(x, y, z) && !Tile::tiles[level->getTile(x, y, z)]->material->isReplaceable()) return false;

	int belowId = level->getTile(x, y - 1, z);
	int newTile = 0;
	int newData = 0;
	bool growShootBelow = false;
	if (belowId == Tile::bamboo_Id)
	{
		// extend the stalk: the new block copies the thickness of the one below
		int belowData = level->getData(x, y - 1, z);
		int age = (belowData & BambooTile::AGE_BIT);
		newTile = Tile::bamboo_Id;
		newData = age;
	}
	else if (belowId == Tile::bamboo_shoot_Id)
	{
		// planting on a shoot grows it, exactly as if it had grown by itself: the shoot becomes a stalk and the new block
		// is its first (small-leafed) block on top. (The stalk has to be placed first - a bamboo block needs a stalk or soil under it.)
		newTile = Tile::bamboo_Id;
		newData = BambooTile::LEAVES_SMALL << BambooTile::LEAVES_SHIFT;
		growShootBelow = true;
	}
	else if (BambooTile::isValidSoil(belowId))
	{
		newTile = Tile::bamboo_shoot_Id;
	}
	else
	{
		return false;
	}

	if (bTestUseOnOnly) return true;

	if (!level->isClientSide)
	{
		if (growShootBelow) level->setTileAndData(x, y - 1, z, Tile::bamboo_Id, 0, Tile::UPDATE_ALL);
		level->setTileAndData(x, y, z, newTile, newData, Tile::UPDATE_ALL);
	}
	Tile *placed = Tile::tiles[newTile];
	level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, placed->soundType->getPlaceSound(), (placed->soundType->getVolume() + 1) / 2, placed->soundType->getPitch() * 0.8f);
	instance->count--;
	return true;
}
