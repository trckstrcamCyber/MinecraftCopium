#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "OreFeature.h"
#include "MiniStructureFeatures.h"
#include "LuckyStructureConfig.h"
#include "WorldGenSafetyChecks.h"

// All seven features share the same overall shape: roll enabled + chance,
// check biome (the general safe-overworld list, same as Lucky Block
// spawns), check the small footprint is still natural, then build.
// Kept as separate small classes (not one switch) so each can be tuned,
// disabled, or reused independently later.

static bool commonGate(Level *level, Random *random, int x, int z, int chance, int &outGroundY)
{
	if (!LuckyStructureConfig::miniStructuresEnabled) return false;
	if (random->nextInt(chance) != 0) return false;

	Biome *biome = level->getBiome(x, z);
	if (!LuckyStructureConfig::isBiomeIn(biome, LuckyStructureConfig::safeOverworldBiomes())) return false;

	return WorldGenSafetyChecks::isSafeOutdoorSurface(level, x, z, outGroundY);
}

// ---------------------------------------------------------------------
// Broken Cart - a couple of disconnected rail tiles, a derailed
// minecart, and a small chance of a lone skeleton keeping watch.
// ---------------------------------------------------------------------
BrokenCartFeature::BrokenCartFeature() : Feature(false) {}

bool BrokenCartFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::brokenCartChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x - 1, z - 1, x + 1, z + 1, groundY)) return false;

	placeBlock(level, x, groundY, z, Tile::rail->id, 0);
	placeBlock(level, x + 1, groundY, z, Tile::rail->id, 0);

	shared_ptr<MinecartRideable> cart = std::make_shared<MinecartRideable>(level, x + 0.5, groundY + 0.5, z + 0.3);
	level->addEntity(cart);

	if (random->nextInt(4) == 0)
	{
		shared_ptr<Skeleton> guard = std::make_shared<Skeleton>(level);
		guard->moveTo(x - 1 + 0.5, groundY + 1, z + 0.5, random->nextFloat() * 360.0f, 0);
		level->addEntity(guard);
	}
	return true;
}

// ---------------------------------------------------------------------
// Tiny Ore Shrine - a small stone-brick pedestal topped with a raw ore
// block, like someone left an offering.
// ---------------------------------------------------------------------
OreShrineFeature::OreShrineFeature() : Feature(false) {}

bool OreShrineFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::oreShrineChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x, z, x, z, groundY)) return false;

	static const int orePool[] = { Tile::coal_ore_Id, Tile::iron_ore_Id, Tile::gold_ore_Id };
	int ore = orePool[random->nextInt(3)];

	placeBlock(level, x, groundY, z, Tile::stoneBrick->id, 0);
	placeBlock(level, x, groundY + 1, z, ore, 0);
	return true;
}

// ---------------------------------------------------------------------
// Ruined Fence - a short broken fence line with a couple of gaps, like
// an old pasture boundary nobody maintained.
// ---------------------------------------------------------------------
RuinedFenceFeature::RuinedFenceFeature() : Feature(false) {}

bool RuinedFenceFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::ruinedFenceChance, groundY)) return false;

	int length = 4 + random->nextInt(3); // 4-6
	bool alongX = random->nextInt(2) == 0;
	int dx = alongX ? 1 : 0;
	int dz = alongX ? 0 : 1;

	int endX = x + dx * (length - 1);
	int endZ = z + dz * (length - 1);
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x, z, endX, endZ, groundY)) return false;

	for (int i = 0; i < length; i++)
	{
		if (random->nextInt(3) == 0) continue; // gap - "broken" section
		placeBlock(level, x + dx * i, groundY, z + dz * i, Tile::fence_Id, 0);
	}
	return true;
}

// ---------------------------------------------------------------------
// Abandoned Campfire - burnt out, not lit (no active fire tile placed,
// deliberately, both thematically and to avoid worldgen fire-spread
// risk). Just a cold ring of cobblestone and a scattering of charcoal.
// ---------------------------------------------------------------------
AbandonedCampfireFeature::AbandonedCampfireFeature() : Feature(false) {}

bool AbandonedCampfireFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::abandonedCampfireChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x - 1, z - 1, x + 1, z + 1, groundY)) return false;

	static const int ringOffsets[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (auto &off : ringOffsets)
	{
		placeBlock(level, x + off[0], groundY, z + off[1], Tile::cobblestone_Id, 0);
	}

	// A little burnt log in the middle rather than fire itself.
	placeBlock(level, x, groundY, z, Tile::treeTrunk->id, 0);

	shared_ptr<ItemEntity> charcoal = std::make_shared<ItemEntity>(
		level, x + 0.5, groundY + 1, z + 0.5,
		std::make_shared<ItemInstance>(Item::coal, 1 + random->nextInt(2), CoalItem::CHAR_COAL));
	level->addEntity(charcoal);

	return true;
}

// ---------------------------------------------------------------------
// Rock Pile - a small irregular mound of stone/andesite/gravel, no
// function, pure scenery.
// ---------------------------------------------------------------------
RockPileFeature::RockPileFeature() : Feature(false) {}

bool RockPileFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::rockPileChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x, z - 1, x + 1, z, groundY)) return false;

	static const int blockPool[] = { Tile::stone_Id, Tile::gravel_Id, Tile::stone_Id };
	static const int dataPool[] = { StoneTile::ANDESITE, 0, StoneTile::GRANITE };

	for (int dx = 0; dx <= 1; dx++)
	{
		for (int dz = -1; dz <= 0; dz++)
		{
			int pick = random->nextInt(3);
			placeBlock(level, x + dx, groundY, z + dz, blockPool[pick], blockPool[pick] == Tile::stone_Id ? dataPool[pick] : 0);
		}
	}
	if (random->nextInt(2) == 0)
	{
		placeBlock(level, x, groundY + 1, z, Tile::stone_Id, StoneTile::ANDESITE);
	}
	return true;
}

// ---------------------------------------------------------------------
// Hunter Post - a raised fence watch-post with a torch, marking old
// hunting grounds. Small chance of a lone skeleton nearby.
// ---------------------------------------------------------------------
HunterPostFeature::HunterPostFeature() : Feature(false) {}

bool HunterPostFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::hunterPostChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x, z, x, z, groundY)) return false;

	placeBlock(level, x, groundY, z, Tile::fence_Id, 0);
	placeBlock(level, x, groundY + 1, z, Tile::fence_Id, 0);
	placeBlock(level, x, groundY + 2, z, Tile::torch_Id, 5);

	if (random->nextInt(5) == 0)
	{
		shared_ptr<Skeleton> hunter = std::make_shared<Skeleton>(level);
		hunter->moveTo(x + 1.5, groundY + 1, z + 0.5, random->nextFloat() * 360.0f, 0);
		level->addEntity(hunter);
	}
	return true;
}

// ---------------------------------------------------------------------
// Mini-Cave - a small open pit dug down into the stone with 1-2 ore
// veins placed just outside its walls. Not a hidden tunnel-style cave
// (this generation pass has no reliable way to detect hillsides to dig
// into sideways) - it reads more as a small sinkhole/quarry pocket with
// exposed ore, which keeps it both discoverable and simple to build.
// ---------------------------------------------------------------------
MiniCaveFeature::MiniCaveFeature() : Feature(false) {}

bool MiniCaveFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int groundY;
	if (!commonGate(level, random, x, z, LuckyStructureConfig::miniCaveChance, groundY)) return false;
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x, z - 1, x + 1, z, groundY)) return false;

	int depth = 3 + random->nextInt(2); // 3-4 deep
	for (int dx = 0; dx <= 1; dx++)
	{
		for (int dz = -1; dz <= 0; dz++)
		{
			for (int dy = 0; dy < depth; dy++)
			{
				placeBlock(level, x + dx, groundY - dy, z + dz, Tile::air_Id, 0);
			}
		}
	}

	OreFeature coalVein(Tile::coal_ore_Id, 10);
	coalVein.place(level, random, x - 1 + random->nextInt(4), groundY - 1 - random->nextInt(depth), z - 2 + random->nextInt(4));

	if (random->nextInt(2) == 0)
	{
		OreFeature ironVein(Tile::iron_ore_Id, 6);
		ironVein.place(level, random, x - 1 + random->nextInt(4), groundY - 2 - random->nextInt(depth), z - 2 + random->nextInt(4));
	}

	return true;
}
