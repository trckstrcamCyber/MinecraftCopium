#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.tile.h"

namespace
{
bool isMesaBiome(Biome* biome)
{
	return biome == Biome::mesa || biome == Biome::mesaPlateauF || biome == Biome::mesaPlateau
		|| biome == Biome::mesaBryce || biome == Biome::mesaPlateauFM || biome == Biome::mesaPlateauM;
}

bool isMesaTerrainBlock(int block)
{
	return block == Tile::stone_Id || block == Tile::dirt_Id || block == Tile::sand_Id
		|| block == Tile::gravel_Id || block == Tile::sandstone_Id || block == Tile::clay_Id
		|| block == Tile::stained_hardened_clay_Id || block == Tile::hardened_clay_Id
		|| block == Tile::red_sandstone_Id;
}

bool hasMesaTerrainAboveOrBelow(Level* level, BoundingBox* pieceBox, BoundingBox* chunkBB)
{
	const int x0 = max(pieceBox->x0, chunkBB->x0);
	const int x1 = min(pieceBox->x1, chunkBB->x1);
	const int z0 = max(pieceBox->z0, chunkBB->z0);
	const int z1 = min(pieceBox->z1, chunkBB->z1);
	if (x0 > x1 || z0 > z1)
	{
		return false;
	}

	const int aboveY = pieceBox->y1 + 1;
	const int belowY = pieceBox->y0 - 1;
	for (int x = x0; x <= x1; ++x)
	{
		for (int z = z0; z <= z1; ++z)
		{
			const bool terrainAbove = aboveY < Level::maxBuildHeight
				&& isMesaTerrainBlock(level->getTile(x, aboveY, z));
			const bool terrainBelow = belowY >= Level::minBuildHeight
				&& isMesaTerrainBlock(level->getTile(x, belowY, z));
			if (!terrainAbove && !terrainBelow)
			{
				return false;
			}
		}
	}
	return true;
}
}

MineShaftStart::MineShaftStart()
{
	mesaWood = false;
}

MineShaftStart::MineShaftStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	Biome* biome = level->getBiome((chunkX << 4) + 8, (chunkZ << 4) + 8);
	mesaWood = isMesaBiome(biome);
	MineShaftPieces::MineShaftRoom *mineShaftRoom = new MineShaftPieces::MineShaftRoom(0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2, mesaWood);
	pieces.push_back(mineShaftRoom);
	mineShaftRoom->addChildren(mineShaftRoom, &pieces, random);

	calculateBoundingBox();
	if (mesaWood)
	{
		const int yShift = level->getSeaLevel() - boundingBox->y1 + boundingBox->getYSpan() / 2 + 5;
		boundingBox->move(0, yShift, 0);
		for (auto& piece : pieces)
		{
			piece->getBoundingBox()->move(0, yShift, 0);
		}
	}
	else
	{
		moveBelowSeaLevel(level, random, 10);
	}
}

void MineShaftStart::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (!mesaWood)
	{
		StructureStart::postProcess(level, random, chunkBB);
		return;
	}

	auto it = pieces.begin();
	while (it != pieces.end())
	{
		StructurePiece* piece = *it;
		BoundingBox* pieceBox = piece->getBoundingBox();
		if (!pieceBox->intersects(chunkBB))
		{
			++it;
			continue;
		}

		const int centerX = (max(pieceBox->x0, chunkBB->x0) + min(pieceBox->x1, chunkBB->x1)) / 2;
		const int centerZ = (max(pieceBox->z0, chunkBB->z0) + min(pieceBox->z1, chunkBB->z1)) / 2;
		if (!isMesaBiome(level->getBiome(centerX, centerZ))
			|| !hasMesaTerrainAboveOrBelow(level, pieceBox, chunkBB))
		{
			++it;
			continue;
		}

		if (!piece->postProcess(level, random, chunkBB))
		{
			it = pieces.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void MineShaftStart::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putBoolean(L"Mesa", mesaWood);
}

void MineShaftStart::readAdditonalSaveData(CompoundTag *tag)
{
	mesaWood = tag->getBoolean(L"Mesa");
	if (!tag->contains(L"Mesa") && !pieces.empty())
	{
		MineShaftPieces::MineShaftRoom* room = dynamic_cast<MineShaftPieces::MineShaftRoom*>(pieces.front());
		mesaWood = room != nullptr && room->usesMesaWood();
	}
}