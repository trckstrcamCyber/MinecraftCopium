#pragma once
#include <vector>

class Biome;

// Central, easy-to-tune knobs for the new atmospheric worldgen layer
// (rare Lucky Block spawns, abandoned tents, and small exploration
// mini-structures). Everything here is a plain static variable rather
// than a compile-time constant, so it can be wired up to an in-game
// settings screen later without touching any generation logic.
//
// NOTE ON "chance" NUMBERS BELOW: BiomeDecorator runs once per 16x16
// chunk. A "1 in N" chance here means "1 in N chunks", not "1 in N
// blocks" - there's no direct block-distance search in this generation
// pass, only a per-chunk roll. A chunk is 256 blocks of surface area,
// so "roughly 1 per 3000-6000 blocks" was translated to a per-chunk
// denominator in that same ballpark (see LuckyStructureConfig.cpp for
// the actual math comment). If you want true block-distance spacing
// later, that would need a min-distance-to-last-spawn tracker, which
// isn't implemented here to keep this pass lightweight.
struct LuckyStructureConfig
{
	// ---- Rare Lucky Block world spawns ----
	static bool luckyBlockSpawnEnabled;
	static int luckyBlockChanceMin; // 1-in-N chunks, lower bound
	static int luckyBlockChanceMax; // 1-in-N chunks, upper bound
	static bool luckyBlockMarkerEnabled;
	static int luckyBlockMarkerWoolColor; // DyePowderItem color index

	// ---- Abandoned tents ----
	static bool tentSpawnEnabled;
	static int tentChance; // 1-in-N chunks
	static int tentTravelersNoteChance; // 1-in-N chance the tent's chest has a note

	// ---- Mini-structures (each independent, own rarity) ----
	static bool miniStructuresEnabled;
	static int brokenCartChance;
	static int oreShrineChance;
	static int ruinedFenceChance;
	static int abandonedCampfireChance;
	static int rockPileChance;
	static int hunterPostChance;
	static int miniCaveChance;

	// Shared biome allow-lists. Populated lazily on first use so this
	// doesn't depend on Biome static init order.
	static std::vector<Biome *> &safeOverworldBiomes();   // plains, forest
	static std::vector<Biome *> &travelerCampBiomes();    // plains, forest, taiga

	static bool isBiomeIn(Biome *biome, std::vector<Biome *> &list);
};
