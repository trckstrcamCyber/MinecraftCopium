#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "BambooShootTile.h"
#include "BambooTile.h"
#include "IconRegister.h"
#include "Random.h"

BambooShootTile::BambooShootTile(int id) : Bush(id)
{
	shootIcon = nullptr;
	setShape(0.5f - 0.25f, 0, 0.5f - 0.25f, 0.5f + 0.25f, 0.75f, 0.5f + 0.25f);
}

void BambooShootTile::registerIcons(IconRegister *iconRegister)
{
	shootIcon = iconRegister->registerIcon(L"bamboo_stage0");
}

Icon *BambooShootTile::getTexture(int face, int data)
{
	return shootIcon;
}

bool BambooShootTile::mayPlaceOn(int tile)
{
	return BambooTile::isValidSoil(tile);
}

bool BambooShootTile::canSurvive(Level *level, int x, int y, int z)
{
	return mayPlaceOn(level->getTile(x, y - 1, z));		// no light requirement to survive, only to grow
}

int BambooShootTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::bamboo_Id;
}

// Java's BambooSaplingBlock.randomTick: the shoot becomes a stalk with a first leafy block on top
void BambooShootTile::tick(Level *level, int x, int y, int z, Random *random)
{
	checkAlive(level, x, y, z);
	if (level->isClientSide) return;
	if (random->nextInt(3) != 0) return;
	if (!level->isEmptyTile(x, y + 1, z)) return;
	if (level->getRawBrightness(x, y + 1, z) < BambooTile::MIN_LIGHT_TO_GROW) return;

	level->setTileAndData(x, y, z, Tile::bamboo_Id, 0, Tile::UPDATE_ALL);
	level->setTileAndData(x, y + 1, z, Tile::bamboo_Id, BambooTile::LEAVES_SMALL << BambooTile::LEAVES_SHIFT, Tile::UPDATE_ALL);
}
