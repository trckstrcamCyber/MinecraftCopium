#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.biome.h"

DesertBiome::DesertBiome(int id) : Biome(id)
{
	// remove default mob spawn settings
	friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	friendlies_wolf.clear(); 	// 4J added
	friendlies.push_back(new MobSpawnerData(eTYPE_RABBIT, 12, 2, 3));
	// (camels do not spawn naturally: one is placed at the centre of each desert village instead)
	// neoLegacy - husks take the place of zombies in deserts
	for (auto it = enemies.begin(); it != enemies.end();)
	{
		if ((*it)->mobClass == eTYPE_ZOMBIE) it = enemies.erase(it); else ++it;
	}
	enemies.push_back(new MobSpawnerData(eTYPE_HUSK, 10, 4, 4));
	// neoLegacy - rare Jelly Slimes (by day, on the surface: JellySlime::canSpawn)
	enemies.push_back(new MobSpawnerData(eTYPE_JELLYSLIME, 3, 1, 1));
	topMaterial = static_cast<BYTE>(Tile::sand_Id);
	material = static_cast<BYTE>(Tile::sand_Id);

	decorator->treeCount = -999;
	decorator->deadBushCount = 2;
	decorator->reedsCount = 50;
	decorator->cactusCount = 10;
}

void DesertBiome::decorate(Level *level, Random *random, int xo, int zo)
{
	Biome::decorate(level, random, xo, zo);

	if (random->nextInt(1000) == 0)
	{
		int x = xo + random->nextInt(16) + 8;
		int z = zo + random->nextInt(16) + 8;
		Feature *well = new DesertWellFeature();
		well->place(level, random, x, level->getHeightmap(x, z) + 1, z);
	}
}