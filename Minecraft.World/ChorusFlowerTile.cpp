#include "stdafx.h"
#include "LevelEvent.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "IconRegister.h"
#include "Facing.h"
#include "ChorusFlowerTile.h"

ChorusFlowerTile::ChorusFlowerTile(int id) : Tile(id, Material::plant, false)
{
	iconDead = nullptr;
	iconPlant = nullptr;
	setLightBlock(0);
	setTicking(true);
}

bool ChorusFlowerTile::allNeighborsEmpty(Level *level, int x, int y, int z, int excludedFace)
{
	for (int face = Facing::NORTH; face <= Facing::EAST; ++face)
	{
		if (face != excludedFace && !level->isEmptyTile(x + Facing::STEP_X[face], y, z + Facing::STEP_Z[face]))
		{
			return false;
		}
	}
	return true;
}

void ChorusFlowerTile::placeGrownFlower(Level *level, int x, int y, int z, int age)
{
	level->setTileAndData(x, y, z, id, age, Tile::UPDATE_CLIENTS);
	level->levelEvent(LevelEvent::SOUND_CHORUS_GROW, x, y, z, 0);
}

void ChorusFlowerTile::placeDeadFlower(Level *level, int x, int y, int z)
{
	level->setTileAndData(x, y, z, id, DEAD_AGE, Tile::UPDATE_CLIENTS);
	level->levelEvent(LevelEvent::SOUND_CHORUS_DEATH, x, y, z, 0);
}

void ChorusFlowerTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!canSurvive(level, x, y, z))
	{
		level->destroyTile(x, y, z, true);
		return;
	}

	if (!level->isEmptyTile(x, y + 1, z) || y + 1 >= Level::maxBuildHeight) return;

	int age = level->getData(x, y, z);
	if (isDead(age)) return;

	bool growUp = false;
	bool rooted = false;
	int below = level->getTile(x, y - 1, z);
	if (below == Tile::end_stone_Id)
	{
		growUp = true;
	}
	else if (below == Tile::chorus_plant_Id)
	{
		int stemHeight = 1;
		for (int i = 0; i < 4; ++i)
		{
			int support = level->getTile(x, y - (stemHeight + 1), z);
			if (support == Tile::chorus_plant_Id)
			{
				++stemHeight;
			}
			else
			{
				if (support == Tile::end_stone_Id) rooted = true;
				break;
			}
		}

		int limit = rooted ? 5 : 4;
		if (stemHeight < 2 || random->nextInt(limit) >= stemHeight)
		{
			growUp = true;
		}
	}
	else if (below == 0)
	{
		growUp = true;
	}

	if (growUp && allNeighborsEmpty(level, x, y + 1, z, -1) && level->isEmptyTile(x, y + 2, z))
	{
		level->setTileAndData(x, y, z, Tile::chorus_plant_Id, 0, Tile::UPDATE_CLIENTS);
		placeGrownFlower(level, x, y + 1, z, age);
	}
	else if (age < 4)
	{
		int branches = random->nextInt(4);
		if (rooted) ++branches;
		bool grew = false;
		for (int i = 0; i < branches; ++i)
		{
			int face = Facing::NORTH + random->nextInt(4);
			int bx = x + Facing::STEP_X[face];
			int bz = z + Facing::STEP_Z[face];
			if (level->isEmptyTile(bx, y, bz) && level->isEmptyTile(bx, y - 1, bz)
				&& allNeighborsEmpty(level, bx, y, bz, Facing::OPPOSITE_FACING[face]))
			{
				placeGrownFlower(level, bx, y, bz, age + 1);
				grew = true;
			}
		}
		if (grew)
		{
			level->setTileAndData(x, y, z, Tile::chorus_plant_Id, 0, Tile::UPDATE_CLIENTS);
		}
		else
		{
			placeDeadFlower(level, x, y, z);
		}
	}
	else if (age == 4)
	{
		placeDeadFlower(level, x, y, z);
	}
}

bool ChorusFlowerTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool ChorusFlowerTile::isCubeShaped()
{
	return false;
}

int ChorusFlowerTile::getRenderShape()
{
	return Tile::SHAPE_CHORUS_FLOWER;
}

int ChorusFlowerTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

bool ChorusFlowerTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && canSurvive(level, x, y, z);
}

void ChorusFlowerTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!canSurvive(level, x, y, z))
	{
		level->addToTickNextTick(x, y, z, id, 1);
	}
}

bool ChorusFlowerTile::canSurvive(Level *level, int x, int y, int z)
{
	int below = level->getTile(x, y - 1, z);
	if (below == Tile::chorus_plant_Id || below == Tile::end_stone_Id)
	{
		return true;
	}
	if (below == 0)
	{
		int plants = 0;
		for (int face = Facing::NORTH; face <= Facing::EAST; ++face)
		{
			int t = level->getTile(x + Facing::STEP_X[face], y, z + Facing::STEP_Z[face]);
			if (t == Tile::chorus_plant_Id)
			{
				++plants;
			}
			else if (t != 0)
			{
				return false;
			}
		}
		return plants == 1;
	}
	return false;
}

void ChorusFlowerTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	Tile::playerDestroy(level, player, x, y, z, data);
	popResource(level, x, y, z, std::make_shared<ItemInstance>(id, 1, 0));
}

int ChorusFlowerTile::getSpawnResourcesAuxValue(int data)
{
	return 0;
}

bool ChorusFlowerTile::isSilkTouchable()
{
	return false;
}

Icon *ChorusFlowerTile::getTexture(int face, int data)
{
	if (face == Facing::DOWN) return iconPlant;
	return isDead(data) ? iconDead : icon;
}

void ChorusFlowerTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"chorus_flower");
	iconDead = iconRegister->registerIcon(L"chorus_flower_dead");
	iconPlant = iconRegister->registerIcon(L"chorus_flower_plant");
}

void ChorusFlowerTile::growTreeRecursive(Level *level, int x, int y, int z, Random *random, int originX, int originZ, int radius, int depth)
{
	int height = random->nextInt(4) + 1;
	if (depth == 0) ++height;

	for (int i = 0; i < height; ++i)
	{
		if (!allNeighborsEmpty(level, x, y + i + 1, z, -1))
		{
			return;
		}
		level->setTileAndData(x, y + i + 1, z, Tile::chorus_plant_Id, 0, Tile::UPDATE_CLIENTS);
	}

	bool grew = false;
	if (depth < 4)
	{
		int branches = random->nextInt(4);
		if (depth == 0) ++branches;
		for (int i = 0; i < branches; ++i)
		{
			int face = Facing::NORTH + random->nextInt(4);
			int bx = x + Facing::STEP_X[face];
			int by = y + height;
			int bz = z + Facing::STEP_Z[face];
			if (abs(bx - originX) < radius && abs(bz - originZ) < radius && level->isEmptyTile(bx, by, bz)
				&& level->isEmptyTile(bx, by - 1, bz) && allNeighborsEmpty(level, bx, by, bz, Facing::OPPOSITE_FACING[face]))
			{
				grew = true;
				level->setTileAndData(bx, by, bz, Tile::chorus_plant_Id, 0, Tile::UPDATE_CLIENTS);
				growTreeRecursive(level, bx, by, bz, random, originX, originZ, radius, depth + 1);
			}
		}
	}

	if (!grew)
	{
		level->setTileAndData(x, y + height, z, Tile::chorus_flower_Id, DEAD_AGE, Tile::UPDATE_CLIENTS);
	}
}

void ChorusFlowerTile::generatePlant(Level *level, int x, int y, int z, Random *random, int radius)
{
	level->setTileAndData(x, y, z, Tile::chorus_plant_Id, 0, Tile::UPDATE_CLIENTS);
	growTreeRecursive(level, x, y, z, random, x, z, radius, 0);
}
