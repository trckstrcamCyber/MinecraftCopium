#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.material.h"
#include "BambooTile.h"
#include "IconRegister.h"
#include "Random.h"
#include "Facing.h"

BambooTile::BambooTile(int id) : Tile(id, Material::wood, false)
{
	stalkIcon = smallLeavesIcon = largeLeavesIcon = nullptr;
	setTicking(true);
	updateDefaultShape();
}

bool BambooTile::isValidSoil(int t)
{
	return t == Tile::grass_Id || t == Tile::dirt_Id || t == Tile::sand_Id || t == Tile::gravel_Id || t == Tile::mycelium_Id;
}

void BambooTile::getOffset(int x, int z, float &offsetX, float &offsetZ)
{
	int64_t seed = (x * 3129871) ^ (z * 116129781LL);
	seed = seed * seed * 42317861 + seed * 11;
	offsetX = ((((seed >> 16) & 0xf) / 15.0f) - 0.5f) * 0.5f;		// +-0.25 like Java
	offsetZ = ((((seed >> 24) & 0xf) / 15.0f) - 0.5f) * 0.5f;
}

Icon *BambooTile::getLeavesIcon(int leaves)
{
	return leaves == LEAVES_LARGE ? largeLeavesIcon : smallLeavesIcon;
}

void BambooTile::registerIcons(IconRegister *iconRegister)
{
	stalkIcon = iconRegister->registerIcon(L"bamboo_stalk");
	smallLeavesIcon = iconRegister->registerIcon(L"bamboo_small_leaves");
	largeLeavesIcon = iconRegister->registerIcon(L"bamboo_large_leaves");
}

Icon *BambooTile::getTexture(int face, int data)
{
	return stalkIcon;
}

bool BambooTile::isSolidRender(bool isServerLevel) { return false; }
bool BambooTile::isCubeShaped() { return false; }
bool BambooTile::blocksLight() { return false; }
int BambooTile::getRenderShape() { return Tile::SHAPE_BAMBOO; }

void BambooTile::updateDefaultShape()
{
	setShape(0.5f - 1.5f / 16.0f, 0, 0.5f - 1.5f / 16.0f, 0.5f + 1.5f / 16.0f, 1, 0.5f + 1.5f / 16.0f);
}

void BambooTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	int data = (forceData == -1) ? level->getData(x, y, z) : forceData;
	float half = ((data & AGE_BIT) != 0 ? 1.5f : 1.0f) / 16.0f;		// thin stalk 2px, thick stalk 3px
	float ox, oz;
	getOffset(x, z, ox, oz);
	setShape(0.5f - half + ox, 0, 0.5f - half + oz, 0.5f + half + ox, 1, 0.5f + half + oz);
}

AABB *BambooTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

AABB *BambooTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

int BambooTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;		// a piston breaks the stalk instead of moving it
}

int BambooTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::bamboo_Id;
}

bool BambooTile::canSurvive(Level *level, int x, int y, int z)
{
	int below = level->getTile(x, y - 1, z);
	return below == id || isValidSoil(below);
}

bool BambooTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && canSurvive(level, x, y, z);
}

void BambooTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	Tile::neighborChanged(level, x, y, z, type);
	if (!canSurvive(level, x, y, z))
	{
		// losing the block underneath (or the stalk below breaking) pops this block, which in turn pops the one above it
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTileAndData(x, y, z, 0, 0, Tile::UPDATE_ALL);
	}
}

int BambooTile::countBambooBelow(Level *level, int x, int y, int z)
{
	int count = 0;
	while (count < MAX_HEIGHT && level->getTile(x, y - count - 1, z) == id) ++count;
	return count;
}

// random tick: Java's BambooStalkBlock.randomTick
void BambooTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;
	int data = level->getData(x, y, z);
	if ((data & STAGE_BIT) != 0) return;		// finished growing
	if (random->nextInt(3) != 0) return;
	if (!level->isEmptyTile(x, y + 1, z)) return;
	if (level->getRawBrightness(x, y + 1, z) < MIN_LIGHT_TO_GROW) return;

	int height = countBambooBelow(level, x, y, z) + 1;
	if (height < MAX_HEIGHT) growBamboo(level, x, y, z, random, height);
}

// Java's BambooStalkBlock.growBamboo: adds one block on top, keeps the leaves on the top three blocks
void BambooTile::growBamboo(Level *level, int x, int y, int z, Random *random, int height)
{
	int data = level->getData(x, y, z);
	int below1Id = level->getTile(x, y - 1, z);
	int below2Id = level->getTile(x, y - 2, z);
	int below1Data = level->getData(x, y - 1, z);

	int leaves = LEAVES_NONE;
	if (height >= 1)
	{
		if (below1Id != id || ((below1Data & LEAVES_MASK) >> LEAVES_SHIFT) == LEAVES_NONE)
		{
			leaves = LEAVES_SMALL;
		}
		else
		{
			leaves = LEAVES_LARGE;
			if (below2Id == id)
			{
				// the block below turns small, and the one under that loses its leaves
				level->setTileAndData(x, y - 1, z, id, (below1Data & ~LEAVES_MASK) | (LEAVES_SMALL << LEAVES_SHIFT), Tile::UPDATE_ALL);
				int below2Data = level->getData(x, y - 2, z);
				level->setTileAndData(x, y - 2, z, id, below2Data & ~LEAVES_MASK, Tile::UPDATE_ALL);
			}
		}
	}

	int age = ((data & AGE_BIT) != 0 || below2Id == id) ? AGE_BIT : 0;
	int stage = ((height >= 11 && random->nextFloat() < 0.25f) || height == MAX_HEIGHT - 1) ? STAGE_BIT : 0;
	level->setTileAndData(x, y + 1, z, id, age | stage | (leaves << LEAVES_SHIFT), Tile::UPDATE_ALL);
}
