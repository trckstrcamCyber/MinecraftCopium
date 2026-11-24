#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "JavaMath.h"
#include "Mth.h"

const wstring MineShaftFeature::OPTION_CHANCE = L"chance";

namespace
{
bool isMesaBiome(Biome* biome)
{
	return biome == Biome::mesa || biome == Biome::mesaPlateauF || biome == Biome::mesaPlateau
		|| biome == Biome::mesaBryce || biome == Biome::mesaPlateauFM || biome == Biome::mesaPlateauM;
}

}

MineShaftFeature::MineShaftFeature()
{
	chance = 0.01;
}

wstring MineShaftFeature::getFeatureName()
{
	return L"Mineshaft";
}

MineShaftFeature::MineShaftFeature(unordered_map<wstring, wstring> options)
{
	chance = 0.01;

	for(auto& option : options)
	{
		if (option.first.compare(OPTION_CHANCE) == 0)
		{
			chance = Mth::getDouble(option.second, chance);
		}
	}
}

bool MineShaftFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	bool forcePlacement = false;
	LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
	if( levelGenOptions != nullptr )
	{
		forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_Mineshaft);
	}

	double effectiveChance = chance;
	bool mesaBiome = false;
	if (level != nullptr)
	{
		Biome *b = level->getBiome(x * 16 + 8, z * 16 + 8);
		mesaBiome = isMesaBiome(b);
		if (mesaBiome)
			effectiveChance = 0.9;
	}

	if (mesaBiome && !forcePlacement)
	{
		const int originX = (x << 4) + 2;
		const int originZ = (z << 4) + 2;
		for (const auto& entry : cachedStructures)
		{
			StructureStart* start = entry.second;
			BoundingBox* box = start->getBoundingBox();
			if (start->isValid() && box != nullptr
				&& box->intersects(originX, originZ, originX + 12, originZ + 12))
			{
				return false;
			}
		}
	}

	return forcePlacement || (random->nextDouble() < effectiveChance && random->nextInt(80) < max(abs(x), abs(z)));
}

StructureStart *MineShaftFeature::createStructureStart(int x, int z)
{
	// 4J added
	app.AddTerrainFeaturePosition(eTerrainFeature_Mineshaft,x,z);

	return new MineShaftStart(level, random, x, z);
}