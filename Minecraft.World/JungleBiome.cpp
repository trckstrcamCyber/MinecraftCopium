	#include "stdafx.h"
	#include "net.minecraft.world.level.biome.h"
	#include "net.minecraft.world.level.levelgen.feature.h"
	#include "net.minecraft.world.level.tile.h"
	#include "net.minecraft.world.level.h"
	#include "net.minecraft.world.entity.animal.h"
	#include "JungleBiome.h"
	#include "MelonFeature.h"
	#include "BambooFeature.h"

	JungleBiome::JungleBiome(int id, bool isEdge, bool isBamboo) : Biome(id)
	{
		this->isEdge = isEdge;
		this->isBamboo = isBamboo;

		if (isEdge)
		{
			decorator->treeCount = 2;
		}
		else
		{
			decorator->treeCount = isBamboo ? 10 : 50;	// the Bamboo Jungle is mostly bamboo, with only a few trees
			enemies.push_back(new MobSpawnerData(eTYPE_OCELOT, 2, 1, 1));
		}

		decorator->grassCount = 25;
		decorator->flowerCount = 4;

		// make chicken a lot more common in the jungle
		friendlies.push_back(new MobSpawnerData(eTYPE_CHICKEN, 10, 4, 4));
	}


	Feature *JungleBiome::getTreeFeature(Random *random)
	{
		if (random->nextInt(10) == 0)
		{
			return new BasicTree(false);
		}
		if (random->nextInt(2) == 0)
		{
			return new GroundBushFeature(TreeTile::JUNGLE_TRUNK, LeafTile::NORMAL_LEAF); // Shrub
		}
		if (!this->isEdge && random->nextInt(3) == 0)
		{
			return new MegaTreeFeature(false, 10 + random->nextInt(20), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF);
		}
		return new TreeFeature(false, 4 + random->nextInt(7), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF, true);
	}

	Feature *JungleBiome::getGrassFeature(Random *random)
	{
		if (random->nextInt(4) == 0)
		{
			return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::FERN);
		}
		return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::TALL_GRASS);
	}

	void JungleBiome::decorate(Level *level, Random *random, int xo, int zo)
	{
		Biome::decorate(level, random, xo, zo);

		PIXBeginNamedEvent(0, "Adding vines");
		VinesFeature *vines = new VinesFeature();

		for (int j = 0; j < 50; j++)
		{
			int x = xo + random->nextInt(16) + 8;
			int y2 = 128;
			int z = zo + random->nextInt(16) + 8;
			vines->place(level, random, x, y2, z);
		}
		delete vines;
		PIXEndNamedEvent();

		PIXBeginNamedEvent(0, "Adding melons");
		if (random->nextInt(4) == 0) // Common: 1 in 4 chunks
		{
			int x = xo + random->nextInt(16) + 8;
			int z = zo + random->nextInt(16) + 8;
			int y = random->nextInt(Level::genDepth);
			MelonFeature *melonFeature = new MelonFeature();
			melonFeature->place(level, random, x, y, z);
			delete melonFeature;
		}
		PIXEndNamedEvent();

		// neoLegacy - natural bamboo. Bamboo Jungle: dense patches on podzol. Jungle / Jungle Hills: a few scattered stalks.
		// Jungle Edge (sparse jungle): none.
		PIXBeginNamedEvent(0, "Adding bamboo");
		if (isBamboo)
		{
			for (int p = 0; p < 6; p++)
			{
				int x = xo + random->nextInt(16) + 8;
				int z = zo + random->nextInt(16) + 8;
				BambooFeature bamboo(14, 4, true);
				bamboo.place(level, random, x, 0, z);
			}
		}
		else if (!isEdge && random->nextInt(3) == 0)
		{
			int x = xo + random->nextInt(16) + 8;
			int z = zo + random->nextInt(16) + 8;
			BambooFeature bamboo(3, 4, false);
			bamboo.place(level, random, x, 0, z);
		}
		PIXEndNamedEvent();
	}
