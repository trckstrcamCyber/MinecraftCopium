#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "LuckyStructureConfig.h"

bool LuckyStructureConfig::luckyBlockSpawnEnabled = true;
// A chunk is a 16x16 = 256 block column footprint. "1 per ~3000-6000
// blocks" is inherently a fuzzy, non-grid spec - treated here as roughly
// "1 in every 200-350 chunks", which lands in a comparable ballpark
// without needing an actual block-distance search.
// Bumped a little more common (was 200-350): roughly 1 per ~2100-3700
// blocks now, about 1.5x as frequent as before. Still rare, just less
// of a needle in a haystack.
int LuckyStructureConfig::luckyBlockChanceMin = 130;
int LuckyStructureConfig::luckyBlockChanceMax = 230;
bool LuckyStructureConfig::luckyBlockMarkerEnabled = true;
int LuckyStructureConfig::luckyBlockMarkerWoolColor = -1; // -1 = pick randomly per spawn

bool LuckyStructureConfig::tentSpawnEnabled = true;
int LuckyStructureConfig::tentChance = 120; // rarer than a single ore vein, rarer than a village
int LuckyStructureConfig::tentTravelersNoteChance = 8; // 1-in-8 tents have a note

bool LuckyStructureConfig::miniStructuresEnabled = true;
int LuckyStructureConfig::brokenCartChance = 60;
int LuckyStructureConfig::oreShrineChance = 90;
int LuckyStructureConfig::ruinedFenceChance = 50;
int LuckyStructureConfig::abandonedCampfireChance = 45;
int LuckyStructureConfig::rockPileChance = 40;
int LuckyStructureConfig::hunterPostChance = 70;
int LuckyStructureConfig::miniCaveChance = 100;

std::vector<Biome *> &LuckyStructureConfig::safeOverworldBiomes()
{
	static std::vector<Biome *> list;
	if (list.empty())
	{
		list.push_back(Biome::plains);
		list.push_back(Biome::forest);
	}
	return list;
}

std::vector<Biome *> &LuckyStructureConfig::travelerCampBiomes()
{
	static std::vector<Biome *> list;
	if (list.empty())
	{
		list.push_back(Biome::plains);
		list.push_back(Biome::forest);
		list.push_back(Biome::taiga);
	}
	return list;
}

bool LuckyStructureConfig::isBiomeIn(Biome *biome, std::vector<Biome *> &list)
{
	if (biome == nullptr) return false;
	for (auto &b : list)
	{
		if (b == biome) return true;
	}
	return false;
}
