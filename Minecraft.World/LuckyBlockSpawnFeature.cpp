#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "LuckyBlockSpawnFeature.h"
#include "LuckyStructureConfig.h"
#include "WorldGenSafetyChecks.h"

LuckyBlockSpawnFeature::LuckyBlockSpawnFeature() : Feature(false)
{
	// doUpdate = false -> placeBlock uses the lighter UPDATE_CLIENTS path,
	// same as the other worldgen features (trees, ores, etc.) - this runs
	// on every chunk, so it should stay cheap even though it almost
	// always rejects and does nothing.
}

bool LuckyBlockSpawnFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (!LuckyStructureConfig::luckyBlockSpawnEnabled) return false;

	int chance = LuckyStructureConfig::luckyBlockChanceMin;
	int span = LuckyStructureConfig::luckyBlockChanceMax - LuckyStructureConfig::luckyBlockChanceMin;
	if (span > 0) chance += random->nextInt(span);
	if (chance < 1) chance = 1;
	if (random->nextInt(chance) != 0) return false;

	Biome *biome = level->getBiome(x, z);
	if (!LuckyStructureConfig::isBiomeIn(biome, LuckyStructureConfig::safeOverworldBiomes())) return false;

	int groundY;
	if (!WorldGenSafetyChecks::isSafeOutdoorSurface(level, x, z, groundY)) return false;

	if (LuckyStructureConfig::luckyBlockMarkerEnabled)
	{
		int color = LuckyStructureConfig::luckyBlockMarkerWoolColor;
		if (color < 0)
		{
			static const int visibleColors[] = {
				DyePowderItem::RED, DyePowderItem::YELLOW, DyePowderItem::LIME,
				DyePowderItem::LIGHT_BLUE, DyePowderItem::MAGENTA, DyePowderItem::ORANGE,
				DyePowderItem::PINK, DyePowderItem::WHITE,
			};
			color = visibleColors[random->nextInt(8)];
		}
		// The marker takes the ground block's place; the Lucky Block sits
		// directly on top of it.
		placeBlock(level, x, groundY - 1, z, Tile::wool_Id, color);
	}

	placeBlock(level, x, groundY, z, Tile::lucky_block_Id, 0);
	return true;
}
